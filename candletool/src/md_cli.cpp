#include "md_cli.hpp"
#include <cstdint>
#include <ios>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include "MDStatus.hpp"
#include "canLoader.hpp"
#include "candle.hpp"
#include "logger.hpp"
#include "mab_types.hpp"
#include "md_types.hpp"
#include "candle_types.hpp"
#include "mabFileParser.hpp"
#include "md_cfg_map.hpp"
#include "utilities.hpp"
#include "MDStatus.hpp"
#include "mini/ini.h"

/* ERROR COLORING NOTE: may not work on all terminals! */
#define REDSTART    "\033[1;31m"
#define GREENSTART  "\033[1;32m"
#define YELLOWSTART "\033[1;33m"
#define BLUESTART   "\x1b[38;5;33m"
#define RESETTEXT   "\033[0m"

#define RED__(x) REDSTART x RESETTEXT
#define RED_(x)  REDSTART + x + RESETTEXT

#define GREEN__(x) GREENSTART x RESETTEXT
#define GREEN_(x)  GREENSTART + x + RESETTEXT

#define YELLOW__(x) YELLOWSTART x RESETTEXT
#define YELLOW_(x)  YELLOWSTART + x + RESETTEXT

#define BLUE__(x) BLUESTART x RESETTEXT
#define BLUE_(x)  BLUESTART + x + RESETTEXT

namespace mab
{
    MDCli::MDCli(CLI::App* rootCli, const std::shared_ptr<const CandleBuilder> candleBuilder)
    {
        if (candleBuilder == nullptr)
        {
            throw std::runtime_error("MDCli arguments can not be nullptr!");
        }
        auto* mdCLi = rootCli->add_subcommand("md", "MD commands.")->require_subcommand();
        const std::shared_ptr<canId_t> mdCanId = std::make_shared<canId_t>(100);
        auto*                          mdCanIdOption =
            mdCLi->add_option("-i,--id", *mdCanId, "CAN ID of the MD to interact with.");

        // Blink ============================================================================
        auto* blink =
            mdCLi->add_subcommand("blink", "Blink LEDs on MD drive.")->needs(mdCanIdOption);
        blink->callback(
            [this, candleBuilder, mdCanId]()
            {
                auto md = getMd(mdCanId, candleBuilder);
                if (md == nullptr)
                {
                    m_logger.error("Coudl not connect to MD!");
                    return;
                }
                md->blink();
                m_logger.success("MD is blinking!");
            });

        // Can ============================================================================
        auto* can = mdCLi
                        ->add_subcommand(
                            "can", "Configure CAN network parameters id, datarate and timeout.")
                        ->needs(mdCanIdOption)
                        ->require_option();

        CanOptions canOptions(can);

        can->callback(
            [this, candleBuilder, mdCanId, canOptions]()
            {
                auto md = getMd(mdCanId, candleBuilder);
                if (md == nullptr)
                {
                    m_logger.error("Coudl not connect to MD!");
                    return;
                }
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
                    auto baudrate = stringToBaud(*canOptions.datarate);
                    if (baudrate.has_value())
                    {
                        registers.canBaudrate = baudToInt(baudrate.value());
                        canChanged            = true;
                    }
                    else
                    {
                        m_logger.error(
                            "Invalid CAN datarate provided! Please specify one of the "
                            "following: [1M, 2M, 5M, 8M]");
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
                        intToBaud(registers.canBaudrate.value)
                            .value_or(CANdleBaudrate_E::CAN_BAUD_1M));
                    newCandleBuilder->pathOrId = candleBuilder->pathOrId;
                    newCandleBuilder->busType  = candleBuilder->busType;
                    md                         = nullptr;  // Reset the old MD instance
                    md                         = getMd(newCanId, newCandleBuilder);
                    if (md == nullptr)
                    {
                        m_logger.error("Coudl not connect to MD!");
                        return;
                    }
                    // Save the new can parameters to the MD
                    if (md->save() != MD::Error_t::OK)
                    {
                        m_logger.error("Could not save can parameters to MD!");
                        return;
                    }
                }
                m_logger.success("MD CAN parameters updated successfully!");
            });

