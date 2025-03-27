#include "ui.hpp"
#include "pds_cli.hpp"
#include "mab_def.hpp"
#include "configHelpers.hpp"

/*
    PDS Ini fields keywords
    Using const char* instead of safer c++ features like std::string is because INI maps and logger
    requires c-strings and it simplifies the code since it does not require calls to .data() or
   c_str() methods and still beeing quite safe as we are giving strings in ""
*/
constexpr const char* CONTROL_BOARD_INI_SECTION = PdsModule::mType2Str(moduleType_E::CONTROL_BOARD);
constexpr const char* CAN_ID_INI_KEY            = "CAN ID";
constexpr const char* CAN_BAUD_INI_KEY          = "CAN BAUD";
constexpr const char* SHUTDOWN_TIME_INI_KEY     = "SHUTDOWN TIME";
constexpr const char* BATT_LVL_1_INI_KEY        = "BATTERY LEVEL 1";
constexpr const char* BATT_LVL_2_INI_KEY        = "BATTERY LEVEL 2";

constexpr const char* TYPE_INI_KEY       = "TYPE";
constexpr const char* TEMP_LIMIT_INI_KEY = "TEMPERATURE LIMIT";
constexpr const char* OCD_LEVEL_INI_KEY  = "OCD LEVEL";
constexpr const char* OCD_DELAY_INI_KEY  = "OCD DELAY";
constexpr const char* BR_SOCKET_INI_KEY  = "BR SOCKET";
constexpr const char* BR_TRIG_V_INI_KEY  = "BR TRIGGER VOLTAGE";

PdsCli::PdsCli(CLI::App& rootCli, mab::Candle& candle) : m_rootCli(rootCli), m_candle(candle)
{
    m_log.m_tag   = "PDS";
    m_log.m_layer = Logger::ProgramLayer_E::TOP;

    m_pdsCmd = m_rootCli.add_subcommand("pds", "Tweak the PDS device");

    m_pdsCmd->add_option("<CAN_ID>", m_canId, "MAB FD-CAN protocol :: Target device ID")
        ->required();

    m_infoCmd = m_pdsCmd->add_subcommand("info", "Display debug info about PDS device");
    m_configSetupCmd =
        m_pdsCmd->add_subcommand("setup_cfg", "Configure PDS device with the .cfg file");

    m_configSetupCmd->add_option("<config_file>", m_cfgFilePath, "PDS configuration .cfg file.")
        ->required();

    m_interactiveSetupCmd = m_pdsCmd->add_subcommand("setup_interactive", "Interactive setup");

    m_configReadCmd =
        m_pdsCmd->add_subcommand("read_cfg", "Read device configuration and save to file");

    m_configReadCmd->add_option("<config_file>", m_cfgFilePath, "PDS configuration .cfg file.")
        ->required();

    m_configSaveCmd =
        m_pdsCmd->add_subcommand("save", "Store current configuration in device memory");

    m_setCanIdCmd =
        m_pdsCmd->add_subcommand("set_can_id", "Assign new FD CAN ID to the PDS device");

    m_setCanIdCmd->add_option("<NEW_CAN_ID>", m_newCanId, "New CAN ID")->required();

    m_setBatteryLevelCmd =
        m_pdsCmd->add_subcommand("set_battery_level", "Set the battery voltage levels");

    m_setBatteryLevelCmd->add_option("<level1>", m_batteryLevel1, "Battery voltage level 1")
        ->required();
    m_setBatteryLevelCmd->add_option("<level2>", m_batteryLevel2, "Battery voltage level 2")
        ->required();

    m_setShutdownTimeCmd = m_pdsCmd->add_subcommand("set_shutdown_time", "Set the shutdown time");

    m_setShutdownTimeCmd->add_option("<time>", m_shutdownTime, "Shutdown time in ms")->required();

    m_disableCmd = m_pdsCmd->add_subcommand("disable", "Disable the PDS device");

    // POWER STAGE commands set
    m_powerStageCmd = m_pdsCmd->add_subcommand("ps", "Manage the Power Stage submodule");

    m_powerStageCmd
        ->add_option("<socket_index>", m_submoduleSocketNumber, "Submodule socket number")
        ->required();

    m_psInfoCmd = m_powerStageCmd->add_subcommand("info", "Display debug info about Power Stage");

    m_psEnableCmd = m_powerStageCmd->add_subcommand("enable", "Enable the Power Stage submodule");

    m_psDisableCmd =
        m_powerStageCmd->add_subcommand("disable", "Disable the Power Stage submodule");

    m_psSetOvcLevelCmd =
        m_powerStageCmd->add_subcommand("set_ovc_level", "Set the Overcurrent Detection level");
    m_psSetOvcLevelCmd->add_option("<ovc_level>", m_ovcLevel, "Overcurrent Detection level")
        ->required();

    m_psGetOvcLevelCmd =
        m_powerStageCmd->add_subcommand("get_ovc_level", "Get the Overcurrent Detection level");

    m_psSetOvcDelayCmd =
        m_powerStageCmd->add_subcommand("set_ovc_delay", "Set the Overcurrent Detection delay");
    m_psSetOvcDelayCmd->add_option("<ovc_delay>", m_ovcDelay, "Overcurrent Detection delay")
        ->required();

    m_psGetOvcDelayCmd =
        m_powerStageCmd->add_subcommand("get_ovc_delay", "Get the Overcurrent Detection delay");

    m_psSetTempLimitCmd =
        m_powerStageCmd->add_subcommand("set_temp_limit", "Set the Temperature Limit");

    m_psSetTempLimitCmd->add_option("<temp_limit>", m_tempLimit, "Temperature Limit")->required();

    m_psGetTempLimitCmd =
        m_powerStageCmd->add_subcommand("get_temp_limit", "Get the Temperature Limit");

    m_psSetBrCmd = m_powerStageCmd->add_subcommand("set_br", "Set the Brake Resistor Socket index");

    m_psSetBrCmd->add_option("<socket_index>", m_brSocket, "Brake Resistor Socket index")
        ->required();

    m_psGetBrCmd = m_powerStageCmd->add_subcommand("get_br", "Get the Brake Resistor Socket index");

    m_psSetBrTriggerCmd =
        m_powerStageCmd->add_subcommand("set_br_trigger", "Set the Brake Resistor Trigger Voltage");

    m_psGetBrTriggerCmd =
        m_powerStageCmd->add_subcommand("get_br_trigger", "Get the Brake Resistor Trigger Voltage");

    // BRAKE RESISTOR commands set

    m_brakeResistorCmd = m_pdsCmd->add_subcommand("br", "Manage the Brake Resistor submodule");

    m_brakeResistorCmd
        ->add_option("<socket_index>", m_submoduleSocketNumber, "Submodule socket number")
        ->required();

    m_brInfoCmd =
        m_brakeResistorCmd->add_subcommand("info", "Display debug info about Brake Resistor");

    m_brSetTempLimitCmd =
        m_brakeResistorCmd->add_subcommand("set_temp_limit", "Set the Temperature Limit");

    m_brSetTempLimitCmd->add_option("<temp_limit>", m_tempLimit, "Temperature Limit")->required();

    m_brGetTempLimitCmd =
        m_brakeResistorCmd->add_subcommand("get_temp_limit", "Get the Temperature Limit");

    // ISOLATED CONVERTER commands set

    m_isolatedConverterCmd =
        m_pdsCmd->add_subcommand("ic", "Manage the Isolated Converter submodule");

    m_isolatedConverterCmd
        ->add_option("<socket_index>", m_submoduleSocketNumber, "Submodule socket number")
        ->required();

    m_icInfoCmd = m_isolatedConverterCmd->add_subcommand(
        "info", "Display debug info about Isolated Converter");

    m_icEnableCmd =
        m_isolatedConverterCmd->add_subcommand("enable", "Enable the Isolated Converter submodule");

    m_icDisableCmd = m_isolatedConverterCmd->add_subcommand(
        "disable", "Disable the Isolated Converter submodule");

    m_icSetOvcLevelCmd = m_isolatedConverterCmd->add_subcommand(
        "set_ovc_level", "Set the Overcurrent Detection level");

    m_icSetOvcLevelCmd->add_option("<ovc_level>", m_ovcLevel, "Overcurrent Detection level")
        ->required();

    m_icGetOvcLevelCmd = m_isolatedConverterCmd->add_subcommand(
        "get_ovc_level", "Get the Overcurrent Detection level");

    m_icSetOvcDelayCmd = m_isolatedConverterCmd->add_subcommand(
        "set_ovc_delay", "Set the Overcurrent Detection delay");

    m_icSetOvcDelayCmd->add_option("<ovc_delay>", m_ovcDelay, "Overcurrent Detection delay")
        ->required();

    m_icGetOvcDelayCmd = m_isolatedConverterCmd->add_subcommand(
        "get_ovc_delay", "Get the Overcurrent Detection delay");

    m_icSetTempLimitCmd =
        m_isolatedConverterCmd->add_subcommand("set_temp_limit", "Set the Temperature Limit");

    m_icSetTempLimitCmd->add_option("<temp_limit>", m_tempLimit, "Temperature Limit")->required();

    m_icGetTempLimitCmd =
        m_isolatedConverterCmd->add_subcommand("get_temp_limit", "Get the Temperature Limit");
}

