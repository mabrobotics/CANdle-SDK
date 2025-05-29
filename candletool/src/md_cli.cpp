#include "md_cli.hpp"
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string_view>
#include "candle.hpp"
#include "logger.hpp"
#include "mab_types.hpp"
#include "md_types.hpp"
#include "candle_types.hpp"
#include "candletool.hpp"
#include "mabFileParser.hpp"

namespace mab
{
    MDCli::MDCli(CLI::App* rootCli, const std::shared_ptr<const CandleBuilder> candleBuilder)
    {
        if (candleBuilder == nullptr)
        {
            throw std::runtime_error("MDCli arguments can not be nullptr!");
        }
        auto*                          mdCLi   = rootCli->add_subcommand("md", "MD commands");
        const std::shared_ptr<canId_t> mdCanId = std::make_shared<canId_t>(100);
        mdCLi->add_option("<CAN_ID>", *mdCanId, "CAN ID of the MD to interact with.")->required();

        // Blink
        auto* blink = mdCLi->add_subcommand("blink", "Blink LEDs on MD drive.");
        blink->callback(
            [this, candleBuilder, mdCanId]()
            {
                auto md = getMd(mdCanId, candleBuilder);
                md->blink();
                m_logger.success("MD is blinking!");
            });

        // Can
        auto* can = mdCLi->add_subcommand(
            "can", "Configure CAN network parameters id, datarate and timeout.");

        CanOptions canOptions(can);

        can->callback(
            [this, candleBuilder, mdCanId, canOptions]()
            {
                auto          md = getMd(mdCanId, candleBuilder);
                MDRegisters_S registers;
                // download current config from md
                if (md->readRegisters(registers.canID,
                                      registers.canBaudrate,
                                      registers.canWatchdog) != MD::Error_t::OK)

                {
                    m_logger.error("Could not get can registers from MD!");
                    return;
                }

                bool canChanged = false;

                if (!canOptions.optionsMap.at("id")->empty())
                {
                    // set new can id
                    registers.canID = *canOptions.canId;
                    canChanged      = true;
                }
                if (!canOptions.optionsMap.at("datarate")->empty())
                {
                    // set new can datarate
                    auto baudrate = CandleTool::stringToBaud(*canOptions.datarate);
                    if (baudrate.has_value())
                    {
                        registers.canBaudrate = *baudrate;
                        canChanged            = true;
                    }
                    else
                    {
                        m_logger.error("Invalid CAN datarate provided!");
                        return;
                    }
                }
                if (!canOptions.optionsMap.at("timeout")->empty())
                {
                    // set new can timeout
                    registers.canWatchdog = *canOptions.timeoutMs;
                    canChanged            = true;
                }
                // Exit if nothing changed
                if (!canChanged)
                {
                    m_logger.warn("No CAN parameters changed, skipping write!");
                    return;
                }

                registers.runCanReinit = 1;  // Set flag to reinitialize CAN

                m_logger.info("New id: %d, baudrate: %d, timeout: %d ms",
                              registers.canID.value,
                              registers.canBaudrate.value,
                              registers.canWatchdog.value);
                if (md->writeRegisters(registers.canID,
                                       registers.canBaudrate,
                                       registers.canWatchdog,
                                       registers.runCanReinit) != MD::Error_t::OK)
                {
                    m_logger.error("Could not write can registers to MD!");
                    return;
                }

                if (*canOptions.save)
                {
                    m_logger.info("Saving new CAN parameters to MD...");
                    usleep(1000'000);  // Wait for the MD to reinitialize CAN
                    auto newCanId              = std::make_shared<canId_t>(registers.canID.value);
                    auto newCandleBuilder      = std::make_shared<CandleBuilder>();
                    newCandleBuilder->datarate = std::make_shared<CANdleBaudrate_E>(
                        CandleTool::intToBaud(registers.canBaudrate.value)
                            .value_or(CANdleBaudrate_E::CAN_BAUD_1M));
                    newCandleBuilder->pathOrId = candleBuilder->pathOrId;
                    newCandleBuilder->busType  = candleBuilder->busType;
                    md                         = nullptr;  // Reset the old MD instance
                    md                         = getMd(newCanId, newCandleBuilder);
                    // Save the new can parameters to the MD
                    if (md->save() != MD::Error_t::OK)
                    {
                        m_logger.error("Could not save can parameters to MD!");
                        return;
                    }
                }
                m_logger.success("MD CAN parameters updated successfully!");
            });

        // Calibration
        auto* calibration = mdCLi->add_subcommand("calibration", "Calibrate the MD drive.");

        CalibrationOptions calibrationOptions(calibration);

        calibration->callback(
            [this, candleBuilder, mdCanId, calibrationOptions]()
            {
                auto          md = getMd(mdCanId, candleBuilder);
                MDRegisters_S registers;
                // Determine if setup error are present
                auto calibrationStatus = md->getCalibrationStatus();
                if (calibrationStatus.second != MD::Error_t::OK)
                {
                    m_logger.error("Could not get calibration status from MD!");
                    return;
                }
                bool setupError =
                    calibrationStatus.first.at(MDStatus::CalibrationStatusBits::ErrorSetup).isSet();
                if (setupError)
                {
                    m_logger.error(
                        "MD setup error present, please validate your configuration file!");
                    return;
                }

                // Determine if output encoder is present
                if (md->readRegisters(registers.auxEncoder) != MD::Error_t::OK)
                {
                    m_logger.error("Could not read auxilary encoder presence from MD!");
                    return;
                }
                // Determine types of calibration that will be performed
                bool performMainEncoderCalibration = false;
                bool performAuxEncoderCalibration  = false;

                // Check if aux encoder will be calibrated
                if (*calibrationOptions.calibrationOfEncoder == "aux" ||
                    *calibrationOptions.calibrationOfEncoder == "all")
                {
                    if (registers.auxEncoder.value == 0)
                    {
                        m_logger.warn(
                            "Auxilary encoder not present, skipping aux encoder "
                            "calibration!");
                    }
                    else
                    {
                        performAuxEncoderCalibration = true;
                    }
                }

                // Check if main encoder will be calibrated
                if (*calibrationOptions.calibrationOfEncoder == "main" ||
                    *calibrationOptions.calibrationOfEncoder == "all")
                {
                    performMainEncoderCalibration = true;
                }

                // Perform main encoder calibration
                if (performMainEncoderCalibration)
                {
                    m_logger.info("Starting main encoder calibration...");
                    registers.runCalibrateCmd = 1;  // Set flag to run main encoder calibration
                    if (md->writeRegister(registers.runCalibrateCmd) != MD::Error_t::OK)
                    {
                        m_logger.error("Main encoder calibration failed!");
                        return;
                    }
                    constexpr int CALIBRATION_TIME = 40;  // seconds
                    for (int seconds = 0; seconds < CALIBRATION_TIME; seconds++)
                    {
                        m_logger.progress(static_cast<double>(seconds) /
                                          static_cast<double>(CALIBRATION_TIME));
                        usleep(1'000'000);  // Wait for the MD to calibrate, TODO: change it when
                                            // routines are ready
                    }
                    m_logger.progress(1.0f);  // Ensure progress is at 100%

                    // Check if main encoder calibration was successful
                    auto mainEncoderStatus = md->getMainEncoderStatus();
                    if (mainEncoderStatus.second != MD::Error_t::OK)
                    {
                        m_logger.error("Could not get calibration status from MD!");
                        return;
                    }
                    if (mainEncoderStatus.first.at(MDStatus::EncoderStatusBits::ErrorCalibration)
                            .isSet())
                    {
                        m_logger.error("Main encoder calibration failed!");
                        return;
                    }
                    m_logger.success("Main encoder calibration completed successfully!");
                }
                // Perform aux encoder calibration
                if (performAuxEncoderCalibration)
                {
                    m_logger.info("Starting aux encoder calibration...");
                    // get gear ratio
                    if (md->readRegister(registers.motorGearRatio))
                    {
                        m_logger.error("Could not read gear ratio from MD!");
                        return;
                    }
                    // Calibrate
                    registers.runCalibrateAuxEncoderCmd =
                        1;  // Set flag to run aux encoder calibration
                    if (md->writeRegister(registers.runCalibrateAuxEncoderCmd) != MD::Error_t::OK)
                    {
                        m_logger.error("Aux encoder calibration failed!");
                        return;
                    }

                    constexpr double AUX_CALIBRATION_TIME_COEFF = 2.8;  // seconds

                    // Calculate calibration time based on gear ratio
                    const int auxCalibrationTime =
                        static_cast<int>((1.0 / registers.motorGearRatio.value) *
                                         AUX_CALIBRATION_TIME_COEFF) +
                        AUX_CALIBRATION_TIME_COEFF;

                    for (int seconds = 0; seconds < auxCalibrationTime; seconds++)
                    {
                        m_logger.progress(static_cast<double>(seconds) /
                                          static_cast<double>(auxCalibrationTime));
                        usleep(1'000'000);  // Wait for the MD to calibrate, TODO: change it when
                                            // routines are ready
                    }
                    m_logger.progress(1.0f);  // Ensure progress is at 100%

                    // Check if aux encoder calibration was successful
                    auto auxEncoderStatus = md->getOutputEncoderStatus();
                    if (auxEncoderStatus.second != MD::Error_t::OK)
                    {
                        m_logger.error("Could not get calibration status from MD!");
                        return;
                    }
                    if (auxEncoderStatus.first.at(MDStatus::EncoderStatusBits::ErrorCalibration)
                            .isSet())
                    {
                        m_logger.error("Aux encoder calibration failed!");
                        return;
                    }
                    m_logger.success("Aux encoder calibration completed successfully!");

                    // Testing aux encoder accuracy
                    m_logger.info("Starting aux encoder accuracy test...");
                    registers.runTestAuxEncoderCmd =
                        1;  // Set flag to run aux encoder accuracy test
                    if (md->writeRegister(registers.runTestAuxEncoderCmd) != MD::Error_t::OK)
                    {
                        m_logger.error("Aux encoder accuracy test failed!");
                        return;
                    }
                    for (int seconds = 0; seconds < auxCalibrationTime; seconds++)
                    {
                        m_logger.progress(static_cast<double>(seconds) / auxCalibrationTime);
                        usleep(1'000'000);  // Wait for the MD to test, TODO: change it when
                                            // routines are ready
                    }
                    m_logger.progress(1.0f);  // Ensure progress is at 100%

                    if (md->readRegisters(registers.calAuxEncoderStdDev,
                                          registers.calAuxEncoderMinE,
                                          registers.calAuxEncoderMaxE) != MD::Error_t::OK)
                    {
                        m_logger.error("Could not read aux encoder accuracy test results!");
                        return;
                    }
                    constexpr double RAD_TO_DEG = 180.0 / M_PI;
                    m_logger.info("Aux encoder accuracy test results:");
                    m_logger.info("  Standard deviation: %.6f rad  (%.4f deg)",
                                  registers.calAuxEncoderStdDev.value,
                                  RAD_TO_DEG * registers.calAuxEncoderStdDev.value);
                    m_logger.info("  Lowest error:      %.6f rad (%.4f deg)",
                                  registers.calAuxEncoderMinE.value,
                                  RAD_TO_DEG * registers.calAuxEncoderMinE.value);
                    m_logger.info("  Highest error:      %.6f rad  (%.4f deg)",
                                  registers.calAuxEncoderMaxE.value,
                                  RAD_TO_DEG * registers.calAuxEncoderMaxE.value);
                }
            });

        // Clear
        auto* clear = mdCLi->add_subcommand("clear", "Clear MD drive errors and warnings.");

        ClearOptions clearOptions(clear);

        clear->callback(
            [this, candleBuilder, mdCanId, clearOptions]()
            {
                auto          md = getMd(mdCanId, candleBuilder);
                MDRegisters_S registers;

                if (*clearOptions.clearType == "warn" || *clearOptions.clearType == "all")
                {
                    m_logger.info("Clearing MD warnings...");
                    registers.runClearWarnings = 1;
                    if (md->writeRegisters(registers.runClearWarnings) != MD::Error_t::OK)
                    {
                        m_logger.error("Could not clear MD warnings!");
                        return;
                    }
                }
                if (*clearOptions.clearType == "err" || *clearOptions.clearType == "all")
                {
                    m_logger.info("Clearing MD errors...");
                    registers.runClearErrors = 1;
                    if (md->writeRegisters(registers.runClearErrors) != MD::Error_t::OK)
                    {
                        m_logger.error("Could not clear MD errors!");
                        return;
                    }
                }
                m_logger.success("MD errors and warnings cleared successfully!");
            });

        // Config
        auto* config = mdCLi->add_subcommand("config", "Configure MD drive.");
        // Download configuration file
        auto* downloadConfig =
            config->add_subcommand("download", "Download configuration from MD drive.");

        ConfigOptions downloadConfigOptions(downloadConfig);

        downloadConfig->callback(
            [this, candleBuilder, mdCanId, downloadConfigOptions]()
            {
                auto          md = getMd(mdCanId, candleBuilder);
                MDRegisters_S registers;

                std::string configFilePath = *downloadConfigOptions.configFile;
                if (configFilePath.empty())
                {
                    m_logger.error("Configuration file path is empty!");
                    return;
                }
                // If the path is not specified, prepend the standard path
                if (std::find(configFilePath.begin(), configFilePath.end(), '/') !=
                    configFilePath.end())
                {
                    configFilePath = "/etc/candletool/config/motors/" + configFilePath;
                }
                mINI::INIFile      iniFile(configFilePath);
                mINI::INIStructure iniStructure;
            });

        // Upload configuration file
        auto* uploadConfig = config->add_subcommand("upload", "Upload configuration to MD drive.");

        // Reset configuration
        auto* factoryReset = config->add_subcommand("factory-reset", "Factory reset the MD drive.");
        factoryReset->callback(
            [this, candleBuilder, mdCanId]()
            {
                auto          md = getMd(mdCanId, candleBuilder);
                MDRegisters_S registers;
                registers.runRestoreFactoryConfig = 1;  // Set flag to restore factory config
                if (md->writeRegister(registers.runRestoreFactoryConfig) != MD::Error_t::OK)
                {
                    m_logger.error("Could not restore factory configuration on MD!");
                    return;
                }
                m_logger.success("MD drive reset successfully!");
            });

        // // Discover
        // auto* discover = mdCLi->add_subcommand("discover", "Discover MD drives on the network.");
        // discover->callback(
        //     [this, candleBuilder, mdCanId]()
        //     {
        //         auto md = getMd(mdCanId, candleBuilder);
        //         // md->discover();
        //         logger::info("Discover command placeholder");
        //     });

        // // Info
        // auto* info = mdCLi->add_subcommand("info", "Get information about the MD drive.");
        // info->callback(
        //     [this, candleBuilder, mdCanId]()
        //     {
        //         auto md = getMd(mdCanId, candleBuilder);
        //         // md->info();
        //         logger::info("Info command placeholder");
        //     });

        // // Register
        // auto* reg = mdCLi->add_subcommand("register", "Register operations for MD drive.");
        // reg->callback(
        //     [this, candleBuilder, mdCanId]()
        //     {
        //         auto md = getMd(mdCanId, candleBuilder);
        //         // md->register();
        //         logger::info("Register command placeholder");
        //     });

        // // Test
        // auto* test = mdCLi->add_subcommand("test", "Test the MD drive.");
        // test->callback(
        //     [this, candleBuilder, mdCanId]()
        //     {
        //         auto md = getMd(mdCanId, candleBuilder);
        //         // md->test();
        //         logger::info("Test command placeholder");
        //     });

        // // Update
        // auto* update = mdCLi->add_subcommand("update", "Update firmware on MD drive.");
        // update->callback(
        //     [this, candleBuilder, mdCanId]()
        //     {
        //         auto md = getMd(mdCanId, candleBuilder);
        //         // md->update();
        //         logger::info("Update command placeholder");
        //     });
    }

    std::unique_ptr<MD, std::function<void(MD*)>> MDCli::getMd(
        const std::shared_ptr<canId_t>             mdCanId,
        const std::shared_ptr<const CandleBuilder> candleBuilder)
    {
        auto candle = candleBuilder->build().value_or(nullptr);
        candle->init();
        if (candle == nullptr)
        {
            return (nullptr);
        }
        std::function<void(MD*)> deleter = [candle](MD* ptr)
        {
            delete ptr;
            detachCandle(candle);
        };
        auto md = std::unique_ptr<MD, std::function<void(MD*)>>(new MD(*mdCanId, candle), deleter);
        if (md->init() == MD::Error_t::OK)
            return md;
        else
        {
            return nullptr;
        }
    }
}  // namespace mab
