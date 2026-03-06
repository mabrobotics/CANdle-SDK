#include "mdco_cli.hpp"
#include <fcntl.h>
#include <array>
#include <chrono>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <memory>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include "CLI/CLI.hpp"
#include "MDCO.hpp"
#include "candle.hpp"
#include "candle_types.hpp"
#include "edsEntry.hpp"
#include "edsParser.hpp"
#include "mab_types.hpp"
#include "md_cfg_map.hpp"
#include "mini/ini.h"
#include "mdco_config_adapter.hpp"

using namespace mab;

std::string MdcoCli::validateAndGetFinalConfigPath(const std::filesystem::path& cfgPath)
{
    // Check if the file exists, if not, check if it exists relative to the default config
    std::filesystem::path finalConfigPath = cfgPath;
    std::filesystem::path pathRelToDefaultConfig =
        getMotorsConfigPath().string() + cfgPath.string();
    // Check if the file exists
    if (!fileExists(finalConfigPath))
    {
        if (!fileExists(pathRelToDefaultConfig))
        {
            m_log.error("Neither \"%s\", nor \"%s\", exists!.",
                        cfgPath.c_str(),
                        pathRelToDefaultConfig.c_str());
            exit(1);
        }
        finalConfigPath = pathRelToDefaultConfig;
    }
    // Check if the file is a valid config file
    if (!isConfigValid(finalConfigPath))
    {
        m_log.error("\"%s\" in not a valid motor .cfg file.", finalConfigPath.c_str());
        m_log.warn("Valid file must have .cfg extension, and size of < 1MB");
        exit(1);
    }
    std::filesystem::path defaultConfigPath =
        finalConfigPath.string().substr(0, finalConfigPath.string().find_last_of('/') + 1) +
        "default.cfg";

    // If default config does not exist, warn the user
    if (!fileExists(defaultConfigPath))
    {
        m_log.warn("No default config found at expected location \"%s\"",
                   defaultConfigPath.c_str());
        m_log.warn("Cannot check completeness of the config file. Proceed with upload? [y/n]");
        if (!getConfirmation())
            exit(0);
    }
    // If default config exists, check if the user config is complete, if not, offer to generate
    // missing parts
    if (fileExists(defaultConfigPath) && !isCanOpenConfigComplete(finalConfigPath))
    {
        m_log.m_layer = Logger::ProgramLayer_E::TOP;
        m_log.error("\"%s\" is incomplete.", finalConfigPath.c_str());
        m_log.info("Generate updated file with all required fields? [y/n]");
        if (getConfirmation())
        {
            finalConfigPath = generateUpdatedConfigFile(finalConfigPath.string());
            m_log.info("Generated updated file \"%s\"", finalConfigPath.c_str());
        }
        else
            m_log.info("Proceeding with original file \"%s\"", finalConfigPath.c_str());
    }
    return finalConfigPath.string();
}

void MdcoCli::clean(std::string& s)
{
    size_t write_pos    = 0;
    bool   seenNonSpace = false;
    for (size_t read_pos = 0; read_pos < s.size(); ++read_pos)
    {
        unsigned char c = s[read_pos];
        if (std::isspace(c))
        {
            if (!seenNonSpace)
                continue;  // skip leading spaces
            continue;      // skip all spaces (internal & trailing)
        }
        seenNonSpace   = true;
        s[write_pos++] = std::tolower(c);  // pass all character in lower case
    }
    s.resize(write_pos);
}

bool MdcoCli::isCanOpenConfigComplete(const std::filesystem::path& pathToConfig)
{
    mINI::INIFile      defaultFile(getMotorsConfigPath() / "CANopen/default.cfg");
    mINI::INIStructure defaultIni;
    defaultFile.read(defaultIni);

    mINI::INIFile      userFile(pathToConfig);
    mINI::INIStructure userIni;
    userFile.read(userIni);

    // Loop fills all lacking fields in the user's config file.
    for (auto const& it : defaultIni)
    {
        auto const& section    = it.first;
        auto const& collection = it.second;
        for (auto const& it2 : collection)
        {
            auto const& key = it2.first;
            if (!userIni[section].has(key))
                return false;
        }
    }
    return true;
}

