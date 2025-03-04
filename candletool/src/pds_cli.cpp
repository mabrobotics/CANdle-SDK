#include "pds_cli.hpp"
#include "configHelpers.hpp"

PdsCli::PdsCli(CLI::App& rootCli, mab::Candle& candle) : m_rootCli(rootCli), m_candle(candle)
{
    m_log.m_tag   = "CANDLETOOL";
    m_log.m_layer = Logger::ProgramLayer_E::TOP;

    m_pdsCmd = m_rootCli.add_subcommand("pds", "Tweak the PDS device");

    m_pdsCmd->add_option("<CAN_ID>", m_canId, "MAB FD-CAN protocol :: Target device ID")
        ->required();

    m_infoCmd        = m_pdsCmd->add_subcommand("info", "Display debug info about PDS device");
    m_configSetupCmd = m_pdsCmd->add_subcommand("setup", "Configure PDS device with the .cfg file");

    m_configSetupCmd->add_option("<config_file>", m_cfgFilePath, "PDS configuration .cfg file.")
        ->required();

    m_configReadCmd =
        m_pdsCmd->add_subcommand("read_cfg", "Read device configuration and save to file");

    m_configReadCmd->add_option("<config_file>", m_cfgFilePath, "PDS configuration .cfg file.")
        ->required();

    m_configSaveCmd =
        m_pdsCmd->add_subcommand("save", "Store current configuration in device memory");

    m_powerStageCmd = m_pdsCmd->add_subcommand("ps", "Manage the Power Stage submodule");
    m_powerStageCmd
        ->add_option("<socket_index>", m_submoduleSocketNumber, "Submodule socket number")
        ->required();

    m_psSetOvcLevelCmd =
        m_powerStageCmd->add_subcommand("set_ovc_level", "Set the Overcurrent Detection level");

    m_psGetOvcLevelCmd =
        m_powerStageCmd->add_subcommand("get_ovc_level", "Get the Overcurrent Detection level");

    m_psSetOvcDelayCmd =
        m_powerStageCmd->add_subcommand("set_ovc_delay", "Set the Overcurrent Detection delay");

    m_psGetOvcDelayCmd =
        m_powerStageCmd->add_subcommand("get_ovc_delay", "Get the Overcurrent Detection delay");

    m_psSetTempLimitCmd =
        m_powerStageCmd->add_subcommand("set_temp_limit", "Set the Temperature Limit");

    m_psGetTempLimitCmd =
        m_powerStageCmd->add_subcommand("get_temp_limit", "Get the Temperature Limit");

    m_psSetBrCmd = m_powerStageCmd->add_subcommand("set_br", "Set the Brake Resistor Socket index");

    m_psGetBrCmd = m_powerStageCmd->add_subcommand("get_br", "Get the Brake Resistor Socket index");

    m_psSetBrTriggerCmd =
        m_powerStageCmd->add_subcommand("set_br_trigger", "Set the Brake Resistor Trigger Voltage");

    m_psGetBrTriggerCmd =
        m_powerStageCmd->add_subcommand("get_br_trigger", "Get the Brake Resistor Trigger Voltage");

    m_brakeResistorCmd = m_pdsCmd->add_subcommand("br", "Manage the Brake Resistor submodule");

    m_brakeResistorCmd
        ->add_option("<socket_index>", m_submoduleSocketNumber, "Submodule socket number")
        ->required();

    m_isolatedConverterCmd =
        m_pdsCmd->add_subcommand("ic", "Manage the Isolated Converter submodule");

    m_isolatedConverterCmd
        ->add_option("<socket_index>", m_submoduleSocketNumber, "Submodule socket number")
        ->required();
}

void PdsCli::parse(void)
{
    if (m_pdsCmd->parsed())
    {
        m_pds.init();

        if (m_infoCmd->parsed())
        {
            pdsSetupInfo(m_canId);
        }

        if (m_configSetupCmd->parsed())
        {
            pdsSetupConfig(m_canId, m_cfgFilePath);
        }

        if (m_configReadCmd->parsed())
        {
            pdsReadConfig(m_canId, m_cfgFilePath);
        }

        if (m_configSaveCmd->parsed())
        {
            pdsStoreConfig(m_canId);
        }

        if (m_powerStageCmd->parsed())
        {
            if (m_psSetOvcLevelCmd->parsed())
            {
            }
        }
    }
}