static bool isCanIdValid(u16 canId)
{
    return ((canId >= CAN_MIN_ID) || (canId <= CAN_MAX_ID));
}

void PdsCli::parse(void)
{
    PdsModule::error_E result = PdsModule::error_E::OK;

    if (m_pdsCmd->parsed())
    {
        m_pds.init(m_canId);

        m_log.info("PDS - Power Distribution System :: CAN ID [ %u ]", m_canId);

        if (m_infoCmd->parsed())
        {
            pdsSetupInfo();
        }

        else if (m_configSetupCmd->parsed())
        {
            m_log.debug("setup config command parsed");
            pdsSetupConfig(m_cfgFilePath);
        }

        else if (m_interactiveSetupCmd->parsed())
        {
            m_log.warn(
                "This command is under development. For now please be patient and "
                "https://pl.wikipedia.org/wiki/RTFM");
        }

        else if (m_configReadCmd->parsed())
        {
            pdsReadConfig(m_cfgFilePath);
        }

        else if (m_configSaveCmd->parsed())
        {
            pdsStoreConfig();
        }

        else if (m_setCanIdCmd->parsed())
        {
            if (!isCanIdValid(m_newCanId))
            {
                m_log.error("Given CAN ID ( %u ) is invalid. Acceptable range is [ %u - %u]",
                            m_newCanId,
                            CAN_MIN_ID,
                            CAN_MAX_ID);
                return;
            }
            result = m_pds.setCanId(m_newCanId);
            if (result != PdsModule::error_E::OK)
                m_log.error("Setting CAN ID failed [ %s ]", PdsModule::error2String(result));
            else
                m_log.success("New CAN ID set [ %u ]", m_newCanId);
        }

        else if (m_powerStageCmd->parsed())
        {
            powerStageCmdParse();
        }

        else if (m_brakeResistorCmd->parsed())
        {
            brakeResistorCmdParse();
        }

        else if (m_isolatedConverterCmd->parsed())
        {
            isolatedConverterCmdParse();
        }

        else if (m_setBatteryLevelCmd->parsed())
        {
            result = m_pds.setBatteryVoltageLevels(m_batteryLevel1, m_batteryLevel2);
            if (result != PdsModule::error_E::OK)
                m_log.error("Battery levels setting failed [ %s ]",
                            PdsModule::error2String(result));
            else
                m_log.success("Battery levels set [ %u, %u ]", m_batteryLevel1, m_batteryLevel2);
        }

        else if (m_setShutdownTimeCmd->parsed())
        {
            result = m_pds.setShutdownTime(m_shutdownTime);
            if (result != PdsModule::error_E::OK)
                m_log.error("Shutdown time setting failed [ %s ]", PdsModule::error2String(result));
            else
                m_log.success("Shutdown time set [ %u ]", m_shutdownTime);
        }

        else if (m_disableCmd->parsed())
        {
            m_pds.shutdown();
            m_log.success("PDS disabled");
        }
        else
            m_log.error("PDS subcommand is missing");
    }
}

