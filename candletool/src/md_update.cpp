#include "md_update.hpp"

#include <cstdio>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "MD.hpp"
#include "MDCO.hpp"
#include "canLoader.hpp"
#include "candle.hpp"
#include "curl_handler.hpp"
#include "edsParser.hpp"
#include "eds_selection.hpp"
#include "mabFileParser.hpp"
#include "mab_types.hpp"
#include "utilities.hpp"
#include "mini/ini.h"

namespace mab
{
    namespace
    {
        std::unique_ptr<MD, std::function<void(MD*)>> connectMd(
            const std::shared_ptr<canId_t>             mdCanId,
            const std::shared_ptr<const CandleBuilder> candleBuilder,
            const Logger&                              log)
        {
            auto candle = candleBuilder->build().value_or(nullptr);
            if (candle == nullptr)
            {
                return nullptr;
            }
            candle->init();
            std::function<void(MD*)> deleter = [candle](MD* ptr)
            {
                delete ptr;
                detachCandle(candle);
            };
            auto md =
                std::unique_ptr<MD, std::function<void(MD*)>>(new MD(*mdCanId, candle), deleter);
            if (md->init() == MD::Error_t::OK)
                return md;

            log.error("Could not connect to MD!");
            return nullptr;
        }

        /// @brief Reset the drive over CANopen instead of the MD protocol
        ///
        /// Drives running CANopen firmware do not answer MD protocol frames, so the reset that
        /// puts them into the bootloader has to be sent as an SDO write on a CAN 2.0 link
        /// @param mdCanId can node id of the drive to reset
        /// @param candleBuilder builder of the candle used for communication, copied so the
        /// CAN 2.0 frame format is only forced for this reset
        /// @param packageEtcPath etc path of the package, holds candletool.ini pointing at the
        /// .eds
        /// @param log logger of the calling cli
        /// @return true when the drive acknowledged the reset
        bool resetOverCanOpen(const std::shared_ptr<canId_t>             mdCanId,
                              const std::shared_ptr<const CandleBuilder> candleBuilder,
                              const std::filesystem::path&               packageEtcPath,
                              const Logger&                              log)
        {
            const std::filesystem::path configFilePath = packageEtcPath / "config/candletool.ini";

            const auto edsPaths = readEdsPaths(configFilePath, log);
            if (!edsPaths.has_value())
            {
                return false;
            }

            auto [od, parserError] = EDSParser::load(edsPaths.value().current);
            if (parserError != EDSParser::Error_t::OK)
                log.warn("EDS parsing failed!");
            if (od == nullptr)
            {
                log.error("Could not load object dictionary from %s!",
                          edsPaths.value().current.c_str());
                return false;
            }

            // CANopen talks CAN 2.0, the builder is copied so the rest of the update keeps
            // flashing with the frame format it was configured with
            auto canOpenBuilder            = std::make_shared<CandleBuilder>(*candleBuilder);
            canOpenBuilder->useCAN20Frames = true;
            auto candle                    = canOpenBuilder->build().value_or(nullptr);
            if (candle == nullptr)
            {
                log.error("Could not connect to candle!");
                return false;
            }
            candle->init();

            MDCO mdco(*mdCanId, candle, od);
            if (mdco.init() != MDCO::Error_t::OK)
            {
                log.error("Could not communicate with MD device with ID %d over CANopen",
                          *mdCanId);
                detachCandle(candle);
                return false;
            }

            // A drive waiting to be flashed may well be running older firmware, whose dictionary
            // holds the reset command at a different address
            useEdsMatchingFirmware(mdco, od, edsPaths.value().legacy, log);

            const MDCO::Error_t err = mdco.reset();
            detachCandle(candle);
            if (err != MDCO::Error_t::OK)
            {
                log.error("Error resetting MD device with ID %d over CANopen", *mdCanId);
                return false;
            }
            return true;
        }

    }  // namespace