        // Calibration  ====================================================================
        auto* calibration =
            mdCLi->add_subcommand("calibration", "Calibrate the MD drive.")->needs(mdCanIdOption);

        CalibrationOptions calibrationOptions(calibration);

        calibration->callback(
            [this, candleBuilder, mdCanId, calibrationOptions]()
            {
                m_logger.info(
                    "Calibration requires about 20W of power, please ensure that the "
                    "power supply is sufficient!");
                m_logger.warn(
                    "The motor will rotate during calibration, "
                    "are you sure you want to proceed?");
                std::string answer;
                std::cout << "Type 'Y' to continue: ";
                std::getline(std::cin, answer);
                if (answer != "Y" && answer != "y")
                {
                    m_logger.error("Calibration aborted by user!");
                    return;
                }
                auto md = getMd(mdCanId, candleBuilder);
                if (md == nullptr)
                {
                    m_logger.error("Coudl not connect to MD!");
                    return;
                }
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
                    if (md->readRegister(registers.motorGearRatio) != MD::Error_t::OK)
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
                    if (*calibrationOptions.runTests)
                    {
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
                }
                auto quickStatus = md->getQuickStatus().first;
                if (quickStatus.at(MDStatus::QuickStatusBits::CalibrationEncoderStatus))
                {
                    m_logger.error("Calibration failed!");
                    auto calibrationStatus = md->getCalibrationStatus().first;
                    if (calibrationStatus.at(
                            MDStatus::CalibrationStatusBits::ErrorOffsetCalibration))
                        m_logger.error("Offset calibration failed!");
                    if (calibrationStatus.at(MDStatus::CalibrationStatusBits::ErrorInductance))
                        m_logger.error("Inductance measurement failed!");
                    if (calibrationStatus.at(MDStatus::CalibrationStatusBits::ErrorResistance))
                        m_logger.error("Resistance measurement failed!");
                    if (calibrationStatus.at(
                            MDStatus::CalibrationStatusBits::ErrorPolePairDetection))
                        m_logger.error("Pole pair detection failed!");
                }
                else if (quickStatus.at(MDStatus::QuickStatusBits::MainEncoderStatus) ||
                         quickStatus.at(MDStatus::QuickStatusBits::OutputEncoderStatus))
                {
                    m_logger.debug("Calibration failed at %s encoder error correction",
                                   quickStatus.at(MDStatus::QuickStatusBits::OutputEncoderStatus)
                                       ? "output"
                                       : "main");
                }
            });

        // Clear  =========================================================================
        auto* clear = mdCLi->add_subcommand("clear", "Clear MD drive errors and warnings.")
                          ->needs(mdCanIdOption);

        ClearOptions clearOptions(clear);