void PdsCli::powerStageCmdParse(void)
{
    socketIndex_E      socketIndex = decodeSocketIndex(m_submoduleSocketNumber);
    PdsModule::error_E result      = PdsModule::error_E::OK;

    if (!m_pds.verifyModuleSocket(moduleType_E::POWER_STAGE, socketIndex))
    {
        m_log.error("Invalid socket number for Power Stage submodule");
        return;
    }

    auto ps = m_pds.attachPowerStage(socketIndex);

    if (ps == nullptr)
    {
        m_log.error("Power Stage submodule is not available");
        return;
    }

    m_log.info("Power Stage submodule :: Socket index [ %u ]", m_submoduleSocketNumber);

    if (m_psInfoCmd->parsed())
    {
        powerStageStatus_S   psStatus         = {0};
        u32                  busVoltage       = 0;
        s32                  current          = 0;
        u32                  ovcLevel         = 0;
        u32                  ovcDelay         = 0;
        f32                  temperature      = 0.0f;
        f32                  temperatureLimit = 0.0f;
        socketIndex_E        brSocket         = socketIndex_E::UNASSIGNED;
        u32                  brTrigger        = 0;
        mab::moduleVersion_E version          = mab::moduleVersion_E::UNKNOWN;

        result = ps->getBoardVersion(version);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get version failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.info("Version [ %d ]", static_cast<u8>(version));

        result = ps->getStatus(psStatus);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get status failed [ %s ]", PdsModule::error2String(result));
        else
        {
            m_log.info("Status:");
            m_log.info("\t* ENABLED           [ %s ]", psStatus.ENABLED ? "YES" : "NO");
            m_log.info("\t* OVER_TEMPERATURE  [ %s ]", psStatus.OVER_TEMPERATURE ? "YES" : "NO");
            m_log.info("\t* OVER_CURRENT      [ %s ]", psStatus.OVER_CURRENT ? "YES" : "NO");
        }

        m_log.info("Measurements:");

        result = ps->getOutputVoltage(busVoltage);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get output voltage failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("\t* Output voltage [ %0.2f ]", busVoltage / 1000.0f);

        result = ps->getLoadCurrent(current);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get load current failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("\t* Load current [ %0.2f ]", current / 1000.0f);

        result = ps->getTemperature(temperature);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get temperature failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("\t* Temperature [ %0.2f ]", temperature);

        m_log.info("Configuration:");

        result = ps->getOcdLevel(ovcLevel);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get OVC level failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.info("\t* OVC level [ %u ]", ovcLevel);

        result = ps->getOcdDelay(ovcDelay);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get OVC delay failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.info("\t* OVC delay [ %u ]", ovcDelay);

        result = ps->getTemperatureLimit(temperatureLimit);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get temperature limit failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("\t* Temperature limit [ %0.2f ]", temperatureLimit);

        result = ps->getBindBrakeResistor(brSocket);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get brake resistor failed [ %s ]",
                        PdsModule::error2String(result));
        else
        {
            if (brSocket == socketIndex_E::UNASSIGNED)
                m_log.info("\t* Brake resistor is not set");
            else
                m_log.info("\t* Brake resistor socket [ %u ]", (u8)brSocket);
        }

        if (brSocket != socketIndex_E::UNASSIGNED)
        {
            result = ps->getBrakeResistorTriggerVoltage(brTrigger);
            if (result != PdsModule::error_E::OK)
                m_log.error("Power Stage get brake resistor trigger voltage failed [ %s ]",
                            PdsModule::error2String(result));
            else
                m_log.info("\t* Brake resistor trigger voltage [ %u ]", brTrigger);
        }
    }

    else if (m_psEnableCmd->parsed())
    {
        result = ps->enable();
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage enabling failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.success("Module enabled");
    }

    else if (m_psDisableCmd->parsed())
    {
        result = ps->disable();
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage disabling failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.success("Module disabled");
    }

    else if (m_psSetOvcLevelCmd->parsed())
    {
        result = ps->setOcdLevel(m_ovcLevel);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage set OVC level failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.success("OVC level set [ %u ]", m_ovcLevel);
    }

    else if (m_psGetOvcLevelCmd->parsed())
    {
        u32 ovcLevel = 0;
        result       = ps->getOcdLevel(ovcLevel);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get OVC level failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.success("OVC level [ %u ]", ovcLevel);
    }

    else if (m_psSetOvcDelayCmd->parsed())
    {
        result = ps->setOcdDelay(m_ovcDelay);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage set OVC delay failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.success("OVC delay set [ %u ]", m_ovcDelay);
    }

    else if (m_psGetOvcDelayCmd->parsed())
    {
        u32 ovcDelay = 0;
        result       = ps->getOcdDelay(ovcDelay);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get OVC delay failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.success("OVC delay [ %u ]", ovcDelay);
    }

    else if (m_psSetTempLimitCmd->parsed())
    {
        result = ps->setTemperatureLimit(m_tempLimit);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage set temperature limit failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("New temperature limit set [ %0.2f ]", m_tempLimit);
    }

    else if (m_psGetTempLimitCmd->parsed())
    {
        f32 tempLimit = 0.0f;
        result        = ps->getTemperatureLimit(tempLimit);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get temperature limit failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("Temperature limit [ %0.2f ]", tempLimit);
    }

    else if (m_psSetBrCmd->parsed())
    {
        result = ps->bindBrakeResistor(decodeSocketIndex(m_brSocket));

        if (!m_pds.verifyModuleSocket(moduleType_E::BRAKE_RESISTOR, decodeSocketIndex(m_brSocket)))
        {
            m_log.error("Invalid socket number for Brake Resistor submodule");
            return;
        }

        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage set brake resistor failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("Brake resistor set");
    }

    else if (m_psGetBrCmd->parsed())
    {
        socketIndex_E brSocket = socketIndex_E::UNASSIGNED;
        result                 = ps->getBindBrakeResistor(brSocket);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get brake resistor failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("Brake resistor socket [ %u ]", (u8)brSocket);
    }

    else if (m_psSetBrTriggerCmd->parsed())
    {
        result = ps->setBrakeResistorTriggerVoltage(m_brTrigger);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage set brake resistor trigger voltage failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("Brake resistor trigger voltage set");
    }

    else if (m_psGetBrTriggerCmd->parsed())
    {
        u32 brTrigger = 0;
        result        = ps->getBrakeResistorTriggerVoltage(brTrigger);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get brake resistor trigger voltage failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("Brake resistor trigger voltage [ %u ]", brTrigger);
    }
    else
        m_log.error("PS subcommand is missing");
}

