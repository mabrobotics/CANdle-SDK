#include "candletool/candle_cli.hpp"

#include <memory>
#include <vector>

#include "candle/communication_device/candle.hpp"
#include "candletool/mabFileParser.hpp"
#include "candle/communication_device/candle_bootloader.hpp"
#include "candletool/mab_crc.hpp"
#include "candletool/web_file_module/curl_handler.hpp"
#include "candletool/web_file_module/web_file.hpp"
#include "candletool/web_file_module/flasher.hpp"
#include "candletool/utilities.hpp"

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
            [this, updateOptions, ctx]()
            {
                m_logger.info("Performing Candle firmware update.");

                std::filesystem::path filepath;

                if (updateOptions.pathToMabFile->empty())
                {
                    if (updateOptions.fwVersion->empty())
                    {
                        m_logger.error(
                            "Please provide version of fw or \"latest\" keyword in the argument!");
                        m_logger.error("For example candletool candle update latest");
                        return;
                    }
                    std::string fallbackPath = ctx.packageEtcPath->generic_string();

                    if (!updateOptions.metadataFile->empty())
                        fallbackPath = *updateOptions.metadataFile;
                    else
                        fallbackPath += "/config/web_files_metadata.ini";

                    m_logger.debug("Fallback path at: %s", fallbackPath.c_str());
                    mINI::INIFile fallbackMetadataFile(fallbackPath);
                    CurlHandler   curl(fallbackMetadataFile);

                    std::string fileId = "MAB_USB_MABFILE_";
                    fileId += *updateOptions.fwVersion;
                    auto curlResult = curl.downloadFile(fileId);
                    if (curlResult.first != CurlHandler::CurlError_E::OK)
                    {
                        m_logger.error("Error on curl download request!");
                        return;
                    }
                    auto file = curlResult.second;
                    if (file.m_type != WebFile_S::Type_E::MAB_FILE)
                    {
                        m_logger.error("Downloaded file is not a MAB file!");
                        return;
                    }
                    filepath = curlResult.second.m_path;
                    m_logger.info("Downloaded firmware to %s", filepath.c_str());
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
