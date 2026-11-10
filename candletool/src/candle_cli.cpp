
#include <memory>
#include <vector>

#include "candle_cli.hpp"
#include "candle.hpp"
#include "mabFileParser.hpp"
#include "candle_bootloader.hpp"
#include "mab_crc.hpp"
#include "curl_handler.hpp"
#include "web_file.hpp"
#include "flasher.hpp"
#include "utilities.hpp"

#ifdef WIN32
#include <windows.h>
#endif

namespace mab
{
    CandleCli::CandleCli(CLI::App* rootCli, CANdleToolCtx_S ctx)
    {
        if (ctx.candleBranchVec.empty())
        {
            throw std::runtime_error("MDCli arguments can not be empty!");
        }
        auto  candleBuilder = ctx.candleBranchVec.at(0).candleBuilder;
        auto* candleCli =
            rootCli->add_subcommand("candle", "CANdle device commands.")->require_subcommand();
        // Update
        auto* update = candleCli->add_subcommand("update", "Update Candle firmware.");

        UpdateOptions updateOptions(update);
        update->callback(
            [this, updateOptions]()
            {
                m_logger.info("Performing Candle firmware update.");

                std::filesystem::path filepath;

                if (updateOptions.pathToMabFile->empty())
                {
                    const std::string& version = *updateOptions.fwVersion;
                    if (version.empty())
                    {
                        m_logger.error(
                            "Please provide version of fw or \"latest\" keyword in the argument!");
                        m_logger.error("For example candletool candle update latest");
                        return;
                    }

                    mINI::INIStructure index;
                    if (!CurlHandler::loadIndex(index))
                        return;

                    std::string filename =
                        CurlHandler::findIndexEntry(index, "candle_", version, "filename");
                    if (filename.empty())
                    {
                        m_logger.error("Firmware %s is not available on the server!",
                                       version.c_str());
                        return;
                    }
                    filepath = std::filesystem::temp_directory_path() / filename;
                    if (!CurlHandler::download(
                            std::string(CurlHandler::FW_SERVER_ROOT) + "candle/" + filename,
                            filepath))
                    {
                        m_logger.error("Could not download firmware [ %s ]", filename.c_str());
                        return;
                    }
                    m_logger.info("Downloaded firmware to %s", filepath.string().c_str());
                }
                else
                {
                    filepath = *updateOptions.pathToMabFile;
                }
                MabFileParser candleFirmware(filepath.string(),
                                             MabFileParser::TargetDevice_E::CANDLE);

                auto candle_bootloader = attachCandleBootloader();
                for (size_t i = 0; i < candleFirmware.m_fwEntry.size;
                     i += CandleBootloader::PAGE_SIZE_STM32G474)
                {
                    std::array<u8, CandleBootloader::PAGE_SIZE_STM32G474> page;
                    std::memcpy(
                        page.data(), &candleFirmware.m_fwEntry.data->data()[i], page.size());
                    u32 crc = candleCRC::crc32(page.data(), page.size());
                    if (candle_bootloader->writePage(page, crc) != candleTypes::Error_t::OK)
                    {
                        m_logger.error("Candle flashing failed!");
                        break;
                    }
                    m_logger.progress((double)(i) / candleFirmware.m_fwEntry.size);
                }
                m_logger.progress(1.0);
                m_logger.success("CANdle firmware was updated succesffuly!");
            });
        // Discover
        auto* discover =
            candleCli->add_subcommand("discover", "List CANdle devices connected via USB.");
        discover->callback(
            [this, candleBuilder]()
            {
                auto serials = USB::listSerials(Candle::CANDLE_VID, Candle::CANDLE_PID);
                if (serials.empty())
                {
                    m_logger.error("No CANdle found!");
                    return;
                }
                m_logger.info("Found %zu CANdle device(s):", serials.size());
                for (const auto& serial : serials)
                {
                    // Connection logs are not useful here, only the summary is printed
                    auto prevVerbosity    = Logger::g_m_verbosity;
                    Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;
                    std::optional<version_ut> version;
                    auto bus = std::make_unique<USB>(Candle::CANDLE_VID, Candle::CANDLE_PID, serial);
                    if (bus->connect() == I_CommunicationInterface::Error_t::OK)
                    {
                        Candle candle(*candleBuilder->datarate, std::move(bus));
                        if (candle.init() == candleTypes::Error_t::OK)
                            version = candle.getCandleVersion();
                    }
                    Logger::g_m_verbosity = prevVerbosity;

                    if (version.has_value())
                        m_logger.info(" - serial: %s | firmware: %d.%d.%d(%c)",
                                      serial.c_str(),
                                      version->s.major,
                                      version->s.minor,
                                      version->s.revision,
                                      version->s.tag);
                    else
                        m_logger.info(" - serial: %s | firmware: unknown (device busy?)",
                                      serial.c_str());
                }
            });

        // Version
        auto* version = candleCli->add_subcommand("version", "Get CANdle device version.");
        version->callback(
            [this, candleBuilder]()
            {
                auto candle = candleBuilder->build();
                if (!candle.has_value())
                {
                    m_logger.error("Could not connect to CANdle!");
                    return;
                }

                auto versionOpt = candle.value()->getCandleVersion();
                if (!versionOpt.has_value())
                {
                    m_logger.error("Could not read CANdle version!");
                    return;
                }

                m_logger.info("CANdle firmware version: %d.%d.%d(%c)",
                              versionOpt.value().s.major,
                              versionOpt.value().s.minor,
                              versionOpt.value().s.revision,
                              versionOpt.value().s.tag);
            });
#ifdef WIN32

        auto* driver = candleCli->add_subcommand("driver", "Install CANdle USB driver.");
        driver->callback(
            [this]()
            {
                m_logger.info("Installing CANdle USB driver...");

                const char* exeName = "candlesdk-win-driver.exe";
                char        fullPath[MAX_PATH];

                DWORD resultSearch = SearchPathA(NULL, exeName, NULL, MAX_PATH, fullPath, NULL);
                if (resultSearch == 0 || resultSearch > MAX_PATH)
                {
                    m_logger.error(
                        "Could not find %s in PATH! Did you enable it during installation? If not "
                        "go to your installation folder and manually run candlesdk-win-driver.exe "
                        "as administrator.",
                        exeName);
                    exit(1);
                }
                m_logger.info("Found driver installer at %s", fullPath);

                // Inline extraction of directory from full path
                std::string fullPathStr(fullPath);
                size_t      lastSlash = fullPathStr.find_last_of("\\/");
                std::string dirPath =
                    (lastSlash == std::string::npos) ? "." : fullPathStr.substr(0, lastSlash);

                HINSTANCE result = ShellExecuteA(NULL,             // no parent window
                                                 "runas",          // causes UAC elevation prompt
                                                 fullPath,         // program to run
                                                 NULL,             // arguments
                                                 dirPath.c_str(),  // working directory
                                                 SW_NORMAL         // show window
                );

                if ((INT_PTR)result <= 32)
                {
                    m_logger.error("Failed to launch process. Error code: %u", (INT_PTR)result);
                }
                m_logger.info("Driver installation process launched. Code %u", (INT_PTR)result);
            });
#endif
    }
}  // namespace mab