void PdsCli::pdsSetupInfo(u16 id)
{
    mab::Pds pds(id, m_candle);

    pds.init();

    mab::Pds::modulesSet_S pdsModules = pds.getModules();

    u32                       shutdownTime  = 0;
    u32                       batteryLvl1   = 0;
    u32                       batteryLvl2   = 0;
    u32                       pdsBusVoltage = 0;
    mab::controlBoardStatus_S pdsStatus     = {0};

    pds.getStatus(pdsStatus);
    pds.getBusVoltage(pdsBusVoltage);
    pds.getShutdownTime(shutdownTime);
    pds.getBatteryVoltageLevels(batteryLvl1, batteryLvl2);

    m_log.info("Submodules:");
    m_log.info("\t1 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket1));
    m_log.info("\t2 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket2));
    m_log.info("\t3 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket3));
    m_log.info("\t4 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket4));
    m_log.info("\t5 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket5));
    m_log.info("\t6 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket6));

    m_log.info("PDS Status:");

    m_log.info("\t* ENABLED           [ %s ]", pdsStatus.ENABLED ? "YES" : "NO");
    m_log.info("\t* OVER_TEMPERATURE  [ %s ]", pdsStatus.OVER_TEMPERATURE ? "YES" : "NO");
    m_log.info("\t* OVER_CURRENT      [ %s ]", pdsStatus.OVER_CURRENT ? "YES" : "NO");
    m_log.info("\t* STO_1             [ %s ]", pdsStatus.STO_1 ? "YES" : "NO");
    m_log.info("\t* STO_2             [ %s ]", pdsStatus.STO_2 ? "YES" : "NO");
    m_log.info("\t* FDCAN_TIMEOUT     [ %s ]", pdsStatus.FDCAN_TIMEOUT ? "YES" : "NO");
    m_log.info("\t* SUBMODULE_1_ERROR [ %s ]", pdsStatus.SUBMODULE_1_ERROR ? "YES" : "NO");
    m_log.info("\t* SUBMODULE_2_ERROR [ %s ]", pdsStatus.SUBMODULE_2_ERROR ? "YES" : "NO");
    m_log.info("\t* SUBMODULE_3_ERROR [ %s ]", pdsStatus.SUBMODULE_3_ERROR ? "YES" : "NO");
    m_log.info("\t* SUBMODULE_4_ERROR [ %s ]", pdsStatus.SUBMODULE_4_ERROR ? "YES" : "NO");
    m_log.info("\t* SUBMODULE_5_ERROR [ %s ]", pdsStatus.SUBMODULE_5_ERROR ? "YES" : "NO");
    m_log.info("\t* SUBMODULE_6_ERROR [ %s ]", pdsStatus.SUBMODULE_6_ERROR ? "YES" : "NO");
    m_log.info("\t* CHARGER_DETECTED  [ %s ]", pdsStatus.CHARGER_DETECTED ? "YES" : "NO");

    m_log.info("---------------------------------");

    m_log.info("Config data:");
    m_log.info("shutdown time: [ %u mS ] ", shutdownTime);
    m_log.info("Battery level 1: %0.2f", batteryLvl1 / 1000.0f);
    m_log.info("Battery level 2: %0.2f", batteryLvl2 / 1000.0f);

    m_log.info("---------------------------------");

    m_log.info("Metrology data:");
    m_log.info("Bus voltage: %0.2f", pdsBusVoltage / 1000.0f);
}

// Fill Power stage Ini structure
static void fillPsIni(PowerStage& ps, mINI::INIStructure& rIni, std::string sectionName)
{
    socketIndex_E brSocket         = socketIndex_E::UNASSIGNED;
    u32           brTriggerVoltage = 0;
    u32           ocdLevel         = 0;
    u32           ocdDelay         = 0;
    f32           temperatureLimit = 0.0f;

    ps.getBindBrakeResistor(brSocket);
    ps.getBrakeResistorTriggerVoltage(brTriggerVoltage);
    ps.getOcdLevel(ocdLevel);
    ps.getOcdDelay(ocdDelay);
    ps.getTemperatureLimit(temperatureLimit);

    rIni[sectionName]["type"]      = PdsModule::moduleType2String(moduleType_E::POWER_STAGE);
    rIni[sectionName]["BR Socket"] = floatToString((uint8_t)brSocket);
    rIni[sectionName]["BR Trigger voltage"] = floatToString(brTriggerVoltage);
    rIni[sectionName]["OCD level"]          = floatToString(ocdLevel);
    rIni[sectionName]["OCD delay"]          = floatToString(ocdDelay);
}

// Fill Brake resistor Ini structure
static void fillBrIni(BrakeResistor& br, mINI::INIStructure& rIni, std::string sectionName)
{
    f32 temperatureLimit = 0.0f;

    br.getTemperatureLimit(temperatureLimit);

    rIni[sectionName]["type"] = PdsModule::moduleType2String(moduleType_E::BRAKE_RESISTOR);
}

// Fill Brake resistor Ini structure
static void fillIcIni(IsolatedConv& ic, mINI::INIStructure& rIni, std::string sectionName)
{
    f32 temperatureLimit = 0.0f;

    ic.getTemperatureLimit(temperatureLimit);

    rIni[sectionName]["type"] = PdsModule::moduleType2String(moduleType_E::ISOLATED_CONVERTER);
}

static void fullModuleIni(Pds&                pds,
                          moduleType_E        moduleType,
                          mINI::INIStructure& rIni,
                          socketIndex_E       socketIndex)
{
    std::string sectionName = "Submodule " + std::to_string((int)socketIndex);

    switch (moduleType)
    {
        case moduleType_E::UNDEFINED:
            rIni[sectionName]["type"] = "NO MODULE";
            break;

        case moduleType_E::CONTROL_BOARD:
            break;

        case moduleType_E::BRAKE_RESISTOR:
        {
            auto br = pds.attachBrakeResistor(socketIndex);
            fillBrIni(*br, rIni, sectionName);
            break;
        }

        case moduleType_E::ISOLATED_CONVERTER:
        {
            auto ic = pds.attachIsolatedConverter(socketIndex);
            fillIcIni(*ic, rIni, sectionName);
            break;
        }

        case moduleType_E::POWER_STAGE:
        {
            auto ps = pds.attachPowerStage(socketIndex);
            fillPsIni(*ps, rIni, sectionName);
            break;
        }

            /* NEW MODULE TYPES HERE */

        default:
            break;
    }
}

void PdsCli::pdsSetupConfig(u16 id, const std::string& cfgPath)
{
    using err_E = mab::PdsModule::error_E;

    mab::Pds pds(id, m_candle);

    mINI::INIFile      pdsCfgFile(cfgPath);
    mINI::INIStructure pdsCfg;
    pdsCfgFile.read(pdsCfg);

    u32 shutdownTime = atoi(pdsCfg["Control board"]["shutdown time"].c_str());
    u32 battLvl1     = atoi(pdsCfg["Control board"]["battery level 1"].c_str());
    u32 battLvl2     = atoi(pdsCfg["Control board"]["battery level 2"].c_str());

    err_E result = pds.setShutdownTime(shutdownTime);
    if (result != err_E::OK)
        m_log.error("PDS Config error [ %u ] [ %s:%u ]", result, __FILE__, __LINE__);

    result = pds.setBatteryVoltageLevels(battLvl1, battLvl2);
    if (result != err_E::OK)
        m_log.error("PDS Config error [ %u ] [ %s:%u ]", result, __FILE__, __LINE__);
}

void PdsCli::pdsReadConfig(u16 id, const std::string& cfgPath)
{
    mINI::INIStructure readIni; /**< mINI structure for read data */
    Pds                pds(id, m_candle);
    u32                shutDownTime = 0;
    u32                batLvl1      = 0;
    u32                batLvl2      = 0;
    Pds::modulesSet_S  pdsModules   = pds.getModules();

    std::string configName = cfgPath;
    if (std::filesystem::path(configName).extension() == "")
        configName += ".cfg";

    pds.getShutdownTime(shutDownTime);
    pds.getBatteryVoltageLevels(batLvl1, batLvl2);

    readIni["Control board"]["CAN ID"]          = floatToString(id);
    readIni["Control board"]["CAN BAUD"]        = "";
    readIni["Control board"]["shutdown time"]   = floatToString(shutDownTime);
    readIni["Control board"]["battery level 1"] = floatToString(batLvl1);
    readIni["Control board"]["battery level 2"] = floatToString(batLvl2);
    fullModuleIni(pds, pdsModules.moduleTypeSocket1, readIni, socketIndex_E::SOCKET_1);
    fullModuleIni(pds, pdsModules.moduleTypeSocket2, readIni, socketIndex_E::SOCKET_2);
    fullModuleIni(pds, pdsModules.moduleTypeSocket3, readIni, socketIndex_E::SOCKET_3);
    fullModuleIni(pds, pdsModules.moduleTypeSocket4, readIni, socketIndex_E::SOCKET_4);
    fullModuleIni(pds, pdsModules.moduleTypeSocket5, readIni, socketIndex_E::SOCKET_5);
    fullModuleIni(pds, pdsModules.moduleTypeSocket6, readIni, socketIndex_E::SOCKET_6);

    mINI::INIFile configFile(configName);
    configFile.write(readIni);
}

void PdsCli::pdsStoreConfig(u16 id)
{
    using err_E = mab::PdsModule::error_E;

    mab::Pds pds(id, m_candle);

    err_E result = pds.saveConfig();

    if (result != err_E::OK)
        m_log.error("PDS Configuration save error [ %u ] [ %s:%u ]", result, __FILE__, __LINE__);
}

socketIndex_E PdsCli::decodeSocketIndex(u8 numericSocketIndex)
{
    return socketIndex_E::UNASSIGNED;
}

void PdsCli::powerStageCmdParse(void)
{
}
void PdsCli::brakeResistorCmdParse(void)
{
}
void PdsCli::isolatedConverterCmdParse(void)
{
}