std::unique_ptr<MDCO, std::function<void(MDCO*)>> MdcoCli::getMdco(
    const std::shared_ptr<canId_t> mdCanId, std::shared_ptr<EDSObjectDictionary> od)
{
    m_candleBuilder->useCAN20Frames = true;
    auto candle                     = m_candleBuilder->build().value_or(nullptr);
    if (candle == nullptr)
    {
        return (nullptr);
    }
    candle->init();
    std::function<void(MDCO*)> deleter = [candle](MDCO* ptr)
    {
        delete ptr;
        detachCandle(candle);
    };
    auto md =
        std::unique_ptr<MDCO, std::function<void(MDCO*)>>(new MDCO(*mdCanId, candle, od), deleter);
    if (md->init() == MDCO::Error_t::OK)
        return md;
    else
    {
        m_log.error("Could not connect to MD!");
        return nullptr;
    }
}

MdcoCli::MdcoCli(CLI::App& rootCli, CANdleToolCtx_S ctx) : m_rootCli(rootCli), m_ctx(ctx)
{
    m_log.m_tag     = "MDCO";
    m_log.m_layer   = Logger::ProgramLayer_E::TOP;
    m_candleBuilder = m_ctx.candleBranchVec.at(0).candleBuilder;

    std::filesystem::path configFilePath =
        m_ctx.packageEtcPath->string() + "/config/candletool.ini";

    if (!std::filesystem::exists(configFilePath))
    {
        m_log.error(
            "Coudl not locate candletool.ini configuration file in %s. Is the candletool installed "
            "properly?",
            configFilePath.c_str());
        throw std::runtime_error(
            "Coudl not locate candletool.ini configuration file. Is the candletool installed "
            "properly?");
    }

    mINI::INIFile      configFile(configFilePath);
    mINI::INIStructure configStruct;

    configFile.read(configStruct);

    std::filesystem::path edsPath = configStruct["eds"]["path"];

    if (edsPath.empty() || !std::filesystem::exists(edsPath))
    {
        m_log.error(
            "Coudl not locate .eds file. Please check the %s file for eds section and fill it "
            "properly. Currently read path is: %s",
            configFilePath.c_str(),
            edsPath.c_str());
        throw std::runtime_error(
            "Coudl not locate .eds file. Please check the config file for eds section and fill it "
            "properly.");
    }

    auto odPair = EDSParser::load(edsPath);
    if (odPair.second != EDSParser::Error_t::OK)
    {
        m_log.error("EDS parsing failed!");
        throw std::runtime_error("EDS parsing failed!");
    }
    auto od = odPair.first;

    CLI::App* mdco = m_rootCli.add_subcommand("mdco", "Send CANopen command instead of CAN FD.");
    const std::shared_ptr<canId_t> mdCanId = std::make_shared<canId_t>(10);
    auto*                          mdCanIdOption =
        mdco->add_option("-i,--id", *mdCanId, "CAN ID of the MD to interact with.");

    // BLINK ============================================================================

    CLI::App* blink =
        mdco->add_subcommand("blink", "Blink LEDs on MD drive.")->needs(mdCanIdOption);
    blink->callback(
        [this, mdCanId, od]()
        {
            auto mdco = getMdco(mdCanId, od);
            if (mdco->blink() != MDCO::Error_t::OK)
            {
                m_log.error("Failed to blink MD device with ID %d", *mdCanId);
                return;
            }
            m_log.success("Blinking MD device with ID %d", *mdCanId);
        });

    // CAN ============================================================================

    CLI::App* can =
        mdco->add_subcommand("can", "Configure CAN network parameters id, datarate and timeout.")
            ->needs(mdCanIdOption)
            ->require_option();
    CanOptions canOptions(can);
    can->callback(
        [this, mdCanId, canOptions, od]()
        {
            constexpr std::string_view canIdName = "Can ID";
            auto                       mdco      = getMdco(mdCanId, od);
            if (*canOptions.canId < 1 || *canOptions.canId > 31)
            {
                m_log.error("CAN id out of range!");
                return;
            }

            // Get id object
            auto canIdOpt = od->getEntryByName(canIdName);
            if (!canIdOpt.has_value())
            {
                m_log.error("%s not found in eds!", canIdName.data());
                return;
            }
            auto& canIdObj = canIdOpt.value().get();

            if (!canOptions.optionsMap.at("id")->empty() && !(*canOptions.canId == *mdCanId))
            {
                // set new can id
                canIdObj = (open_types::UNSIGNED32_t)(*canOptions.canId);
                if (mdco->writeSDO(canIdObj) != MDCO::Error_t::OK)
                {
                    m_log.error("Failed setting id of %d", *canOptions.canId);
                    return;
                }
            }

            if (mdco->save() != MDCO::Error_t::OK)
            {
                m_log.error("Failed to save parameters!");
                return;
            }

            m_log.success("Succesfully updated CAN parameters!");
        });

    // CONFIG ===========================================================================
    auto* config = mdco->add_subcommand("config", "Configure MD drive.")
                       ->needs(mdCanIdOption)
                       ->require_subcommand();
    // Download configuration file
    auto* downloadConfig =
        config->add_subcommand("download", "Download configuration from MD drive.");

    ConfigOptions downloadConfigOptions(downloadConfig);

    downloadConfig->callback(
        [this, od, mdCanId, downloadConfigOptions]()
        {
            auto md = getMdco(mdCanId, od);
            if (md == nullptr)
            {
                return;
            }

            std::string configFilePath = *downloadConfigOptions.configFile;
            if (configFilePath.empty())
            {
                m_log.error("Configuration file path is empty!");
                return;
            }
            // If the path is not specified, prepend the standard path
            if (std::find(configFilePath.begin(), configFilePath.end(), '/') ==
                configFilePath.end())
            {
                configFilePath = "/etc/candletool/config/motors/" + configFilePath;
            }

            MDConfigMap cfgMap;
            // for (auto& [regAddress, cfgElement] : cfgMap.m_map)
            // {
            //     cfgElement.m_value = registerRead(*md, regAddress).value_or("NOT FOUND");
            // }
            // // Write the configuration to the file
            // mINI::INIFile      configFile(configFilePath);
            // mINI::INIStructure ini;
            // for (const auto& [regAddress, cfgElement] : cfgMap.m_map)
            // {
            //     ini[cfgElement.m_tomlSection.data()][cfgElement.m_tomlKey.data()] =
            //         cfgElement.getReadable();
            // }
            // if (!configFile.generate(ini, true))
            // {
            //     m_logger.error("Could not write configuration to file: %s",
            //     configFilePath.c_str()); return;
            // }
            // m_logger.success("Configuration downloaded successfully to %s",
            // configFilePath.c_str());
        });

    // Upload configuration file
    auto* uploadConfig =
        config->add_subcommand("upload", "Upload configuration to MD drive.")->needs(mdCanIdOption);

    ConfigOptions uploadConfigOptions(uploadConfig);

    uploadConfig->callback(
        [this, od, mdCanId, uploadConfigOptions]()
        {
            auto md = getMdco(mdCanId, od);
            // if (md == nullptr)
            // {
            //     return;
            // }

            // std::string configFilePath = *uploadConfigOptions.configFile;
            // if (configFilePath.empty())
            // {
            //     m_logger.error("Configuration file path is empty!");
            //     return;
            // }
            // // If the path is not specified, prepend the standard path
            // if (std::find(configFilePath.begin(), configFilePath.end(), '/') ==
            //     configFilePath.end())
            // {
            //     configFilePath = "/etc/candletool/config/motors/" + configFilePath;
            // }

            // mINI::INIFile      configFile(configFilePath);
            // mINI::INIStructure ini;
            // if (!configFile.read(ini))
            // {
            //     m_logger.error("Could not read configuration file: %s", configFilePath.c_str());
            //     return;
            // }

            // MDConfigMap cfgMap;

            // for (auto& [address, toml] : cfgMap.m_map)
            // {
            //     auto it = ini[toml.m_tomlSection.data()][toml.m_tomlKey.data()];
            //     if (it.empty())
            //     {
            //         m_logger.warn("Key %s.%s not found in configuration file. Skipping.",
            //                       toml.m_tomlSection.data(),
            //                       toml.m_tomlKey.data());
            //         continue;
            //     }
            //     if (!toml.setFromReadable(it))
            //     {
            //         m_logger.error("Could not set value for %s.%s",
            //                        toml.m_tomlSection.data(),
            //                        toml.m_tomlKey.data());
            //         return;
            //     }
            //     // Write the value to the MD
            //     registerWrite(*md, address, toml.m_value);
            // }

            // if (md->save() != MD::Error_t::OK)
            // {
            //     m_logger.error("Could not save configuration!");
            //     return;
            // }
            // m_logger.success("Uploaded configuration to the MD!");
        });
    // CLEAR ============================================================================

    CLI::App* clear =
        mdco->add_subcommand("clear", "Clear MD drive errors and warnings.")->needs(mdCanIdOption);
    clear->callback(
        [this, mdCanId, od]()
        {
            auto mdco = getMdco(mdCanId, od);
            if (mdco->clearErrors() != MDCO::Error_t::OK)
            {
                m_log.error("Failed to clear errors!");
            }
            m_log.success("Cleared errors and warnings");
        });

    // DISCOVER ============================================================================
    CLI::App* discover = mdco->add_subcommand("discover",
                                              "Discover MD drives on the"
                                              " network.")
                             ->excludes(mdCanIdOption);

    discover->callback(
        [this, od]()
        {
            auto candle = std::unique_ptr<Candle>(
                attachCandle(*(m_candleBuilder->datarate), *(m_candleBuilder->busType), true));
            std::vector<canId_t> mdIds;
            mdIds = MDCO::discoverOpenMDs(candle.get(), od);
            m_log.info("Discovered MDCOs: ");
            for (const auto& id : mdIds)
            {
                m_log.info("- %d", id);
            }
        });

    // ENCODER CANopen ============================================================================
    CLI::App* encoder = mdco->add_subcommand("encoder", "Encoder test");

    // ENCODER display
    CLI::App* encoderDisplay = encoder->add_subcommand("display", "Display MD motor position.");
    encoderDisplay->callback(
        [this, mdCanId, od]()
        {
            auto          mdco = getMdco(mdCanId, od);
            MDCO::Error_t err  = mdco->readSDO(((*od)[0x6064]));
            if (err != MDCO::Error_t::OK)
            {
                m_log.error("Error reading encoder value");
            }
            m_log.success("The value of the encoder is: %i",
                          (i32)(open_types::INTEGER32_t)((*od)[0x6064]));
        });

    // SDO ============================================================================

    CLI::App* sdo =
        mdco->add_subcommand("sdo", "Access MD drive via OD SDO read/write.")->needs(mdCanIdOption);

    // REGISTER read
    CLI::App* sdoRead = sdo->add_subcommand("read", "Read MD register.");

    ReadOptions readOption(sdoRead);

    sdoRead->callback(
        [this, mdCanId, readOption, od]()
        {
            auto mdco = getMdco(mdCanId, od);

            if (readOption.subindex->has_value())
            {
                if (!(*od)[*readOption.index][readOption.subindex->value()]
                         .getValueMetaData()
                         .has_value())
                {
                    m_log.error("This sdo has no value to read!");
                }
                auto err = mdco->readSDO((*od)[*readOption.index][readOption.subindex->value()]);
                if (err != MDCO::Error_t::OK)
                {
                    m_log.error("Coudl not read this sdo!");
                    return;
                }
                m_log.success(
                    "%s = %s",
                    (*od)[*readOption.index][readOption.subindex->value()]
                        .getEntryMetaData()
                        .parameterName.c_str(),
                    (*od)[*readOption.index][readOption.subindex->value()].getAsString().c_str());
            }
            else
            {
                if (!(*od)[*readOption.index].getValueMetaData().has_value())
                {
                    m_log.error("This sdo has no value to read!");
                }
                auto err = mdco->readSDO((*od)[*readOption.index]);
                if (err != MDCO::Error_t::OK)
                {
                    m_log.error("Coudl not read this sdo!");
                    return;
                }
                m_log.success("%s = %s",
                              (*od)[*readOption.index].getEntryMetaData().parameterName.c_str(),
                              (*od)[*readOption.index].getAsString().c_str());
            }
        });

    // REGISTER write
    CLI::App*    sdoWrite = sdo->add_subcommand("write", "Write MD register.");
    std::string  sdoValueString;
    WriteOptions writeOption(sdoWrite);

    sdoWrite->callback(
        [this, mdCanId, writeOption, od]()
        {
            auto mdco = getMdco(mdCanId, od);

            if (writeOption.subindex->has_value())
            {
                if (!(*od)[*writeOption.index][writeOption.subindex->value()]
                         .getValueMetaData()
                         .has_value())
                {
                    m_log.error("This sdo has no value to write!");
                }
                (*od)[*writeOption.index][writeOption.subindex->value()].setFromString(
                    *writeOption.valueStr);
                auto err = mdco->writeSDO((*od)[*writeOption.index][writeOption.subindex->value()]);
                if (err != MDCO::Error_t::OK)
                {
                    m_log.error("Coudl not write this sdo!");
                    return;
                }
                m_log.success(
                    "%s = %s",
                    (*od)[*writeOption.index][writeOption.subindex->value()]
                        .getEntryMetaData()
                        .parameterName.c_str(),
                    (*od)[*writeOption.index][writeOption.subindex->value()].getAsString().c_str());
            }
            else
            {
                if (!(*od)[*writeOption.index].getValueMetaData().has_value())
                {
                    m_log.error("This sdo has no value to write!");
                }
                (*od)[*writeOption.index].setFromString(*writeOption.valueStr);
                auto err = mdco->writeSDO((*od)[*writeOption.index]);
                if (err != MDCO::Error_t::OK)
                {
                    m_log.error("Coudl not write this sdo!");
                    return;
                }
                m_log.success("%s = %s",
                              (*od)[*writeOption.index].getEntryMetaData().parameterName.c_str(),
                              (*od)[*writeOption.index].getAsString().c_str());
            }
        });

    // RESET ============================================================================
    CLI::App* reset = mdco->add_subcommand("reset", "Reset MD drive.")->needs(mdCanIdOption);
    reset->callback(
        [this, mdCanId, od]()
        {
            auto          md  = getMdco(mdCanId, od);
            MDCO::Error_t err = md->reset();
            if (err != MDCO::Error_t::OK)
            {
                m_log.error("Error resetting MD device with ID %d", *mdCanId);
                return;
            }
            m_log.success("Driver %d is restarting", (uint)*mdCanId);
        });

    // Calibration
    CLI::App* setupCalib = mdco->add_subcommand("calibration", "Calibrate main MD encoder.");
    setupCalib->callback(
        [this, mdCanId, od]()
        {
            constexpr std::string_view calibrationName = "Run Calibration";
            auto                       mdco            = getMdco(mdCanId, od);
            if (mdco->enterConfigMode() != MDCO::Error_t::OK)
            {
                m_log.error("Could not enter config mode!");
                return;
            }
            auto calibrationOpt = od->getEntryByName(calibrationName);
            if (!calibrationOpt.has_value())
            {
                m_log.error("Could not find %s command in the .eds file!", calibrationName.data());
                return;
            }

            auto& calibrationObj = calibrationOpt.value().get();
            calibrationObj       = (open_types::BOOLEAN_t)1;
            if (mdco->writeSDO(calibrationObj) != MDCO::Error_t::OK)
            {
                m_log.error("Could not write SDO calibration command!");
                return;
            }
            m_log.success("Calibration is running!");
        });

    // SETUP calibration output
    CLI::App* setupCalibOut = mdco->add_subcommand("calibration_out", "Calibrate output encoder.");
    setupCalibOut->callback(
        [this, mdCanId, od]()
        {
            auto mdco = getMdco(mdCanId, od);
            // MDCO::Error_t err  = mdco->encoderCalibration(0, 1);
            // if (err != MDCO::OK)
            // {
            //     m_log.error("Error running output encoder calibration");
            // }
            m_log.warn("To be implemented!");
        });

    // SETUP info
    CLI::App* info = mdco->add_subcommand("info", "Display info about the MD drive.");
    info->callback(
        [this, mdCanId, od]()
        {
            auto mdco = getMdco(mdCanId, od);

            for (auto& object : *od)
            {
                u32 idx = object.first;
                // error fields skipped
                bool skip = false;
                switch (idx)
                {
                    case 0x1003:
                    case 0x1801:
                        skip = true;
                        break;
                    default:
                        skip = false;
                        break;
                }
                if (skip)
                    continue;
                if (object.second.getContainerMetaData().has_value())
                {
                    std::stringstream ss;
                    ss << "[0x" << std::hex << idx << "] "
                       << object.second.getEntryMetaData().parameterName;
                    m_log.info("%s", ss.str().c_str());
                    for (auto& subobject : object.second)
                    {
                        if (mdco->readSDO(*subobject.second) != MDCO::Error_t::OK)
                        {
                            m_log.error("Coudl not read object %s",
                                        subobject.second->getEntryMetaData().parameterName.c_str());
                            continue;
                        }
                        std::stringstream ss;
                        ss << "[0x" << std::hex << idx << "]" << "[0x"
                           << (unsigned int)subobject.second->getEntryMetaData()
                                  .address.second.value()
                           << "]" << subobject.second->getEntryMetaData().parameterName << " = "
                           << subobject.second->getAsString();
                        m_log.info("%s", ss.str().c_str());
                    }
                    continue;
                }
                if (mdco->readSDO(object.second) != MDCO::Error_t::OK)
                {
                    m_log.error("Coudl not read object %s",
                                object.second.getEntryMetaData().parameterName.c_str());
                    continue;
                }
                std::stringstream ss;
                ss << "[0x" << std::hex << idx << "] "
                   << object.second.getEntryMetaData().parameterName << " = "
                   << object.second.getAsString();
                m_log.info("%s", ss.str().c_str());
            }
        });

    // Save
    CLI::App* save =
        mdco->add_subcommand("save", "Save registers to persistant memory and reset the driver.");
    save->callback(
        [this, mdCanId, od]()
        {
            auto mdco = getMdco(mdCanId, od);
            if (mdco == nullptr)
            {
                m_log.error("Coudl not connect to MD via CANopen!");
                return;
            }
            if (mdco->save() != MDCO::Error_t::OK)
            {
                m_log.error("Saving failed!");
                return;
            }
            m_log.success("Saving registers succesful!");
        });

    // TEST ============================================================================
    CLI::App* test = mdco->add_subcommand("test", "Test the MD drive movement.")
                         ->needs(mdCanIdOption)
                         ->require_subcommand();

    // // TEST move
    CLI::App* testMove =
        test->add_subcommand("move", "Validate if motor can move.")->require_subcommand();

    // // TEST move absolute
    CLI::App* testMoveAbs = testMove->add_subcommand(
        "absolute", "Move motor to absolute position using position profile mode.");

    MoveOptions moveOptionsAbs(testMoveAbs);

    testMoveAbs->callback(
        [this, mdCanId, od, moveOptionsAbs]()
        {
            auto mdco = getMdco(mdCanId, od);
            if (mdco->enable() != MDCO::Error_t::OK)
            {
                m_log.error("Failed move");
                return;
            }
            if (mdco->setOperationMode(mab::ModesOfOperation::ProfilePosition) != MDCO::Error_t::OK)
            {
                m_log.error("Failed move");
                return;
            }
            if (mdco->setTargetPosition(*moveOptionsAbs.position) != MDCO::Error_t::OK)
            {
                m_log.error("Failed move");
                return;
            }
            while (!mdco->targetReached().first)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                auto position = mdco->getPosition().first;
                std::cout << "Pos: " << position << '\n';
                mdco->setTargetPosition(
                    *moveOptionsAbs.position);  // get driver unstuck from quickstop
            }

            m_log.success("Target Reached!");

            if (mdco->disable() != MDCO::Error_t::OK)
            {
                m_log.error("Failed disable");
                return;
            }
        });

    // Relative
    CLI::App* testMoveRel = testMove->add_subcommand(
        "relative", "Move motor to relative position using impedance mode.");

    MoveOptions moveOptionsRel(testMoveRel);

    testMoveRel->callback(
        [this, mdCanId, od, moveOptionsRel]()
        {
            auto mdco = getMdco(mdCanId, od);
            if (mdco->zero() != MDCO::Error_t::OK)
            {
                m_log.error("Failed move");
                return;
            }
            if (mdco->disable() != MDCO::Error_t::OK)
            {
                m_log.error("Failed move");
                return;
            }
            if (mdco->setOperationMode(mab::ModesOfOperation::Impedance) != MDCO::Error_t::OK)
            {
                m_log.error("Failed move");
                return;
            }
            if (mdco->enable() != MDCO::Error_t::OK)
            {
                m_log.error("Failed move");
                return;
            }
            if (mdco->setOperationMode(mab::ModesOfOperation::Impedance) != MDCO::Error_t::OK)
            {
                m_log.error("Failed move");
                return;
            }
            // Arbitrary clamping, better to change that in the future
            *moveOptionsRel.position = std::clamp(*moveOptionsRel.position, -32'000, 32'000);

            auto             position       = mdco->getPosition().first;
            auto             targetPosition = *moveOptionsRel.position;
            constexpr size_t steps          = 100;

            std::array<i32, steps> trajectory;

            size_t i = 0;
            for (auto& elem : trajectory)
            {
                elem = position + (targetPosition - position) * (double)(i++) / (steps);
            }

            for (const auto& trajectoryPoint : trajectory)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                position = mdco->getPosition().first;
                m_log << "Pos: " << position << '\n';
                m_log << "Target: " << trajectoryPoint << '\n';
                mdco->setTargetPosition(trajectoryPoint);
            }

            m_log.success("Target Reached!");

            if (mdco->disable() != MDCO::Error_t::OK)
            {
                m_log.error("Failed disable");
                return;
            }
        });
}