void PdsCli::brakeResistorCmdParse(void)
{
    socketIndex_E      socketIndex = decodeSocketIndex(m_submoduleSocketNumber);
    PdsModule::error_E result      = PdsModule::error_E::OK;

    if (!m_pds.verifyModuleSocket(moduleType_E::BRAKE_RESISTOR, socketIndex))
    {
        m_log.error("Invalid socket number for Brake Resistor submodule");
        return;
    }

    auto br = m_pds.attachBrakeResistor(socketIndex);

    if (br == nullptr)
    {
        m_log.error("Brake Resistor submodule is not available");
        return;
    }

    m_log.info("Brake Resistor submodule :: Socket index [ %u ]", m_submoduleSocketNumber);

    if (m_brInfoCmd->parsed())
    {
        brakeResistorStatus_S brStatus         = {0};
        f32                   temperature      = 0.0f;
        f32                   temperatureLimit = 0.0f;
        mab::moduleVersion_E  version          = mab::moduleVersion_E::UNKNOWN;

        result = br->getBoardVersion(version);
        if (result != PdsModule::error_E::OK)
            m_log.error("Brake Resistor get version failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("Version [ %d ]", static_cast<u8>(version));

        result = br->getStatus(brStatus);
        if (result != PdsModule::error_E::OK)
            m_log.error("Brake Resistor get status failed [ %s ]", PdsModule::error2String(result));
        else
        {
            m_log.info("Status:");
            m_log.info("\t* ENABLED           [ %s ]", brStatus.ENABLED ? "YES" : "NO");
            m_log.info("\t* OVER_TEMPERATURE  [ %s ]", brStatus.OVER_TEMPERATURE ? "YES" : "NO");
        }

        m_log.info("Measurements:");
        result = br->getTemperature(temperature);
        if (result != PdsModule::error_E::OK)
            m_log.error("Brake Resistor get temperature failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("\t* Temperature [ %0.2f ]", temperature);

        m_log.info("Configuration:");
        result = br->getTemperatureLimit(temperatureLimit);
        if (result != PdsModule::error_E::OK)
            m_log.error("Brake Resistor get temperature limit failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("\t* Temperature limit [ %0.2f ]", temperatureLimit);
    }

    else if (m_brSetTempLimitCmd->parsed())
    {
        result = br->setTemperatureLimit(m_tempLimit);
        if (result != PdsModule::error_E::OK)
            m_log.error("Brake Resistor set temperature limit failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("New temperature limit set [ %0.2f ]", m_tempLimit);
    }

    else if (m_brGetTempLimitCmd->parsed())
    {
        f32 tempLimit = 0.0f;
        result        = br->getTemperatureLimit(tempLimit);
        if (result != PdsModule::error_E::OK)
            m_log.error("Brake Resistor get temperature limit failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("Temperature limit [ %0.2f ]", tempLimit);
    }
    else
        m_log.error("BR subcommand is missing");
}

void PdsCli::isolatedConverterCmdParse(void)
{
    socketIndex_E      socketIndex = decodeSocketIndex(m_submoduleSocketNumber);
    PdsModule::error_E result      = PdsModule::error_E::OK;

    if (!m_pds.verifyModuleSocket(moduleType_E::ISOLATED_CONVERTER, socketIndex))
    {
        m_log.error("Invalid socket number for Isolated Converter submodule");
        return;
    }

    auto ic = m_pds.attachIsolatedConverter(socketIndex);

    if (ic == nullptr)
    {
        m_log.error("Isolated Converter submodule is not available");
        return;
    }

    m_log.info("Isolated Converter submodule :: Socket index [ %u ]", m_submoduleSocketNumber);

    if (m_icInfoCmd->parsed())
    {
        isolatedConverterStatus_S icStatus         = {0};
        u32                       busVoltage       = 0;
        s32                       current          = 0;
        u32                       ovcLevel         = 0;
        u32                       ovcDelay         = 0;
        f32                       temperature      = 0.0f;
        f32                       temperatureLimit = 0.0f;
        mab::moduleVersion_E      version          = mab::moduleVersion_E::UNKNOWN;

        result = ic->getBoardVersion(version);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter get version failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("Version [ %d ]", static_cast<u8>(version));

        result = ic->getStatus(icStatus);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter get status failed [ %s ]",
                        PdsModule::error2String(result));
        else
        {
            m_log.info("Status:");
            m_log.info("\t* ENABLED           [ %s ]", icStatus.ENABLED ? "YES" : "NO");
            m_log.info("\t* OVER_TEMPERATURE  [ %s ]", icStatus.OVER_TEMPERATURE ? "YES" : "NO");
        }

        m_log.info("Measurements:");

        result = ic->getOutputVoltage(busVoltage);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter get output voltage failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("\t* Output voltage [ %0.2f ]", busVoltage / 1000.0f);

        result = ic->getLoadCurrent(current);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter get load current failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("\t* Load current [ %0.2f ]", current / 1000.0f);

        result = ic->getTemperature(temperature);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter get temperature failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("\t* Temperature [ %0.2f ]", temperature);

        m_log.info("Configuration:");

        result = ic->getOcdLevel(ovcLevel);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter get OVC level failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("\t* OVC level [ %u ]", ovcLevel);

        result = ic->getOcdDelay(ovcDelay);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter get OVC delay failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("\t* OVC delay [ %u ]", ovcDelay);

        result = ic->getTemperatureLimit(temperatureLimit);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter get temperature limit failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("\t* Temperature limit [ %0.2f ]", temperatureLimit);
    }

    else if (m_icEnableCmd->parsed())
    {
        result = ic->enable();
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter enabling failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("Module enabled");
    }

    else if (m_icDisableCmd->parsed())
    {
        result = ic->disable();
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter disabling failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("Module disabled");
    }

    else if (m_icSetOvcLevelCmd->parsed())
    {
        result = ic->setOcdLevel(m_ovcLevel);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter set OVC level failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("OVC level set [ %u ]", m_ovcLevel);
    }

    else if (m_icGetOvcLevelCmd->parsed())
    {
        u32 ovcLevel = 0;
        result       = ic->getOcdLevel(ovcLevel);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter get OVC level failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("OVC level [ %u ]", ovcLevel);
    }

    else if (m_icSetOvcDelayCmd->parsed())
    {
        result = ic->setOcdDelay(m_ovcDelay);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter set OVC delay failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("OVC delay set [ %u ]", m_ovcDelay);
    }

    else if (m_icGetOvcDelayCmd->parsed())
    {
        u32 ovcDelay = 0;
        result       = ic->getOcdDelay(ovcDelay);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter get OVC delay failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("OVC delay [ %u ]", ovcDelay);
    }

    else if (m_icSetTempLimitCmd->parsed())
    {
        result = ic->setTemperatureLimit(m_tempLimit);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter set temperature limit failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("New temperature limit set [ %0.2f ]", m_tempLimit);
    }

    else if (m_icGetTempLimitCmd->parsed())
    {
        f32 tempLimit = 0.0f;
        result        = ic->getTemperatureLimit(tempLimit);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter get temperature limit failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("Temperature limit [ %0.2f ]", tempLimit);
    }
    else
        m_log.error("IC subcommand is missing");
}

void PdsCli::pdsSetupInfo()
{
    mab::Pds::modulesSet_S pdsModules = m_pds.getModules();

    u32                       shutdownTime  = 0;
    u32                       batteryLvl1   = 0;
    u32                       batteryLvl2   = 0;
    u32                       pdsBusVoltage = 0;
    mab::controlBoardStatus_S pdsStatus     = {0};

    m_pds.getStatus(pdsStatus);
    m_pds.getBusVoltage(pdsBusVoltage);
    m_pds.getShutdownTime(shutdownTime);
    m_pds.getBatteryVoltageLevels(batteryLvl1, batteryLvl2);

    m_log.info("Power Distribution Module");

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
    m_log.info("Bus voltage: %0.2f V", pdsBusVoltage / 1000.0f);
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

    rIni[sectionName][TYPE_INI_KEY] = PdsModule::mType2Str(moduleType_E::POWER_STAGE);
    rIni[sectionName][TEMP_LIMIT_INI_KEY] =
        floatToString(temperatureLimit) + "\t; Temperature limit [ ^C ]";
    rIni[sectionName][OCD_LEVEL_INI_KEY] =
        floatToString(ocdLevel, true) + "\t\t; Over-current detection level [ mA ]";
    rIni[sectionName][OCD_DELAY_INI_KEY] =
        floatToString(ocdDelay, true) + "\t\t; Over-current detection delay [ ms ]";
    rIni[sectionName][BR_SOCKET_INI_KEY] =
        floatToString((uint8_t)brSocket, true) +
        "\t\t\t; Socket index where corresponding Brake Resistor is connected";
    rIni[sectionName][BR_TRIG_V_INI_KEY] =
        floatToString(brTriggerVoltage, true) + "\t; Brake resistor trigger voltage [ mV ]";
}

// Fill Brake resistor Ini structure
static void fillBrIni(BrakeResistor& br, mINI::INIStructure& rIni, std::string sectionName)
{
    f32 temperatureLimit = 0.0f;

    br.getTemperatureLimit(temperatureLimit);

    rIni[sectionName][TYPE_INI_KEY] = PdsModule::mType2Str(moduleType_E::BRAKE_RESISTOR);
    rIni[sectionName][TEMP_LIMIT_INI_KEY] =
        floatToString(temperatureLimit) + "\t; Temperature limit [ ^C ]";
}

// Fill Isolated Converter Ini structure
static void fillIcIni(IsolatedConv& ic, mINI::INIStructure& rIni, std::string sectionName)
{
    f32 temperatureLimit = 0.0f;
    u32 ocdLevel         = 0;
    u32 ocdDelay         = 0;

    ic.getTemperatureLimit(temperatureLimit);
    ic.getOcdLevel(ocdLevel);
    ic.getOcdDelay(ocdDelay);

    rIni[sectionName][TYPE_INI_KEY] = PdsModule::mType2Str(moduleType_E::ISOLATED_CONVERTER);
    rIni[sectionName][TEMP_LIMIT_INI_KEY] =
        floatToString(temperatureLimit) + "\t; Temperature limit [ ^C ]";
    rIni[sectionName][OCD_LEVEL_INI_KEY] =
        floatToString(ocdLevel, true) + "\t\t; Over-current detection level [ mA ]";
    rIni[sectionName][OCD_DELAY_INI_KEY] =
        floatToString(ocdDelay, true) + "\t\t; Over-current detection delay [ ms ]";
}

static void fullModuleIni(Pds&                pds,
                          moduleType_E        moduleType,
                          mINI::INIStructure& rIni,
                          socketIndex_E       socketIndex)
{
    std::string sectionName = "Socket " + std::to_string((int)socketIndex);

    switch (moduleType)
    {
        case moduleType_E::UNDEFINED:
            rIni[sectionName]["type"] = "NO MODULE";
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
        case moduleType_E::CONTROL_BOARD:
        default:
            break;
    }
}

static std::optional<canBaudrate_E> parseCanBaudIniString(std::string_view baudString)
{
    if (baudString == "1M")
        return canBaudrate_E::BAUD_1M;
    else if (baudString == "2M")
        return canBaudrate_E::BAUD_2M;
    else if (baudString == "5M")
        return canBaudrate_E::BAUD_5M;
    else if (baudString == "8M")
        return canBaudrate_E::BAUD_8M;

    return std::nullopt;
}
void PdsCli::setupCtrlConfig(mINI::INIMap<std::string>& iniMap)
{
    using err_E  = mab::PdsModule::error_E;
    err_E result = err_E::OK;

    // CAN Id
    if (iniMap.has(CAN_ID_INI_KEY))
    {
        u16 canId = atoi(iniMap[CAN_ID_INI_KEY].c_str());
        m_log.debug("CAN ID field found with value [ %u ]", canId);
        result = m_pds.setCanId(canId);
        if (result != PdsModule::error_E::OK)
            m_log.error("CAN ID setting failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.success("CAN ID set [ %u ]", canId);
    }
    else
    {
        m_log.error("CAN ID field missing so will be ignored");
    }

    // CAN Baudrate
    if (iniMap.has(CAN_BAUD_INI_KEY))
    {
        std::string_view             canBaudString = iniMap[CAN_BAUD_INI_KEY];
        std::optional<canBaudrate_E> canBaud       = parseCanBaudIniString(canBaudString);
        if (canBaud.has_value())
        {
            result = m_pds.setCanBaudrate(canBaud.value());
            if (result != PdsModule::error_E::OK)
                m_log.error("CAN BAUD setting failed [ %s ]", PdsModule::error2String(result));
            else
                m_log.success("CAN Baud set [ %s ]", canBaudString.data());
        }
        else
        {
            m_log.error("Given CAN Baud [ %s ] is INVALID! Acceptable values are: 1M, 2M, 5M, 8M");
            m_log.warn("CAN Baudrate setting was omitted!");
        }
    }
    else
    {
        m_log.error("CAN Baudrate field missing so will be ignored");
    }

    // Shutdown time
    if (iniMap.has(SHUTDOWN_TIME_INI_KEY))
    {
        u32 shutdownTime = atoi(iniMap[SHUTDOWN_TIME_INI_KEY].c_str());
        m_log.debug("Shutdown time field found with value [ %u ]", shutdownTime);
        result = m_pds.setShutdownTime(shutdownTime);
        if (result != PdsModule::error_E::OK)
            m_log.error("Shutdown time setting failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.success("Shutdown time set [ %u ]", shutdownTime);
    }
    else
    {
        m_log.error("Shutdown time field missing so will be ignored");
    }

    // Battery voltage levels
    if (iniMap.has(BATT_LVL_1_INI_KEY) && iniMap.has(BATT_LVL_2_INI_KEY))
    {
        u32 battLvl1 = atoi(iniMap[BATT_LVL_1_INI_KEY].c_str());
        m_log.debug("Battery level 1 field found with value [ %u ]", battLvl1);
        u32 battLvl2 = atoi(iniMap[BATT_LVL_2_INI_KEY].c_str());
        m_log.debug("Battery level 2 field found with value [ %u ]", battLvl2);
        result = m_pds.setBatteryVoltageLevels(battLvl1, battLvl2);
        if (result != PdsModule::error_E::OK)
            m_log.error("Battery levels setting failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.success("Battery levels set [ %u, %u ]", battLvl1, battLvl2);
    }
    else
    {
        m_log.error("Battery levels field missing so will be ignored");
    }
}

void PdsCli::setupModuleCfg(moduleType_E type, socketIndex_E si, mINI::INIMap<std::string>& iniMap)
{
    if (type == moduleType_E::POWER_STAGE)
    {
        auto powerStage = m_pds.attachPowerStage(si);
        if (powerStage == nullptr)
        {
            m_log.error("Attaching Power Stage module at socket [ %u ] failed...", (u8)si);
        }
        else
        {
            setupPsCfg(*powerStage, iniMap);
        }
    }

    if (type == moduleType_E::ISOLATED_CONVERTER)
    {
        auto isolatedConverter = m_pds.attachIsolatedConverter(si);
        if (isolatedConverter == nullptr)
        {
            m_log.error("Attaching Isolated converter module at socket [ %u ] failed...", (u8)si);
        }
        else
        {
            setupIcCfg(*isolatedConverter, iniMap);
        }
    }

    if (type == moduleType_E::BRAKE_RESISTOR)
    {
        auto brakeResistor = m_pds.attachBrakeResistor(si);
        if (brakeResistor == nullptr)
        {
            m_log.error("Attaching Brake resistor module at socket [ %u ] failed...", (u8)si);
        }
        else
        {
            setupBrCfg(*brakeResistor, iniMap);
        }
    }
}

void PdsCli::setupPsCfg(PowerStage& ps, mINI::INIMap<std::string>& iniMap)
{
    m_log.debug("Setting up config for Power Stage [ %u ] module", ps.getSocketIndex());

    PdsModule::error_E result = PdsModule::error_E::OK;

    if (iniMap.has(TEMP_LIMIT_INI_KEY))
    {
        f32 temperatureLimit = atof(iniMap[TEMP_LIMIT_INI_KEY].c_str());
        m_log.debug("Temperature limit field found with value [ %.2f ]", temperatureLimit);
        result = ps.setTemperatureLimit(temperatureLimit);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage set temperature limit failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("New temperature limit set [ %.2f ]", temperatureLimit);
    }
    else
    {
        m_log.debug("Temperature limit field missing so will be ignored");
    }

    if (iniMap.has(OCD_LEVEL_INI_KEY))
    {
        u32 ocdLevel = atoi(iniMap[OCD_LEVEL_INI_KEY].c_str());
        m_log.debug("OCD level field found with value [ %u ]", ocdLevel);
        result = ps.setOcdLevel(ocdLevel);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage set OCD level failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.success("OCD level set [ %u ]", ocdLevel);
    }
    else
    {
        m_log.debug("OCD level field missing so will be ignored");
    }

    if (iniMap.has(OCD_DELAY_INI_KEY))
    {
        u32 ocdDelay = atoi(iniMap[OCD_DELAY_INI_KEY].c_str());
        m_log.debug("OCD delay field found with value [ %u ]", ocdDelay);
        result = ps.setOcdDelay(ocdDelay);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage set OCD delay failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.success("OCD delay set [ %u ]", ocdDelay);
    }
    else
    {
        m_log.debug("OCD delay field missing so will be ignored");
    }

    if (iniMap.has(BR_SOCKET_INI_KEY))
    {
        u8            socketIndexNumber = atoi(iniMap[BR_SOCKET_INI_KEY].c_str());
        socketIndex_E brSocket          = decodeSocketIndex(socketIndexNumber);
        if (brSocket == socketIndex_E::UNASSIGNED)
        {
            m_log.warn("Brake resistor UNASSIGNED");
        }
        else
        {
            m_log.debug("Brake resistor socket field found with value [ %u ]", (u8)brSocket);
            result = ps.bindBrakeResistor(brSocket);
            if (result != PdsModule::error_E::OK)
                m_log.error("Power Stage bind brake resistor failed [ %s ]",
                            PdsModule::error2String(result));
            else
                m_log.success("Brake resistor bind to socket [ %u ]", (u8)brSocket);
        }
    }
    else
    {
        m_log.debug("Brake resistor socket field missing so will be ignored");
    }

    if (iniMap.has(BR_TRIG_V_INI_KEY))
    {
        u32 brTriggerVoltage = atoi(iniMap[BR_TRIG_V_INI_KEY].c_str());
        m_log.debug("Brake resistor trigger voltage field found with value [ %u ]",
                    brTriggerVoltage);
        result = ps.setBrakeResistorTriggerVoltage(brTriggerVoltage);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage set brake resistor trigger voltage failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("Brake resistor trigger voltage set [ %u ]", brTriggerVoltage);
    }
    else
    {
        m_log.debug("Brake resistor trigger voltage field missing so will be ignored");
    }
}

void PdsCli::setupIcCfg(IsolatedConv& ic, mINI::INIMap<std::string>& iniMap)
{
    m_log.debug("Setting up config for Isolated Converter [ %u ] module", ic.getSocketIndex());
    PdsModule::error_E result = PdsModule::error_E::OK;

    if (iniMap.has(TEMP_LIMIT_INI_KEY))
    {
        f32 temperatureLimit = atof(iniMap[TEMP_LIMIT_INI_KEY].c_str());
        m_log.debug("Temperature limit field found with value [ %.2f ]", temperatureLimit);
        result = ic.setTemperatureLimit(temperatureLimit);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter set temperature limit failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("New temperature limit set [ %.2f ]", temperatureLimit);
    }
    else
    {
        m_log.debug("Temperature limit field missing so will be ignored");
    }

    if (iniMap.has(OCD_LEVEL_INI_KEY))
    {
        u32 ocdLevel = atoi(iniMap[OCD_LEVEL_INI_KEY].c_str());
        m_log.debug("OCD level field found with value [ %u ]", ocdLevel);
        result = ic.setOcdLevel(ocdLevel);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter set OCD level failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("OCD level set [ %u ]", ocdLevel);
    }
    else
    {
        m_log.debug("OCD level field missing so will be ignored");
    }

    if (iniMap.has(OCD_DELAY_INI_KEY))
    {
        u32 ocdDelay = atoi(iniMap[OCD_DELAY_INI_KEY].c_str());
        m_log.debug("OCD delay field found with value [ %u ]", ocdDelay);
        result = ic.setOcdDelay(ocdDelay);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter set OCD delay failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("OCD delay set [ %u ]", ocdDelay);
    }
    else
    {
        m_log.debug("OCD delay field missing so will be ignored");
    }
}

void PdsCli::setupBrCfg(BrakeResistor& br, mINI::INIMap<std::string>& iniMap)
{
    m_log.debug("Setting up config for Brake Resistor [ %u ] module", br.getSocketIndex());
    PdsModule::error_E result = PdsModule::error_E::OK;

    if (iniMap.has(TEMP_LIMIT_INI_KEY))
    {
        f32 temperatureLimit = atof(iniMap[TEMP_LIMIT_INI_KEY].c_str());
        m_log.debug("Temperature limit field found with value [ %.2f ]", temperatureLimit);
        result = br.setTemperatureLimit(temperatureLimit);
        if (result != PdsModule::error_E::OK)
            m_log.error("Brake Resistor set temperature limit failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("New temperature limit set [ %.2f ]", temperatureLimit);
    }
    else
    {
        m_log.debug("Temperature limit field missing so will be ignored");
    }
}

static std::optional<moduleType_E> parseSubmoduleTypeStrimg(std::string_view typeString)
{
    if (typeString == Pds::moduleTypeToString(moduleType_E::ISOLATED_CONVERTER))
        return moduleType_E::ISOLATED_CONVERTER;

    else if (typeString == Pds::moduleTypeToString(moduleType_E::POWER_STAGE))
        return moduleType_E::POWER_STAGE;

    else if (typeString == Pds::moduleTypeToString(moduleType_E::BRAKE_RESISTOR))
        return moduleType_E::BRAKE_RESISTOR;

    return std::nullopt;
}

void PdsCli::pdsSetupConfig(const std::string& cfgPath)
{
    mINI::INIFile      pdsCfgFile(cfgPath);
    mINI::INIStructure pdsCfg;
    pdsCfgFile.read(pdsCfg);

    if (pdsCfg.has(CONTROL_BOARD_INI_SECTION))
        setupCtrlConfig(pdsCfg[CONTROL_BOARD_INI_SECTION]);
    else
        m_log.warn("No \"control_board\" section in ,cfg file.");

    for (u8 si = (u8)socketIndex_E::SOCKET_1; si <= (u8)socketIndex_E::SOCKET_6; si++)
    {
        m_log.debug("Checking \"Socket %u\" section", si);
        std::string sectionName = "Socket " + std::to_string(si);
        // Check if ini has Socket <si> section
        if (pdsCfg.has(sectionName.c_str()))
        {
            // Check if there is a "type" field in this section
            if (pdsCfg[sectionName.c_str()].has(TYPE_INI_KEY))
            {
                std::string_view moduleTypeString = pdsCfg[sectionName.c_str()][TYPE_INI_KEY];
                m_log.debug("%s type field :: %s", sectionName.c_str(), moduleTypeString.data());

                // Check if given type name is valid. If yes,set it up on physical device
                std::optional<moduleType_E> moduleType = parseSubmoduleTypeStrimg(moduleTypeString);
                if (moduleType.has_value())
                {
                    setupModuleCfg(
                        moduleType.value(), (socketIndex_E)si, pdsCfg[sectionName.c_str()]);
                }
                else
                {
                    m_log.warn("No \"%s\" field under \"%s\" so this section will be ignored",
                               TYPE_INI_KEY,
                               sectionName.c_str());
                }
            }
            else
            {
                m_log.warn("%s has no \"type\" field adn thus will be ignored...",
                           sectionName.c_str());
            }
        }
        else
        {
            m_log.warn("No \"%s\" section in .cfg file", sectionName.c_str());
        }
    }
}

void PdsCli::pdsReadConfig(const std::string& cfgPath)
{
    mINI::INIStructure readIni; /**< mINI structure for read data */
    // Control Board properties
    u32               shutDownTime = 0;
    u32               batLvl1      = 0;
    u32               batLvl2      = 0;
    Pds::modulesSet_S pdsModules   = m_pds.getModules();

    std::string configName = cfgPath;
    if (configName == "")
        configName = "pds_config.cfg";
    else if (std::filesystem::path(configName).extension() == "")
        configName += ".cfg";

    bool saveConfig = ui::getSaveConfigConfirmation(configName);

    m_pds.getShutdownTime(shutDownTime);
    m_pds.getBatteryVoltageLevels(batLvl1, batLvl2);

    readIni[CONTROL_BOARD_INI_SECTION][CAN_ID_INI_KEY]   = floatToString(m_canId, true);
    readIni[CONTROL_BOARD_INI_SECTION][CAN_BAUD_INI_KEY] = "TODO";
    readIni[CONTROL_BOARD_INI_SECTION][SHUTDOWN_TIME_INI_KEY] =
        floatToString(shutDownTime, true) + "\t\t; Shutdown time [ ms ]";
    readIni[CONTROL_BOARD_INI_SECTION][BATT_LVL_1_INI_KEY] =
        floatToString(batLvl1, true) + "\t\t; Battery monitor lvl 1 [ mV ]";
    readIni[CONTROL_BOARD_INI_SECTION][BATT_LVL_2_INI_KEY] =
        floatToString(batLvl2, true) + "\t\t; Battery monitor lvl 2 [ mV ]";
    fullModuleIni(m_pds, pdsModules.moduleTypeSocket1, readIni, socketIndex_E::SOCKET_1);
    fullModuleIni(m_pds, pdsModules.moduleTypeSocket2, readIni, socketIndex_E::SOCKET_2);
    fullModuleIni(m_pds, pdsModules.moduleTypeSocket3, readIni, socketIndex_E::SOCKET_3);
    fullModuleIni(m_pds, pdsModules.moduleTypeSocket4, readIni, socketIndex_E::SOCKET_4);
    fullModuleIni(m_pds, pdsModules.moduleTypeSocket5, readIni, socketIndex_E::SOCKET_5);
    fullModuleIni(m_pds, pdsModules.moduleTypeSocket6, readIni, socketIndex_E::SOCKET_6);

    mINI::INIFile configFile(configName);

    if (saveConfig)
    {
        configFile.write(readIni, true);
    }
}

void PdsCli::pdsStoreConfig(void)
{
    using err_E = mab::PdsModule::error_E;

    err_E result = m_pds.saveConfig();

    if (result != err_E::OK)
        m_log.error("PDS Configuration save error [ %u ] [ %s:%u ]", result, __FILE__, __LINE__);
    else
        m_log.success("PDS Configuration saved");
}

socketIndex_E PdsCli::decodeSocketIndex(u8 numericSocketIndex)
{
    if (numericSocketIndex < 7)
        return static_cast<socketIndex_E>(numericSocketIndex);
    else
        return socketIndex_E::UNASSIGNED;
}