        clear->callback(
            [this, candleBuilder, mdCanId, clearOptions]()
            {
                auto md = getMd(mdCanId, candleBuilder);
                if (md == nullptr)
                {
                    m_logger.error("Coudl not connect to MD!");
                    return;
                }
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

        // Config  ===========================================================
        auto* config = mdCLi->add_subcommand("config", "Configure MD drive.")
                           ->needs(mdCanIdOption)
                           ->require_subcommand();
        // Download configuration file
        auto* downloadConfig =
            config->add_subcommand("download", "Download configuration from MD drive.");

        ConfigOptions downloadConfigOptions(downloadConfig);

        downloadConfig->callback(
            [this, candleBuilder, mdCanId, downloadConfigOptions]()
            {
                auto md = getMd(mdCanId, candleBuilder);
                if (md == nullptr)
                {
                    m_logger.error("Coudl not connect to MD!");
                    return;
                }

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
        auto* uploadConfig = config->add_subcommand("upload", "Upload configuration to MD drive.")
                                 ->needs(mdCanIdOption);

        ConfigOptions uploadConfigOptions(uploadConfig);

        uploadConfig->callback(
            [this, candleBuilder, mdCanId, uploadConfigOptions]()
            {
                auto md = getMd(mdCanId, candleBuilder);
                if (md == nullptr)
                {
                    m_logger.error("Coudl not connect to MD!");
                    return;
                }

                std::string configFilePath = *uploadConfigOptions.configFile;
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

                mINI::INIFile      configFile(configFilePath);
                mINI::INIStructure ini;
                if (!configFile.read(ini))
                {
                    m_logger.error("Could not read configuration file: %s", configFilePath.c_str());
                    return;
                }

                MDConfigMap cfgMap;

                for (auto& [address, toml] : cfgMap.m_map)
                {
                    auto it = ini[toml.m_tomlSection.data()][toml.m_tomlKey.data()];
                    if (it.empty())
                    {
                        m_logger.warn("Key %s.%s not found in configuration file. Skipping.",
                                      toml.m_tomlSection.data(),
                                      toml.m_tomlKey.data());
                        continue;
                    }
                    if (!toml.setFromReadable(it))
                    {
                        m_logger.error("Could not set value for %s.%s",
                                       toml.m_tomlSection.data(),
                                       toml.m_tomlKey.data());
                        return;
                    }
                    // Write the value to the MD
                    registerWrite(*md, address, toml.m_value);
                }

                if (md->save() != MD::Error_t::OK)
                {
                    m_logger.error("Could not save configuration!");
                    return;
                }
                m_logger.success("Uploaded configuration to the MD!");
            });

        // Reset configuration
        auto* factoryReset = config->add_subcommand("factory-reset", "Factory reset the MD drive.")
                                 ->needs(mdCanIdOption);
        factoryReset->callback(
            [this, candleBuilder, mdCanId]()
            {
                auto md = getMd(mdCanId, candleBuilder);
                if (md == nullptr)
                {
                    m_logger.error("Coudl not connect to MD!");
                    return;
                }
                MDRegisters_S registers;
                registers.runRestoreFactoryConfig = 1;  // Set flag to restore factory config
                if (md->writeRegister(registers.runRestoreFactoryConfig) != MD::Error_t::OK)
                {
                    m_logger.error("Could not restore factory configuration on MD!");
                    return;
                }
                m_logger.success("MD drive reset successfully!");
            });

        // Discover ============================================================================
        auto* discover = mdCLi
                             ->add_subcommand("discover",
                                              "Discover MD drives on the"
                                              "network.")
                             ->excludes(mdCanIdOption);

        discover->callback(
            [this, candleBuilder]()
            {
                auto candleOpt = candleBuilder->build();
                if (!candleOpt.has_value())
                {
                    m_logger.error("Could not initialize Candle!");
                    return;
                }
                Candle* candle = candleOpt.value();
                auto    ids    = MD::discoverMDs(candle);

                if (ids.empty())
                {
                    m_logger.error("No MD found on the bus for baud %s",
                                   datarateToString(*(candleBuilder->datarate))
                                       .value_or("NOT A DATARATE")
                                       .c_str());
                }
                else
                {
                    m_logger.info("Found following MDs:");
                }
                for (const auto& id : ids)
                {
                    m_logger.info("- %d", id);
                }
                detachCandle(candle);
            });
        // Save ============================================================================
        auto* save = mdCLi->add_subcommand("save", "Save MD drive configuration to flash memory.")
                         ->needs(mdCanIdOption);
        save->callback(
            [this, candleBuilder, mdCanId]()
            {
                auto md = getMd(mdCanId, candleBuilder);
                if (md == nullptr)
                {
                    m_logger.error("Coudl not connect to MD!");
                    return;
                }
                if (md->save() != MD::Error_t::OK)
                {
                    m_logger.error("Could not save MD configuration!");
                    return;
                }
                m_logger.success("MD configuration saved successfully!");
            });

        // Info ============================================================================
        auto* info = mdCLi->add_subcommand("info", "Get information about the MD drive.")
                         ->needs(mdCanIdOption);
        info->callback(
            [this, candleBuilder, mdCanId]()
            {
                auto          md = getMd(mdCanId, candleBuilder);
                MDRegisters_S readableRegisters;

                auto readReadableRegs = [&]<typename T>(MDRegisterEntry_S<T>& reg)
                {
                    // TODO: skipping new registers for now
                    if ((reg.m_regAddress < 0x800 && reg.m_regAddress > 0x700) ||
                        reg.m_regAddress > 0x810)
                        return;
                    if (reg.m_accessLevel != RegisterAccessLevel_E::WO)
                    {
                        auto fault = md->readRegisters(reg);
                        if (fault != MD::Error_t::OK)
                            m_logger.error("Error while reading register %s", reg.m_name.data());
                    }
                };

                readableRegisters.forEachRegister(readReadableRegs);

                m_logger << std::fixed;
                m_logger << "Drive " << *mdCanId << ":" << std::endl;
                m_logger << "- actuator name: " << readableRegisters.motorName.value << std::endl;
                m_logger << "- CAN speed: " << readableRegisters.canBaudrate.value / 1000000 << " M"
                         << std::endl;
                m_logger << "- CAN termination resistor: "
                         << ((readableRegisters.canTermination.value == true) ? "enabled"
                                                                              : "disabled")
                         << std::endl;
                m_logger << "- gear ratio: " << std::setprecision(5)
                         << readableRegisters.motorGearRatio.value << std::endl;
                mab::version_ut firmwareVersion = {{0, 0, 0, 0}};
                firmwareVersion.i               = readableRegisters.firmwareVersion.value;
                m_logger << "- firmware version: v" << (int)firmwareVersion.s.major << "."
                         << (int)firmwareVersion.s.minor << "." << (int)firmwareVersion.s.revision
                         << "." << firmwareVersion.s.tag << std::endl;
                m_logger << "- hardware version(legacy): " << "Not implemented yet" << std::endl;
                m_logger << "- hardware type: "
                         << MDLegacyHwVersion_S::toReadable(
                                readableRegisters.legacyHardwareVersion.value)
                                .value_or("Unknown")
                         << std::endl;
                m_logger << "- build date: "
                         << MDBuildDateValue_S::toReadable(readableRegisters.buildDate.value)
                                .value_or("Unknown")
                         << std::endl;
                m_logger << "- commit hash: " << readableRegisters.commitHash.value
                         << std::endl;  // TODO: printable format
                m_logger << "- max current: " << std::setprecision(1)
                         << readableRegisters.motorIMax.value << " A" << std::endl;
                m_logger << "- bridge type: " << std::to_string(readableRegisters.bridgeType.value)
                         << std::endl;
                m_logger << "- shunt resistance: " << std::setprecision(4)
                         << readableRegisters.shuntResistance.value << " Ohm" << std::endl;
                m_logger << "- pole pairs: "
                         << std::to_string(readableRegisters.motorPolePairs.value) << std::endl;
                m_logger << "- KV rating: " << std::to_string(readableRegisters.motorKV.value)
                         << " rpm/V" << std::endl;
                m_logger << "- motor shutdown temperature: "
                         << std::to_string(readableRegisters.motorShutdownTemp.value) << " *C"
                         << std::endl;
                m_logger << "- motor calibration mode: "
                         << MDAuxEncoderCalibrationModeValue_S::toReadable(
                                readableRegisters.auxEncoderCalibrationMode.value)
                                .value_or("Unknown")
                         << std::endl;
                m_logger << "- motor torque constant: " << std::setprecision(4)
                         << readableRegisters.motorKt.value << " Nm/A" << std::endl;
                m_logger << std::fixed << "- d-axis resistance: " << std::setprecision(3)
                         << readableRegisters.motorResistance.value << " Ohm\n";
                m_logger << std::fixed << "- d-axis inductance: " << std::setprecision(6)
                         << readableRegisters.motorInductance.value << " H\n";
                m_logger << "- torque bandwidth: " << readableRegisters.motorTorqueBandwidth.value
                         << " Hz" << std::endl;
                m_logger << "- CAN watchdog: " << readableRegisters.canWatchdog.value << " ms"
                         << std::endl;
                m_logger << "- GPIO mode: "
                         << MDUserGpioConfigurationValue_S::toReadable(
                                readableRegisters.userGpioConfiguration.value)
                                .value_or("Unknown")
                         << std::endl;

                m_logger << "- auxilary encoder: "
                         << MDAuxEncoderValue_S::toReadable(readableRegisters.auxEncoder.value)
                                .value_or("Unknown")
                         << std::endl;

                if (readableRegisters.auxEncoder.value != 0)
                {
                    m_logger << "   - aux encoder mode: "
                             << MDAuxEncoderModeValue_S::toReadable(
                                    readableRegisters.auxEncoderMode.value)
                                    .value_or("Unknown")
                             << std::endl;
                    m_logger << "   - aux encoder calibration mode: "
                             << MDAuxEncoderCalibrationModeValue_S::toReadable(
                                    readableRegisters.auxEncoderCalibrationMode.value)
                                    .value_or("Unknown")
                             << std::endl;
                    m_logger << "   - aux encoder position: "
                             << readableRegisters.auxEncoderPosition.value << " rad" << std::endl;
                    m_logger << "   - aux encoder velocity: "
                             << readableRegisters.auxEncoderVelocity.value << " rad/s" << std::endl;
                }

                m_logger << "- motion limits: " << std::endl;
                m_logger << "   - max torque: " << std::setprecision(2)
                         << readableRegisters.maxTorque.value << " Nm" << std::endl;
                m_logger << "   - max acceleration: " << std::setprecision(2)
                         << readableRegisters.maxAcceleration.value << " rad/s^2" << std::endl;
                m_logger << "   - max deceleration: " << std::setprecision(2)
                         << readableRegisters.maxDeceleration.value << " rad/s^2" << std::endl;
                m_logger << "   - max velocity: " << std::setprecision(2)
                         << readableRegisters.maxVelocity.value << " rad/s" << std::endl;
                m_logger << "   - position limit min: " << std::setprecision(2)
                         << readableRegisters.positionLimitMin.value << " rad" << std::endl;
                m_logger << "   - position limit max: " << std::setprecision(2)
                         << readableRegisters.positionLimitMax.value << " rad" << std::endl;

                m_logger << "- position: " << std::setprecision(2)
                         << readableRegisters.mainEncoderPosition.value << " rad" << std::endl;
                m_logger << "- velocity: " << std::setprecision(2)
                         << readableRegisters.mainEncoderVelocity.value << " rad/s" << std::endl;
                m_logger << "- torque: " << std::setprecision(2)
                         << readableRegisters.motorTorque.value << " Nm" << std::endl;
                m_logger << "- MOSFET temperature: " << std::fixed << std::setprecision(2)
                         << readableRegisters.mosfetTemperature.value << " *C" << std::endl;
                m_logger << "- motor temperature: " << std::fixed << std::setprecision(2)
                         << readableRegisters.motorTemperature.value << " *C" << std::endl;
                m_logger << std::endl;
                auto statusToString =
                    []<typename T>(
                        const std::unordered_map<T, MDStatus::StatusItem_S> statusItemList)
                    -> std::string
                {
                    std::string result;
                    for (const auto& [key, item] : statusItemList)
                    {
                        if (item.isSet())
                        {
                            if (!result.empty())
                                result += ", ";
                            if (item.isError)
                                result += RED_(item.name);
                            else
                                result += YELLOW_(item.name);
                        }
                    }
                    if (result.empty())
                        result = GREEN__("OK");
                    else
                        result = "Set flags: " + result;
                    return result;
                };

                m_logger << "***** ERRORS *****" << std::endl;
                m_logger << "- main encoder error: 	0x" << std::hex
                         << readableRegisters.mainEncoderStatus.value << std::dec << " ("
                         << statusToString(md->getMainEncoderStatus().first) << ")" << std::endl;

                if (readableRegisters.auxEncoder.value != 0)
                {
                    m_logger << "- aux encoder status: 	0x" << std::hex
                             << readableRegisters.auxEncoderStatus.value << std::dec << " ("
                             << statusToString(md->getOutputEncoderStatus().first) << ")"
                             << std::endl;
                }

                m_logger << "- calibration status: 	0x" << std::hex
                         << readableRegisters.calibrationStatus.value << std::dec << " ("
                         << statusToString(md->getCalibrationStatus().first) << ")" << std::endl;

                m_logger << "- bridge status: 	0x" << std::hex
                         << readableRegisters.bridgeStatus.value << std::dec << " ("
                         << statusToString(md->getBridgeStatus().first) << ")" << std::endl;

                m_logger << "- hardware status: 	0x" << std::hex
                         << readableRegisters.hardwareStatus.value << std::dec << " ("
                         << statusToString(md->getHardwareStatus().first) << ")" << std::endl;

                m_logger << "- communication status:  0x" << std::hex
                         << readableRegisters.communicationStatus.value << std::dec << " ("
                         << statusToString(md->getCommunicationStatus().first) << ")" << std::endl;

                m_logger << "- motion status: 	0x" << std::hex
                         << readableRegisters.motionStatus.value << std::dec << " ("
                         << statusToString(md->getMotionStatus().first) << ")" << std::endl;
            });

        // Register =======================================================================
        auto* reg = mdCLi->add_subcommand("register", "Register operations for MD drive.")
                        ->needs(mdCanIdOption)
                        ->require_subcommand();

        // Register read
        auto regRead = reg->add_subcommand("read", "Read register value.");

        RegisterReadOption regReadOptions(regRead);

        regRead->callback(
            [this, candleBuilder, mdCanId, regReadOptions]()
            {
                std::string result      = "Failed to read";
                auto        md          = getMd(mdCanId, candleBuilder);
                u16         address     = 0x0;
                std::string registerStr = *(regReadOptions.registerAddressOrName);
                if (md == nullptr)
                {
                    m_logger.error("Coudl not connect to MD!");
                    return;
                }
                if (std::string("0x").compare(registerStr.substr(0, 2)) == 0)
                {
                    address = std::stoll(registerStr, nullptr, 16);
                    if (address != 0x0)
                        result = registerRead(*md, address).value_or(result);
                    else
                    {
                        m_logger.error("Could not find provided register!");
                        return;
                    }
                }
                else
                {
                    MDRegisters_S regs;
                    auto          findAddressByName = [&]<typename T>(MDRegisterEntry_S<T> reg)
                    {
                        if (registerStr.compare(reg.m_name) == 0)
                            address = reg.m_regAddress;
                    };
                    regs.forEachRegister(findAddressByName);
                    if (address != 0x0)
                    {
                        result = registerRead(*md, address).value_or(result);
                    }
                    else
                    {
                        m_logger.error("Could not find provided register!");
                        return;
                    }
                }
                m_logger.success(
                    "Register %s has a value of %s", registerStr.c_str(), result.c_str());
            });

        // Register write
        auto regWrite = reg->add_subcommand("write", "Write register value to MD");

        RegisterWriteOption registerWriteOption(regWrite);

        regWrite->callback(
            [this, candleBuilder, mdCanId, registerWriteOption]()
            {
                auto        md           = getMd(mdCanId, candleBuilder);
                std::string resultBefore = "0";
                std::string resultAffter = "0";
                u16         address      = 0x0;
                std::string registerStr  = *(registerWriteOption.registerAddressOrName);
                bool        isWriteOnly  = false;

                if (md == nullptr)
                {
                    m_logger.error("Coudl not connect to MD!");
                    return;
                }
                if (std::string("0x").compare(registerStr.substr(0, 2)) == 0)
                {
                    MDRegisters_S regs;
                    address = std::stoll(registerStr, nullptr, 16);
                    // Is write-only check
                    auto isWriteOnlyFunc = [&]<typename T>(MDRegisterEntry_S<T> reg)
                    {
                        if (reg.m_regAddress == address)
                        {
                            isWriteOnly = reg.m_accessLevel == RegisterAccessLevel_E::WO;
                        }
                    };
                    regs.forEachRegister(isWriteOnlyFunc);

                    if (address != 0x0 && !isWriteOnly)
                        resultBefore = registerRead(*md, address).value_or(resultBefore);
                    else if (address != 0x0 && isWriteOnly)
                        m_logger.info("Skipping read check for read-only variable...");
                    else
                    {
                        m_logger.error("Could not find provided register!");
                        return;
                    }
                }
                else
                {
                    MDRegisters_S regs;
                    auto          findAddressByName = [&]<typename T>(MDRegisterEntry_S<T> reg)
                    {
                        if (registerStr.compare(reg.m_name) == 0)
                        {
                            address     = reg.m_regAddress;
                            isWriteOnly = reg.m_accessLevel == RegisterAccessLevel_E::WO;
                        }
                    };
                    regs.forEachRegister(findAddressByName);
                    if (address != 0x0 && !isWriteOnly)
                    {
                        m_logger.info("Is RO - %u", isWriteOnly);
                        resultBefore = registerRead(*md, address).value_or(resultBefore);
                    }
                    else if (address != 0x0 && isWriteOnly)
                        m_logger.debug("Skipping read check for read-only variable...");
                    else
                    {
                        m_logger.error("Could not find provided register!");
                        return;
                    }
                }
                if (registerWrite(*md, address, *registerWriteOption.registerValue) == false)
                {
                    m_logger.error("Could not parse value to the MD register!");
                }
                if (!isWriteOnly)
                {
                    resultAffter = registerRead(*md, address).value_or("Failed to read");
                    m_logger.success(
                        "Written value to the register %s which had a value of %s, and now has a "
                        "value "
                        "of %s",
                        registerStr.c_str(),
                        resultBefore.c_str(),
                        resultAffter.c_str());
                }
                else
                {
                    m_logger.debug("Skipping read check for read-only variable...");
                    m_logger.success("Writen value to write-only register %s", registerStr.c_str());
                }
            });
        // Reset
        mdCLi->add_subcommand("reset", "Reboot the MD drive")
            ->callback(
                [this, candleBuilder, mdCanId]()
                {
                    auto md = getMd(mdCanId, candleBuilder);
                    if (md == nullptr)
                    {
                        m_logger.error("Coudl not connect to MD!");
                        return;
                    }
                    md->reset();
                    m_logger.success("MD drive reset");
                });

        // Test
        auto* test = mdCLi->add_subcommand("test", "Test the MD drive movement.")
                         ->needs(mdCanIdOption)
                         ->require_subcommand();

        // Absolute
        auto* absolute =
            test->add_subcommand("absolute", "Move to target utilizing trapezoidal profile")
                ->require_option();
        TestOptions absoluteTestOptions(absolute);

        absolute->callback(
            [this, candleBuilder, mdCanId, absoluteTestOptions]()
            {
                auto md = getMd(mdCanId, candleBuilder);
                if (md == nullptr)
                {
                    m_logger.error("Coudl not connect to MD!");
                    return;
                }
                if (md->isMDError(md->setTargetPosition(*absoluteTestOptions.target)))
                    return;

                if (md->isMDError(md->setMotionMode(mab::MdMode_E::POSITION_PROFILE)))
                    return;

                if (md->isMDError(md->enable()))
                    return;

                while (!(md->getQuickStatus()
                             .first.at(MDStatus::QuickStatusBits::TargetPositionReached)
                             .isSet()))
                {
                    m_logger.info("Position: %4.2f", md->getPosition().first);
                    usleep(50'000);
                }
                md->disable();
                m_logger.info("TARGET REACHED!");
            });

        // Relative
        auto* relative =
            test->add_subcommand("relative", "Move relative to current position")->require_option();

        TestOptions relativeTestOptions(relative);

        relative->callback(
            [this, candleBuilder, mdCanId, relativeTestOptions]()
            {
                auto md = getMd(mdCanId, candleBuilder);
                if (md == nullptr)
                {
                    m_logger.error("Coudl not connect to MD!");
                    return;
                }
                if (*relativeTestOptions.target > 10.0f)
                    *relativeTestOptions.target = 10.0f;
                else if (*relativeTestOptions.target < -10.0f)
                    *relativeTestOptions.target = -10.0f;

                md->setMotionMode(mab::MdMode_E::IMPEDANCE);
                f32 pos = md->getPosition().first;
                md->setTargetPosition(pos);
                *relativeTestOptions.target += pos;
                md->enable();

                for (f32 t = 0.f; t < 1.f; t += 0.01f)
                {
                    f32 target = std::lerp(pos, *relativeTestOptions.target, t);
                    md->setTargetPosition(target);
                    m_logger.info("Position: %4.2f, Velocity: %4.1f",
                                  md->getPosition().first,
                                  md->getVelocity().first);
                    usleep(30000);
                }
                m_logger.success("Movement ended.");
            });

        // Update
        auto* update =
            mdCLi->add_subcommand("update", "Update firmware on MD drive.")->needs(mdCanIdOption);
        UpdateOptions updateOptions(update);

        update->callback(
            [this, candleBuilder, mdCanId, updateOptions]()
            {
                MabFileParser mabFile(*updateOptions.pathToMabFile,
                                      MabFileParser::TargetDevice_E::MD);

                if (*(updateOptions.recovery) == false)
                {
                    auto md = getMd(mdCanId, candleBuilder);
                    if (md == nullptr)
                    {
                        m_logger.error("Could not communicate with MD device with ID %d", *mdCanId);
                        return;
                    }
                    md->reset();
                    usleep(300'000);
                }
                else
                {
                    m_logger.warn("Recovery mode...");
                    m_logger.warn("Please make sure driver is in the bootload phase (rebooting)");
                }
                auto candle = candleBuilder->build().value_or(nullptr);
                if (candle == nullptr)
                {
                    m_logger.error("Could not connect to candle!");
                    return;
                }
                CanLoader canLoader(candle, &mabFile, *mdCanId);
                if (canLoader.flashAndBoot())
                {
                    m_logger.success("Update complete for MD @ %d", *mdCanId);
                }
                else
                {
                    m_logger.error("MD flashing failed!");
                }
            });
        // Version
        auto* version = mdCLi->add_subcommand("version", "Check version of the MD device.");

        version->callback(
            [this, candleBuilder, mdCanId]()
            {
                auto md = getMd(mdCanId, candleBuilder);
                if (md == nullptr)
                {
                    m_logger.error("Coudl not connect to MD!");
                    return;
                }
                MDRegisters_S regs;
                if (md->readRegisters(regs.firmwareVersion, regs.legacyHardwareVersion) !=
                    MD::Error_t::OK)
                {
                    m_logger.error("Could not read fw and hw versions!");
                    return;
                }
                version_ut fwVersion;
                fwVersion.i = regs.firmwareVersion.value;
                m_logger.info("FW version: %d.%d.%d(%c)",
                              fwVersion.s.major,
                              fwVersion.s.minor,
                              fwVersion.s.revision,
                              fwVersion.s.tag);
                m_logger.info("Legacy hardware version: %s",
                              MDLegacyHwVersion_S::toReadable(regs.legacyHardwareVersion.value)
                                  .value_or("Unknown")
                                  .c_str());
            });
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

    bool MDCli::registerWrite(MD& md, u16 regAdress, const std::string& value)
    {
        std::string trimmedValue = trim(value);

        MDRegisters_S                             regs;
        std::variant<int64_t, float, std::string> regValue;
        bool                                      foundRegister      = false;
        bool                                      registerCompatible = false;

        // Check if the value is a string or a number
        if (trimmedValue.find_first_not_of("-0123456789.f") == std::string::npos)
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
            return false;
        }
        if (!registerCompatible)
        {
            m_logger.error("Register %d not compatible with value %s", regAdress, value.c_str());
            return false;
        }
        return true;
    }

    std::optional<std::string> MDCli::registerRead(MD& md, u16 regAdress)
    {
        std::optional<std::string> registerStringValue;
        MDRegisters_S              regs;
        std::string                nameOfRegister   = "NULL";
        auto                       getValueByAdress = [&]<typename T>(MDRegisterEntry_S<T> reg)
        {
            if (reg.m_regAddress == regAdress)
            {
                nameOfRegister = reg.m_name;
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
                    m_logger.success(
                        "Register %s value = %s", nameOfRegister.c_str(), value.c_str());
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
                    m_logger.success("Register %s value = %s", nameOfRegister.c_str(), value);
                    return true;
                }
            }
            return false;
        };
        regs.forEachRegister(getValueByAdress);
        return registerStringValue;
    }
}  // namespace mab