    void updateMd(const UpdateOptions&                       options,
                  UpdateReset_E                              resetMode,
                  const std::shared_ptr<canId_t>             mdCanId,
                  const std::shared_ptr<const CandleBuilder> candleBuilder,
                  const std::filesystem::path&               packageEtcPath,
                  const Logger&                              log)
    {
        if (*options.forceErase)
        {
            log.info(
                "The force-erase, will erase the whole configuration from the drive, "
                "including"
                "bootloader configuration. Drives' CAN ID will be set default 100 (0x64)! "
                "Proceed?");
            std::string answer;
            std::cout << "Type 'Y' to continue: ";
            std::getline(std::cin, answer);
            if (answer != "Y" && answer != "y")
            {
                log.error("Factory Reset aborted by user!");
                return;
            }
            if (!*options.recovery && resetMode == UpdateReset_E::CANOPEN)
            {
                if (!resetOverCanOpen(mdCanId, candleBuilder, packageEtcPath, log))
                    return;
                usleep(200'000);
            }
            auto candle = candleBuilder->build().value_or(nullptr);
            if (candle == nullptr)
            {
                log.error("Could not connect to candle!");
                return;
            }
            if (!*options.recovery && resetMode == UpdateReset_E::MD_PROTOCOL)
            {
                MD md(*mdCanId, candle);
                md.reset();
                usleep(200'000);
            }
            CanLoader canLoader(candle, nullptr, *mdCanId);
            if (!canLoader.forceEraseConfig())
            {
                log.error("Force-erase failed!");
                return;
            }
            log.success("Force-erase complete for MD @ %d", *mdCanId);
            return;
        }
        std::filesystem::path mabPath = *options.pathToMabFile;
        if (mabPath.empty())
        {
            if (options.fwVersion->empty())
            {
                log.error("Please provide version of fw or  \"latest\" keyword in the argument!");
                log.error("For example candletool %s update latest",
                          resetMode == UpdateReset_E::CANOPEN ? "mdco" : "md");
                return;
            }
            mINI::INIStructure index;
            if (!CurlHandler::loadIndex(index))
                return;
            std::string filename =
                CurlHandler::findIndexEntry(index, "md_app_", *options.fwVersion, "filename");
            if (filename.empty())
            {
                log.error("Firmware %s is not available on the server!",
                          options.fwVersion->c_str());
                return;
            }
            mabPath = std::filesystem::temp_directory_path() / filename;
            if (!CurlHandler::download(std::string(CurlHandler::FW_SERVER_ROOT) + "md/" + filename,
                                       mabPath))
            {
                log.error("Could not download firmware [ %s ]", filename.c_str());
                return;
            }
        }
        else
            log.info("Overriding download of file. Using local provided path.");

        MabFileParser mabFile(mabPath.string(), MabFileParser::TargetDevice_E::MD);

        if (*(options.recovery) == false && resetMode == UpdateReset_E::CANOPEN)
        {
            // CANopen firmware does not answer the MD protocol registers the version
            // check reads, so the drive is only reset here
            if (!resetOverCanOpen(mdCanId, candleBuilder, packageEtcPath, log))
                return;
            usleep(200'000);
        }
        else if (*(options.recovery) == false)
        {
            auto md = connectMd(mdCanId, candleBuilder, log);
            if (md == nullptr)
            {
                log.error("Could not communicate with MD device with ID %d", *mdCanId);
                return;
            }
            auto fw = getMdFirmwareVersion(*md);
            if (!isVersionAtLeast(fw, 3, 0, 0))
            {
                log.warn(
                    "You are attempting to update MD from version v%d.%d.%d to version "
                    "%s.\n This comes with changes, that in specific conditions "
                    "(motor+encoder combinations) may require you to:\n"
                    "- reapply .cfg file,\n"
                    "- perform calibration,\n"
                    "- set zero offset.\n"
                    "Continue? [y/n]",
                    fw.s.major,
                    fw.s.minor,
                    fw.s.revision,
                    mabFile.m_fwEntry.version);
                char c;
                std::cin >> c;
                if (c != 'y' && c != 'Y')
                    return;
            }

            md->reset();
            usleep(200'000);
        }
        else
        {
            log.warn("Recovery mode...");
            log.warn("Please make sure driver is in the bootloader phase (rebooting)");
        }
        auto candle = candleBuilder->build().value_or(nullptr);
        if (candle == nullptr)
        {
            log.error("Could not connect to candle!");
            return;
        }
        CanLoader canLoader(candle, &mabFile, *mdCanId);
        if (!canLoader.flashAndBoot(*(options.recovery)))
        {
            log.error("MD flashing failed!");
            return;
        }
        log.success("Update complete for MD @ %d", *mdCanId);
    }
}  // namespace mab
