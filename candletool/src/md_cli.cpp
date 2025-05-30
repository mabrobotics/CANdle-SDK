#include "md_cli.hpp"
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <variant>
#include "candle.hpp"
#include "logger.hpp"
#include "mab_types.hpp"
#include "md_types.hpp"
#include "candle_types.hpp"
#include "candletool.hpp"
#include "mabFileParser.hpp"
#include "md_cfg_map.hpp"
#include "utilities.hpp"

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
                auto md = getMd(mdCanId, candleBuilder);

                std::string configFilePath = *downloadConfigOptions.configFile;
                if (configFilePath.empty())
                {
                    m_logger.error("Configuration file path is empty!");
                    return;
                }
                // If the path is not specified, prepend the standard path
                if (std::find(configFilePath.begin(), configFilePath.end(), '/') ==
                    configFilePath.end())
                {
                    configFilePath = "/etc/candletool/config/motors/" + configFilePath;
                }

                MDConfigMap cfgMap;
                for (auto& [regAddress, cfgElement] : cfgMap.m_map)
                {
                    cfgElement.m_value = registerRead(*md, regAddress).value_or("NOT FOUND");
                }
                // Write the configuration to the file
                mINI::INIFile      configFile(configFilePath);
                mINI::INIStructure ini;
                for (const auto& [regAddress, cfgElement] : cfgMap.m_map)
                {
                    ini[cfgElement.m_tomlSection.data()][cfgElement.m_tomlKey.data()] =
                        cfgElement.getReadable();
                }
                if (!configFile.generate(ini, true))
                {
                    m_logger.error("Could not write configuration to file: %s",
                                   configFilePath.c_str());
                    return;
                }
                m_logger.success("Configuration downloaded successfully to %s",
                                 configFilePath.c_str());
            });

        // Upload configuration file
        // auto* uploadConfig = config->add_subcommand("upload", "Upload configuration to MD
        // drive.");

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

    void MDCli::registerWrite(MD& md, u16 regAdress, const std::string& value)
    {
        std::string trimmedValue = trim(value);

        MDRegisters_S                             regs;
        std::variant<int64_t, float, std::string> regValue;
        bool                                      foundRegister      = false;
        bool                                      registerCompatible = false;

        // Check if the value is a string or a number
        if (trimmedValue.find_first_not_of("0123456789.f") == std::string::npos)
        {
            /// Check if the value is a float or an integer
            if (trimmedValue.find('.') != std::string::npos)
                regValue = std::stof(value);
            else
                regValue = std::stoll(value);
        }
        else
        {
            regValue = trimmedValue;
        }

        auto setRegValueByAdress = [&]<typename T>(MDRegisterEntry_S<T> reg)
        {
            if (reg.m_regAddress == regAdress)
            {
                foundRegister = true;
                if constexpr (std::is_arithmetic<T>::value)
                {
                    registerCompatible = true;
                    if (std::holds_alternative<int64_t>(regValue))
                        reg.value = std::get<int64_t>(regValue);
                    else if (std::holds_alternative<float>(regValue))
                        reg.value = std::get<float>(regValue);

                    auto result = md.writeRegisters(reg);

                    if (result != MD::Error_t::OK)
                    {
                        m_logger.error("Failed to write register %d", reg.m_regAddress);
                        return;
                    }
                    m_logger.success("Writing register %s successful!", reg.m_name.data());
                }
                else if constexpr (std::is_same<std::decay_t<T>, char*>::value)
                {
                    registerCompatible = true;
                    std::string_view strV;
                    if (std::holds_alternative<std::string>(regValue))
                        strV = std::get<std::string>(regValue).c_str();
                    else
                    {
                        m_logger.error("Invalid value type for register %d", reg.m_regAddress);
                        return;
                    }

                    if (strV.length() > sizeof(reg.value) + 1)
                    {
                        m_logger.error("Value too long for register %d", reg.m_regAddress);
                        return;
                    }

                    std::copy(strV.data(), strV.data() + strV.length(), reg.value);

                    auto result = md.writeRegisters(reg);

                    if (result != MD::Error_t::OK)
                    {
                        m_logger.error("Failed to write register %d", reg.m_regAddress);
                        return;
                    }
                    m_logger.success("Writing register %s successful!", reg.m_name.data());
                }
            }
        };
        regs.forEachRegister(setRegValueByAdress);
        if (!foundRegister)
        {
            m_logger.error("Register %d not found", regAdress);
            return;
        }
        if (!registerCompatible)
        {
            m_logger.error("Register %d not compatible with value %s", regAdress, value.c_str());
            return;
        }
    }

    std::optional<std::string> MDCli::registerRead(MD& md, u16 regAdress)
    {
        std::optional<std::string> registerStringValue;
        MDRegisters_S              regs;
        auto                       getValueByAdress = [&]<typename T>(MDRegisterEntry_S<T> reg)
        {
            if (reg.m_regAddress == regAdress)
            {
                if constexpr (std::is_arithmetic_v<T>)
                {
                    auto result = md.readRegister(reg);
                    if (result != MD::Error_t::OK)
                    {
                        m_logger.error("Failed to read register %d", regAdress);
                        return false;
                    }
                    std::string value   = std::to_string(reg.value);
                    registerStringValue = value;  // Store the value in the result
                    m_logger.success("Register %d value: %s", regAdress, value.c_str());
                    return true;
                }
                else if constexpr (std::is_same<std::decay_t<T>, char*>::value)
                {
                    auto result = md.readRegisters(reg);
                    if (result != MD::Error_t::OK)
                    {
                        m_logger.error("Failed to read register %d", regAdress);
                        return false;
                    }
                    const char* value   = reg.value;
                    registerStringValue = std::string(value);  // Store the value in the result
                    m_logger.success("Register %d value: %s", regAdress, value);
                    return true;
                }
            }
            return false;
        };
        regs.forEachRegister(getValueByAdress);
        return registerStringValue;
    }
}  // namespace mab
