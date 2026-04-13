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
#include "candle/MD/MDCO.hpp"
#include "candle/communication_device/candle.hpp"
#include "candle/communication_device/candle_types.hpp"
#include "candle/objectDictionary/edsEntry.hpp"
#include "candle/objectDictionary/edsParser.hpp"
#include "mab_types.hpp"
#include "md_cfg_map.hpp"
#include "mini/ini.h"
#include "mdco_config_adapter.hpp"

using namespace mab;

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
    m_log.m_tag     = "MDCO_CLI";
    m_log.m_layer   = Logger::ProgramLayer_E::TOP;
    m_candleBuilder = m_ctx.candleBranchVec.at(0).candleBuilder;

    std::filesystem::path configFilePath =
        m_ctx.packageEtcPath->string() + "/config/candletool.ini";

    auto loadEDS =
        [this,
         configFilePath]() -> std::pair<std::shared_ptr<EDSObjectDictionary>, EDSParser::Error_t>
    {
        if (!std::filesystem::exists(configFilePath))
        {
            m_log.error(
                "could not locate candletool.ini configuration file in %s. Is the candletool "
                "installed "
                "properly?",
                configFilePath.c_str());
            exit(1);
        }

        mINI::INIFile      configFile(configFilePath);
        mINI::INIStructure configStruct;

        configFile.read(configStruct);

        std::filesystem::path edsPath = configStruct["eds"]["path"];
        if (edsPath.empty() || !std::filesystem::exists(edsPath))
        {
            m_log.error(
                "could not locate .eds file. Please check the %s file for eds section and fill it "
                "properly. Currently read path is: %s",
                configFilePath.c_str(),
                edsPath.c_str());
            exit(1);
        }

        auto odPair = EDSParser::load(edsPath);
        if (odPair.second != EDSParser::Error_t::OK)
        {
            m_log.warn("EDS parsing failed!");
        }
        return odPair;
    };

    CLI::App* mdco = m_rootCli.add_subcommand("mdco", "Send CANopen command instead of CAN FD.");
    const std::shared_ptr<canId_t> mdCanId = std::make_shared<canId_t>(10);
    auto*                          mdCanIdOption =
        mdco->add_option("-i,--id", *mdCanId, "CAN ID of the MD to interact with.");

    // BLINK ============================================================================

    CLI::App* blink =
        mdco->add_subcommand("blink", "Blink LEDs on MD drive.")->needs(mdCanIdOption);
    blink->callback(
        [this, mdCanId, loadEDS]()
        {
            auto od   = loadEDS().first;
            auto mdco = getMdco(mdCanId, od);
            if (mdco == nullptr)
                m_log.error("Failed to conect to mdco!");
            if (mdco->blink() != MDCO::Error_t::OK)
            {
                m_log.error("Failed to blink MD device with ID %d", *mdCanId);
                return;
            }
            m_log.success("Blinking MD device with ID %d", *mdCanId);
        });

    // CAN ============================================================================

    CLI::App* can = mdco->add_subcommand("can", "Configure CAN id of the driver.")
                        ->needs(mdCanIdOption)
                        ->require_option();
    CanOptions canOptions(can);
    can->callback(
        [this, mdCanId, canOptions, loadEDS]()
        {
            constexpr std::string_view canIdName = "Can ID";
            auto                       od        = loadEDS().first;
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
                canIdObj = (canopen_types::UNSIGNED32_t)(*canOptions.canId);
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
    auto* config = mdco->add_subcommand("config", "Manage configuration of the driver.")
                       ->needs(mdCanIdOption)
                       ->require_subcommand();
    // Download configuration file
    auto* downloadConfig =
        config->add_subcommand("download", "Download configuration from MD drive.");

    ConfigOptions downloadConfigOptions(downloadConfig);

    downloadConfig->callback(
        [this, loadEDS, mdCanId, downloadConfigOptions]()
        {
            auto od = loadEDS().first;
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

            MDConfigMap       cfgMap;
            MDCOConfigAdapter odCfgAdapter;
            for (auto& [regAddr, objName, subidxOpt] : odCfgAdapter.manufacturerRegMaping)
            {
                auto objOpt = od->getEntryByName(objName);
                if (!objOpt.has_value())
                {
                    m_log.warn("Obj %s does not exist in the eds!", objName.data());
                    continue;
                }
                auto& obj = subidxOpt.has_value() ? objOpt.value().get()[subidxOpt.value()]
                                                  : objOpt.value().get();
                if (md->readSDO(obj) != MDCO::Error_t::OK)
                {
                    m_log.error("Obj %s could not be read from md!", objName.data());
                    continue;
                }
            }
            for (auto& [regAddr, objAddress, subidxOpt] : odCfgAdapter.standardRegMaping)
            {
                auto& obj = subidxOpt.has_value() ? (*od)[objAddress][subidxOpt.value()]
                                                  : (*od)[objAddress];
                if (md->readSDO(obj) != MDCO::Error_t::OK)
                {
                    m_log.error("Obj %d could not be read from md!", objAddress);
                    continue;
                }
            }

            odCfgAdapter.configFromOd(od, cfgMap);
            // // Write the configuration to the file
            mINI::INIFile      configFile(configFilePath);
            mINI::INIStructure ini;
            for (const auto& [regAddress, cfgElement] : cfgMap.m_map)
            {
                if (!cfgElement.getReadable().empty())
                    ini[cfgElement.m_tomlSection.data()][cfgElement.m_tomlKey.data()] =
                        cfgElement.getReadable();
            }
            if (!configFile.generate(ini, true))
            {
                m_log.error("Could not write configuration to file: %s", configFilePath.c_str());
                return;
            }
            m_log.success("Configuration downloaded successfully to %s", configFilePath.c_str());
        });

    // Upload configuration file
    auto* uploadConfig =
        config->add_subcommand("upload", "Upload configuration to MD drive.")->needs(mdCanIdOption);

    ConfigOptions uploadConfigOptions(uploadConfig);

    uploadConfig->callback(
        [this, loadEDS, mdCanId, uploadConfigOptions]()
        {
            auto od = loadEDS().first;
            auto md = getMdco(mdCanId, od);
            if (md == nullptr)
            {
                return;
            }

            std::string configFilePath = *uploadConfigOptions.configFile;
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

            mINI::INIFile      configFile(configFilePath);
            mINI::INIStructure ini;
            if (!configFile.read(ini))
            {
                m_log.error("Could not read configuration file: %s", configFilePath.c_str());
                return;
            }

            MDConfigMap       cfgMap;
            MDCOConfigAdapter odCfgAdapter;

            for (auto& [address, toml] : cfgMap.m_map)
            {
                auto it = ini[toml.m_tomlSection.data()][toml.m_tomlKey.data()];
                if (it.empty())
                {
                    m_log.warn("Key %s.%s not found in configuration file. Skipping.",
                               toml.m_tomlSection.data(),
                               toml.m_tomlKey.data());
                    continue;
                }
                if (!toml.setFromReadable(it))
                {
                    m_log.error("Could not set value for %s.%s",
                                toml.m_tomlSection.data(),
                                toml.m_tomlKey.data());
                    return;
                }
            }

            auto entries = odCfgAdapter.configToOd(cfgMap, od);

            for (auto& entry : entries)
            {
                if (md->writeSDO(entry.get()) != MDCO::Error_t::OK)
                {
                    m_log.error("Error writing %s",
                                entry.get().getEntryMetaData().parameterName.c_str());
                }
            }

            if (md->save() != MDCO::Error_t::OK)
            {
                m_log.error("Could not save configuration!");
                return;
            }
            m_log.success("Uploaded configuration to the MD!");
        });
    // CLEAR ============================================================================

    CLI::App* clear =
        mdco->add_subcommand("clear", "Clear MD drive errors and warnings.")->needs(mdCanIdOption);
    clear->callback(
        [this, mdCanId, loadEDS]()
        {
            auto od   = loadEDS().first;
            auto mdco = getMdco(mdCanId, od);
            if (mdco == nullptr)
                m_log.error("Failed to conect to mdco!");
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
        [this, loadEDS]()
        {
            auto candle = std::unique_ptr<Candle>(
                attachCandle(*(m_candleBuilder->datarate), *(m_candleBuilder->busType), true));
            std::vector<canId_t> mdIds;
            auto                 od = loadEDS().first;
            mdIds                   = MDCO::discoverOpenMDs(candle.get(), od);
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
        [this, mdCanId, loadEDS]()
        {
            auto          od   = loadEDS().first;
            auto          mdco = getMdco(mdCanId, od);
            MDCO::Error_t err  = mdco->readSDO(((*od)[0x6064]));
            if (err != MDCO::Error_t::OK)
            {
                m_log.error("Error reading encoder value");
            }
            m_log.success("The value of the encoder is: %i",
                          (i32)(canopen_types::INTEGER32_t)((*od)[0x6064]));
        });

    // SDO ============================================================================

    CLI::App* sdo =
        mdco->add_subcommand("sdo", "Access MD drive via OD SDO read/write.")->needs(mdCanIdOption);

    // REGISTER read
    CLI::App* sdoRead = sdo->add_subcommand("read", "Read MD register.");

    ReadOptions readOption(sdoRead);

    sdoRead->callback(
        [this, mdCanId, readOption, loadEDS]()
        {
            auto od   = loadEDS().first;
            auto mdco = getMdco(mdCanId, od);
            if (mdco == nullptr)
                m_log.error("Failed to conect to mdco!");

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
                    m_log.error("could not read this sdo!");
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
                    m_log.error("could not read this sdo!");
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
        [this, mdCanId, writeOption, loadEDS]()
        {
            auto od   = loadEDS().first;
            auto mdco = getMdco(mdCanId, od);
            if (mdco == nullptr)
                m_log.error("Failed to conect to mdco!");

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
                    m_log.error("could not write this sdo!");
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
                    m_log.error("could not write this sdo!");
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
        [this, mdCanId, loadEDS]()
        {
            auto          od  = loadEDS().first;
            auto          md  = getMdco(mdCanId, od);
            MDCO::Error_t err = md->reset();
            if (err != MDCO::Error_t::OK)
            {
                m_log.error("Error resetting MD device with ID %d", *mdCanId);
                return;
            }
            m_log.success("Driver %d is restarting", (unsigned int)*mdCanId);
        });

    // Calibration
    CLI::App* setupCalib = mdco->add_subcommand("calibration", "Calibrate main MD encoder.");

    CalibrationOptions calibrationOptions(setupCalib);
    setupCalib->callback(
        [this, mdCanId, loadEDS, calibrationOptions]()
        {
            std::string calibrationName =
                *calibrationOptions.calibrationOfEncoder == std::string_view("main")
                    ? "Run Calibration"
                    : "Run Output Encoder Calibration";
            m_log.debug("Running cal with: %s", calibrationName.c_str());
            auto od   = loadEDS().first;
            auto mdco = getMdco(mdCanId, od);
            if (mdco == nullptr)
                m_log.error("Failed to conect to mdco!");
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
            calibrationObj       = (canopen_types::BOOLEAN_t)1;
            if (mdco->writeSDO(calibrationObj) != MDCO::Error_t::OK)
            {
                m_log.error("Could not write SDO calibration command!");
                return;
            }
            m_log.info("Calibration is running...");
        });

    // SETUP info
    CLI::App* info = mdco->add_subcommand("info", "Display info about the MD drive.");
    info->callback(
        [this, mdCanId, loadEDS]()
        {
            auto od   = loadEDS().first;
            auto mdco = getMdco(mdCanId, od);
            if (mdco == nullptr)
                m_log.error("Failed to conect to mdco!");

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
                            m_log.error("could not read object %s",
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
                    m_log.error("could not read object %s",
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
        [this, mdCanId, loadEDS]()
        {
            auto od   = loadEDS().first;
            auto mdco = getMdco(mdCanId, od);
            if (mdco == nullptr)
            {
                m_log.error("could not connect to MD via CANopen!");
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
        [this, mdCanId, loadEDS, moveOptionsAbs]()
        {
            auto od   = loadEDS().first;
            auto mdco = getMdco(mdCanId, od);
            if (mdco == nullptr)
                m_log.error("Failed to conect to mdco!");
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
        [this, mdCanId, loadEDS, moveOptionsRel]()
        {
            auto od   = loadEDS().first;
            auto mdco = getMdco(mdCanId, od);
            if (mdco == nullptr)
                m_log.error("Failed to conect to mdco!");
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
