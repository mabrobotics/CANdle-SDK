#include "mdco_cli.hpp"
#include <cstddef>
#include <exception>
#include <filesystem>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include "CLI/CLI.hpp"
#include "MDCO.hpp"
#include "candle.hpp"
#include "candle_types.hpp"
#include "edsEntry.hpp"
#include "edsParser.hpp"
#include "mini/ini.h"

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
            auto mdco = getMdco(mdCanId, od);
            // long newbaud;
            // if (*canOptions.datarate == "1M")
            //     newbaud = 1000000;
            // else if (*canOptions.datarate == "500K")
            //     newbaud = 500000;
            // else
            // {
            //     m_log.error("Invalid baudrate for CANopen, only 1M and 500K is supported");
            //     return;
            // }
            // MDCO::Error_t err =
            //     mdco->newCanOpenConfig(*canOptions.canId, newbaud, *canOptions.timeoutMs);
            // if (err != MDCO::OK)
            // {
            //     m_log.error("Error setting CANopen config");
            //     return;
            // }
            // if (*canOptions.save)
            // {
            //     err = mdco->openSave();
            //     if (err != MDCO::OK)
            //     {
            //         m_log.error("Error saving CANopen config");
            //         return;
            //     }
            // }
            m_log.warn("To be implemented!");
        });

    // CLEAR ============================================================================

    CLI::App* clear =
        mdco->add_subcommand("clear", "Clear MD drive errors and warnings.")->needs(mdCanIdOption);
    ClearOptions clearOptions(clear);
    clear->callback(
        [this, mdCanId, clearOptions, od]()
        {
            // auto mdco = getMdco(mdCanId, od);
            // m_log.info("sending in canOpen clear error \n");
            // MDCO::Error_t err = MDCO::OK;
            // if (*clearOptions.clearType == "error")
            //     err = mdco->clearOpenErrors(1);
            // else if (*clearOptions.clearType == "warning")
            //     err = mdco->clearOpenErrors(2);
            // else if (*clearOptions.clearType == "all")
            //     err = mdco->clearOpenErrors(3);
            // else
            // {
            //     m_log.error("Unknown command");
            //     return;
            // }
            // if (err != MDCO::OK)
            // {
            //     m_log.error("Error clearing errors (%s)", (*clearOptions.clearType).c_str());
            //     return;
            // }
            m_log.warn("To be implemented!");
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

    // SETUP upload
    // auto* setupupload = setup->add_subcommand("upload", "Upload actuator config from .cfg
    // file."); setupupload
    //     ->add_option("--file_path",
    //                  cmdCANopen.cfgPath,
    //                  "Filename of motor config. Default config files are "
    //                  "in:`/etc/candletool/config/motors/`.")
    //     ->required();
    // setupupload->callback(
    //     [this, mdCanId]()
    //     {
    //         std::string finalConfigPath = cmdCANopen.cfgPath;
    //         if (!cmdCANopen.force)
    //             finalConfigPath = validateAndGetFinalConfigPath(cmdCANopen.cfgPath);
    //         else
    //         {
    //             m_log.warn("Omitting config validation on user request!");
    //             if (!fileExists(finalConfigPath))
    //             {
    //                 finalConfigPath = getMotorsConfigPath() / cmdCANopen.cfgPath;
    //                 if (!fileExists(finalConfigPath))
    //                 {
    //                     m_log.error("Neither \"%s\", nor \"%s\", exists!.",
    //                                 cmdCANopen.cfgPath.c_str(),
    //                                 finalConfigPath.c_str());
    //                     exit(1);
    //                 }
    //             }
    //         }
    //         m_log.info("Uploading config from \"%s\"", finalConfigPath.c_str());
    //         mINI::INIFile      motorCfg(finalConfigPath);
    //         mINI::INIStructure cfg;
    //         motorCfg.read(cfg);
    //         auto          mdco = getMdco(mdCanId);
    //         MDCOConfigMap configMap;

    //         for (const auto& entry : configMap.m_map)
    //         {
    //             const auto& addr    = entry.first;
    //             const auto& element = entry.second;

    //             if (element.Section == "motor" && element.Key == "KV")
    //                 continue;

    //             if (!cfg.has(element.Section))
    //                 continue;

    //             auto& sectionMap = cfg[element.Section];
    //             if (!sectionMap.has(element.Key))
    //                 continue;

    //             std::string   value = sectionMap[element.Key];
    //             MDCO::Error_t err   = MDCO::OK;

    //             switch (element.Type)
    //             {
    //                 case MDCOValueType::STRING:
    //                     err = mdco->writeLongOpenRegisters(addr.first, addr.second, value, true);
    //                     break;

    //                 case MDCOValueType::FLOAT:
    //                 {
    //                     float    f = std::stof(value);
    //                     uint32_t as_long;
    //                     std::memcpy(&as_long, &f, sizeof(float));
    //                     err = mdco->writeOpenRegisters(addr.first, addr.second, as_long, 4);
    //                     break;
    //                 }

    //                 case MDCOValueType::INT:
    //                 {
    //                     int i = std::stoi(value);
    //                     err   = mdco->writeOpenRegisters(addr.first, addr.second, i);
    //                     break;
    //                 }
    //             }

    //             if (err != MDCO::OK)
    //             {
    //                 m_log.error("Error setting  skipping section [%s] element %s",
    //                             element.Section.c_str(),
    //                             element.Key.c_str());
    //             }
    //         }

    //         m_log.success("Don't forget to save the config before shutting down the MD!");
    //     });

    // // SETUP download
    // setupdownload =
    //     setup->add_subcommand("download", "Download actuator config from MD to .cfg file.");
    // setupdownload->add_option("--path", cmdCANopen.value, "File to save config to.")->required();
    // setupdownload->callback(
    //     [this, mdCanId]()
    //     {
    //         auto          mdco = getMdco(mdCanId);
    //         MDCOConfigMap configMap;
    //         std::ofstream cfg(cmdCANopen.value);

    //         if (!cfg.is_open())
    //         {
    //             m_log.error("Impossible to open %s in writing mode", cmdCANopen.value.c_str());
    //             return;
    //         }
    //         cfg << std::fixed << std::setprecision(5);
    //         std::string currentSection;
    //         for (const auto& [address, element] : configMap.m_map)
    //         {
    //             if (currentSection != element.Section)
    //             {
    //                 currentSection = element.Section;
    //                 cfg << "\n[" << currentSection << "]\n";
    //             }
    //             if (element.Section == "motor" && element.Key == "KV")
    //                 continue;
    //             if (element.Key == "name")
    //             {
    //                 std::vector<u8> name;
    //                 auto            err =
    //                     mdco->readLongOpenRegisters(address.first, address.second, name, true);
    //                 if (err != 0)
    //                 {
    //                     name.clear();
    //                 }
    //                 cfg << element.Key << " = " << std::string(name.begin(), name.end()) << "\n";
    //             }
    //             else
    //             {
    //                 long raw_data = mdco->getValueFromOpenRegister(address.first,
    //                 address.second); if (element.Key == "kp" || element.Key == "ki" ||
    //                 element.Key == "kd" ||
    //                     element.Key == "windup" ||
    //                     element.Key.find("constant") != std::string::npos ||
    //                     element.Key.find("ratio") != std::string::npos)
    //                 {
    //                     float f;
    //                     std::memcpy(&f, &raw_data, sizeof(float));
    //                     cfg << element.Key << " = " << f << "\n";
    //                 }
    //                 else
    //                 {
    //                     cfg << element.Key << " = " << raw_data << "\n";
    //                 }
    //             }
    //         }
    //         cfg.close();
    //         m_log.success("File %s generate with success.", cmdCANopen.value.c_str());
    //     });

    // TEST ============================================================================
    // CLI::App* test = mdco->add_subcommand("test", "Test the MD drive movement.")
    //                      ->needs(mdCanIdOption)
    //                      ->require_subcommand();

    // // TEST move
    // CLI::App* testMove =
    //     test->add_subcommand("move", "Validate if motor can move.")->require_subcommand();

    // // TEST move absolute
    // CLI::App* testMoveAbs =
    //     testMove->add_subcommand("absolute", "Move motor to absolute position.");
    // testMoveAbs
    //     ->add_option("--position", cmdCANopen.desiredPos, "Absolute position to reach [rad].")
    //     ->required();

    // MovementLimitsOPtions moveAbsParam(testMoveAbs);

    // testMoveAbs->callback(
    //     [this, mdCanId, moveAbsParam]()
    //     {
    //         auto          mdco = getMdco(mdCanId);
    //         MDCO::Error_t err;
    //         err = mdco->setProfileParameters(*moveAbsParam.param);
    //         if (err != MDCO::OK)
    //         {
    //             m_log.error("Error setting profile parameters");
    //             return;
    //         }
    //         err = mdco->enableDriver(ProfilePosition);
    //         if (err != MDCO::OK)
    //         {
    //             m_log.error("Error enabling driver");
    //             return;
    //         }
    //         // mdco->movePosition(cmdCANopen.desiredPos);
    //         err = mdco->disableDriver();
    //         if (err != MDCO::OK)
    //         {
    //             m_log.error("Error disabling driver");
    //             return;
    //         }
    //     });

    // // TEST move relative
    // testMoveRel = testMove->add_subcommand("relative", "Move motor to relative position.");
    // testMoveRel
    //     ->add_option("--position",
    //                  cmdCANopen.desiredPos,
    //                  "Relative position to reach.<0x0, "
    //                  "0xFFFFFFFF>[inc] ")
    //     ->required();

    // MovementLimitsOPtions moveRelParam(testMoveRel);

    // testMoveRel->callback(
    //     [this, mdCanId, moveRelParam]()
    //     {
    //         auto          mdco = getMdco(mdCanId);
    //         MDCO::Error_t err;
    //         err = mdco->setProfileParameters(*moveRelParam.param);
    //         if (err != MDCO::OK)
    //         {
    //             m_log.error("Error setting profile parameters");
    //             return;
    //         }
    //         err = mdco->enableDriver(CyclicSyncPosition);
    //         if (err != MDCO::OK)
    //         {
    //             m_log.error("Error enabling driver");
    //             return;
    //         }
    //         // mdco->movePosition(cmdCANopen.desiredPos);
    //         err = mdco->disableDriver();
    //         if (err != MDCO::OK)
    //         {
    //             m_log.error("Error disabling driver");
    //             return;
    //         }
    //     });

    // // TEST move impedance
    // testImpedance = testMove->add_subcommand(
    //     "impedance",
    //     "Put the motor into Impedance PD mode "
    //     "cf:https://mabrobotics.github.io/MD80-x-CANdle-Documentation/"
    //     "md_x_candle_ecosystem_overview/Motion%20modes.html#impedance-pd.");
    // testImpedance
    //     ->add_option(
    //         "--speed", cmdCANopen.desiredSpeed, "Sets the target velocity for all motion modes.")
    //     ->required();
    // testImpedance
    //     ->add_option("--position",
    //                  cmdCANopen.desiredPos,
    //                  "Relative position to reach. <0x0, "
    //                  "0xFFFFFFFF>[inc].")
    //     ->required();
    // testImpedance->add_option("--kp", cmdCANopen.param.kp, "Position gain.")->required();
    // testImpedance->add_option("--kd", cmdCANopen.param.kd, "Velocity gain.")->required();
    // testImpedance->add_option("--torque_ff", cmdCANopen.param.torqueff, "Torque
    // FF.")->required();

    // MovementLimitsOPtions moveImpedanceParam(testImpedance);

    // testImpedance->callback(
    //     [this, mdCanId, moveImpedanceParam]()
    //     {
    //         auto          mdco = getMdco(mdCanId);
    //         MDCO::Error_t err;
    //         err = mdco->setProfileParameters(*moveImpedanceParam.param);
    //         if (err != MDCO::OK)
    //         {
    //             m_log.error("Error setting profile parameters");
    //             return;
    //         }
    //         err = mdco->openSave();
    //         if (err != MDCO::OK)
    //         {
    //             m_log.error("Error enabling driver");
    //             return;
    //         }
    //         err = mdco->enableDriver(Impedance);
    //         if (err != MDCO::OK)
    //         {
    //             m_log.error("Error enabling driver");
    //             return;
    //         }
    //         err =
    //             MDCO::Error_t::UNKNOWN_OBJECT;  // mdco->moveImpedance(
    //                                             //  cmdCANopen.desiredSpeed,
    //                                             cmdCANopen.desiredPos,
    //                                             //  cmdCANopen.param, 5000);
    //         if (err != MDCO::OK)
    //         {
    //             m_log.error("Error moving impedance");
    //             return;
    //         }
    //         err = mdco->disableDriver();
    //         if (err != MDCO::OK)
    //         {
    //             m_log.error("Error disabling driver");
    //             return;
    //         }
    //     });

    // // TIME STAMP ============================================================================
    // timeStamp =
    //     mdco->add_subcommand("time", "Send a time stamp message using the computer's clock.")
    //         ->needs(mdCanIdOption);

    // timeStamp->callback(
    //     [this, mdCanId]()
    //     {
    //         auto mdco = getMdco(mdCanId);

    //         auto now = std::chrono::system_clock::now();

    //         std::tm epoch_tm  = {};
    //         epoch_tm.tm_year  = 84;
    //         epoch_tm.tm_mon   = 0;
    //         epoch_tm.tm_mday  = 1;
    //         epoch_tm.tm_hour  = 0;
    //         epoch_tm.tm_min   = 0;
    //         epoch_tm.tm_sec   = 0;
    //         epoch_tm.tm_isdst = -1;

    //         auto epoch_time_t = std::mktime(&epoch_tm);
    //         auto epoch_tp     = std::chrono::system_clock::from_time_t(epoch_time_t);
    //         auto days_since = std::chrono::duration_cast<std::chrono::days>(now -
    //         epoch_tp).count(); std::time_t now_time_t  =
    //         std::chrono::system_clock::to_time_t(now); std::tm     local_tm    =
    //         *std::localtime(&now_time_t); std::tm     midnight_tm = local_tm;

    //         midnight_tm.tm_hour = 0;
    //         midnight_tm.tm_min  = 0;
    //         midnight_tm.tm_sec  = 0;

    //         auto midnight_time_t       = std::mktime(&midnight_tm);
    //         auto midnight_tp           = std::chrono::system_clock::from_time_t(midnight_time_t);
    //         long millis_since_midnight = static_cast<long>(
    //             std::chrono::duration_cast<std::chrono::milliseconds>(now -
    //             midnight_tp).count());

    //         m_log.info("The actual time according to your computer is: %s",
    //                    std::asctime(&local_tm));
    //         m_log.info("Number of days since 1st January 1984: %ld", days_since);
    //         m_log.info("Number of millis since midnight: %ld", millis_since_midnight);

    //         long TimeMessageId = mdco->getValueFromOpenRegister(0x1012, 0x00);

    //         std::vector<u8> frame = {
    //             ((u8)millis_since_midnight),
    //             ((u8)(millis_since_midnight >> 8)),
    //             ((u8)(millis_since_midnight >> 16)),
    //             ((u8)(millis_since_midnight >> 24)),
    //             ((u8)days_since),
    //             ((u8)(days_since >> 8)),
    //         };

    //         MDCO::Error_t err = mdco->writeOpenPDORegisters(TimeMessageId, frame);
    //         if (err != MDCO::OK)
    //         {
    //             m_log.error("Error sending time message");
    //             return;
    //         }
    //         else
    //         {
    //             m_log.success("message send");
    //         }
    //     });
}
