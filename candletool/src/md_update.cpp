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
#include "flasher.hpp"
#include "mabFileParser.hpp"
#include "mab_types.hpp"
#include "utilities.hpp"
#include "mini/ini.h"

namespace mab
{
    namespace
    {
        // Firmware server directories, relative to CurlHandler::FW_SERVER_ROOT
        constexpr const char* FW_MD_DIR        = "md/";         // .mab files, fw >= 3.0.0
        constexpr const char* FW_MD_LEGACY_DIR = "md/legacy/";  // flasher executables, fw < 3.0.0

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

        /// @brief Reset the drive, or read its firmware version, over CANopen instead of the MD
        /// protocol
        ///
        /// Drives running CANopen firmware do not answer MD protocol frames, so the reset that
        /// puts them into the bootloader has to be sent as an SDO write on a CAN 2.0 link
        /// @param mdCanId can node id of the drive to reset
        /// @param candleBuilder builder of the candle used for communication, copied so the
        /// CAN 2.0 frame format is only forced for this reset
        /// @param packageEtcPath etc path of the package, holds candletool.ini pointing at the
        /// .eds
        /// @param log logger of the calling cli
        /// @param versionOut when set, the firmware version is read into it instead of resetting
        /// the drive
        /// @return true when the drive acknowledged the reset / reported its version
        bool overCanOpen(const std::shared_ptr<canId_t>             mdCanId,
                         const std::shared_ptr<const CandleBuilder> candleBuilder,
                         const std::filesystem::path&               packageEtcPath,
                         const Logger&                              log,
                         version_ut*                                versionOut = nullptr)
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

            if (versionOut != nullptr)
            {
                const auto [version, err] = mdco.getFirmwareVersion();
                detachCandle(candle);
                if (err != MDCO::Error_t::OK)
                {
                    log.error("Could not read firmware version of MD device with ID %d",
                              *mdCanId);
                    return false;
                }
                *versionOut = version;
                return true;
            }

            const MDCO::Error_t err = mdco.reset();
            detachCandle(candle);
            if (err != MDCO::Error_t::OK)
            {
                log.error("Error resetting MD device with ID %d over CANopen", *mdCanId);
                return false;
            }
            return true;
        }

        /// @brief parse "X.Y.Z" with optional suffix (e.g. "3.0.0_3b52568")
        bool parseVersion(const char* str, version_ut* version)
        {
            unsigned int major = 0, minor = 0, revision = 0;
            if (sscanf(str, "%u.%u.%u", &major, &minor, &revision) != 3 || major > 255 ||
                minor > 255 || revision > 255)
                return false;
            version->i          = 0;
            version->s.major    = major;
            version->s.minor    = minor;
            version->s.revision = revision;
            return true;
        }

        /// @brief compare versions ignoring tag, returns <0, 0 or >0
        int compareVersion(version_ut a, version_ut b)
        {
            u32 va = (a.s.major << 16) | (a.s.minor << 8) | a.s.revision;
            u32 vb = (b.s.major << 16) | (b.s.minor << 8) | b.s.revision;
            return (va > vb) - (va < vb);
        }

        bool readFirmwareVersion(UpdateReset_E                              resetMode,
                                 const std::shared_ptr<canId_t>             mdCanId,
                                 const std::shared_ptr<const CandleBuilder> candleBuilder,
                                 const std::filesystem::path&               packageEtcPath,
                                 const Logger&                              log,
                                 version_ut*                                version)
        {
            if (resetMode == UpdateReset_E::CANOPEN)
                return overCanOpen(mdCanId, candleBuilder, packageEtcPath, log, version);
            auto md = connectMd(mdCanId, candleBuilder, log);
            if (md == nullptr)
            {
                log.error("Could not communicate with MD device with ID %d", *mdCanId);
                return false;
            }
            *version = getMdFirmwareVersion(*md);
            return true;
        }

        bool resetDrive(UpdateReset_E                              resetMode,
                        const std::shared_ptr<canId_t>             mdCanId,
                        const std::shared_ptr<const CandleBuilder> candleBuilder,
                        const std::filesystem::path&               packageEtcPath,
                        const Logger&                              log)
        {
            if (resetMode == UpdateReset_E::CANOPEN)
                return overCanOpen(mdCanId, candleBuilder, packageEtcPath, log);
            auto md = connectMd(mdCanId, candleBuilder, log);
            if (md == nullptr)
            {
                log.error("Could not communicate with MD device with ID %d", *mdCanId);
                return false;
            }
            md->reset();
            return true;
        }

        bool confirmUpdate(version_ut                                 targetVersion,
                           UpdateReset_E                              resetMode,
                           const std::shared_ptr<canId_t>             mdCanId,
                           const std::shared_ptr<const CandleBuilder> candleBuilder,
                           const std::filesystem::path&               packageEtcPath,
                           const Logger&                              log)
        {
            version_ut currentVersion;
            if (!readFirmwareVersion(
                    resetMode, mdCanId, candleBuilder, packageEtcPath, log, &currentVersion))
                return false;

            int cmp = compareVersion(targetVersion, currentVersion);
            log.info("%s MD firmware v%d.%d.%d -> v%d.%d.%d",
                     cmp > 0 ? "Upgrading" : (cmp < 0 ? "Downgrading" : "Reinstalling"),
                     currentVersion.s.major,
                     currentVersion.s.minor,
                     currentVersion.s.revision,
                     targetVersion.s.major,
                     targetVersion.s.minor,
                     targetVersion.s.revision);
            if ((currentVersion.s.major < 3) != (targetVersion.s.major < 3))
                log.warn(
                    "This comes with changes, that in specific conditions "
                    "(motor+encoder combinations) may require you to:\n"
                    "- reapply .cfg file,\n"
                    "- perform calibration,\n"
                    "- set zero offset.");
            if (!userConfirm())
            {
                log.error("Update aborted by user!");
                return false;
            }
            return true;
        }

        void flashMabFile(const std::filesystem::path&               path,
                          bool                                       recovery,
                          UpdateReset_E                              resetMode,
                          const std::shared_ptr<canId_t>             mdCanId,
                          const std::shared_ptr<const CandleBuilder> candleBuilder,
                          const std::filesystem::path&               packageEtcPath,
                          const Logger&                              log)
        {
            MabFileParser mabFile(path.string(), MabFileParser::TargetDevice_E::MD);

            version_ut targetVersion = {.i = 0};
            if (!parseVersion((const char*)mabFile.m_fwEntry.version, &targetVersion))
            {
                log.error("Invalid firmware version in .mab file!");
                return;
            }

            // In recovery the drive sits in bootloader and can not report its version
            if (!recovery)
            {
                if (!confirmUpdate(
                        targetVersion, resetMode, mdCanId, candleBuilder, packageEtcPath, log))
                    return;
                if (!resetDrive(resetMode, mdCanId, candleBuilder, packageEtcPath, log))
                    return;
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
            if (!canLoader.flashAndBoot(recovery))
            {
                log.error("MD flashing failed!");
                return;
            }
            log.success("Update complete for MD @ %d", *mdCanId);
        }

    }  // namespace

    void updateMd(const UpdateOptions&                       options,
                  UpdateReset_E                              resetMode,
                  const std::shared_ptr<canId_t>             mdCanId,
                  const std::shared_ptr<const CandleBuilder> candleBuilder,
                  const std::filesystem::path&               packageEtcPath,
                  const Logger&                              log)
    {
        const char* commandName = resetMode == UpdateReset_E::CANOPEN ? "mdco" : "md";

        if (*options.forceErase)
        {
            log.info(
                "The force-erase, will erase the whole configuration from the drive, "
                "including"
                "bootloader configuration. Drives' CAN ID will be set default 100 (0x64)! "
                "Proceed?");
            if (!userConfirm())
            {
                log.error("Factory Reset aborted by user!");
                return;
            }
            if (!*options.recovery && resetMode == UpdateReset_E::CANOPEN)
            {
                if (!overCanOpen(mdCanId, candleBuilder, packageEtcPath, log))
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

        const bool recovery = *options.recovery;

        if (!options.pathToMabFile->empty())
        {
            log.info("Overriding download of file. Using local provided path.");
            flashMabFile(*options.pathToMabFile,
                         recovery,
                         resetMode,
                         mdCanId,
                         candleBuilder,
                         packageEtcPath,
                         log);
            return;
        }

        const std::string& version       = *options.fwVersion;
        version_ut         targetVersion = {.i = 0};
        const bool         latest        = version == "latest";
        if (!latest && !parseVersion(version.c_str(), &targetVersion))
        {
            log.error(
                "Please provide version of fw (X.Y.Z) or \"latest\" keyword in the "
                "argument!");
            log.error("For example candletool %s update latest", commandName);
            return;
        }

        mINI::INIStructure index;
        if (!CurlHandler::loadIndex(index))
            return;
        std::filesystem::path tmpDir = std::filesystem::temp_directory_path();

        // Firmware older than 3.0.0 is shipped as platform specific flasher
        // executables with firmware compiled in, newer as platform independent .mab
        if (!latest && targetVersion.s.major < 3)
        {
#ifdef WIN32
            log.error("Firmware older than 3.0.0 can only be installed on Linux!");
            return;
#else
            constexpr sysArch_E arch    = getSysArch();
            const char*         archKey = "filename";
            if constexpr (arch == sysArch_E::X86_64)
                archKey = "filename_x86_64";
            else if constexpr (arch == sysArch_E::ARM64)
                archKey = "filename_arm64";
            else if constexpr (arch == sysArch_E::ARMHF)
                archKey = "filename_armhf";

            std::string filename =
                CurlHandler::findIndexEntry(index, "mab_can_flasher_", version, archKey);
            if (filename.empty())
            {
                log.error("Firmware %s is not available for this platform!", version.c_str());
                return;
            }
            if (!recovery && !confirmUpdate(targetVersion,
                                            resetMode,
                                            mdCanId,
                                            candleBuilder,
                                            packageEtcPath,
                                            log))
                return;
            if (!recovery && resetMode == UpdateReset_E::CANOPEN)
                log.warn(
                    "The downloaded legacy flasher resets the drive on its own, over the MD "
                    "protocol.");

            WebFile_S flasherFile;
            flasherFile.m_type = WebFile_S::Type_E::MD_FLASHER;
            flasherFile.m_path = tmpDir / filename;
            if (!CurlHandler::download(
                    std::string(CurlHandler::FW_SERVER_ROOT) + FW_MD_LEGACY_DIR + filename,
                    flasherFile.m_path))
            {
                log.error("Could not download firmware [ %s ]", filename.c_str());
                return;
            }
            Flasher flasher(flasherFile);
            canId_t flashId = recovery ? 9 : *mdCanId;
            if (flasher.flash(flashId, recovery) != Flasher::Error_E::OK)
            {
                log.error("Error while flashing firmware!");
                return;
            }
            log.success("Update complete for MD @ %d", *mdCanId);
            return;
#endif
        }

        std::string filename = CurlHandler::findIndexEntry(index, "md_app_", version, "filename");
        if (filename.empty())
        {
            log.error("Firmware %s is not available on the server!", version.c_str());
            return;
        }
        std::filesystem::path mabPath = tmpDir / filename;
        if (!CurlHandler::download(std::string(CurlHandler::FW_SERVER_ROOT) + FW_MD_DIR + filename,
                                   mabPath))
        {
            log.error("Could not download firmware [ %s ]", filename.c_str());
            return;
        }
        flashMabFile(mabPath, recovery, resetMode, mdCanId, candleBuilder, packageEtcPath, log);
    }
}  // namespace mab
