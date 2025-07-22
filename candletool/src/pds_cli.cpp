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
constexpr const char* AUTOSTART_INI_KEY  = "AUTOSTART";

PdsCli::PdsCli(CLI::App& rootCli) : m_rootCli(rootCli)
{
    m_log.m_tag   = "PDS";
    m_log.m_layer = Logger::ProgramLayer_E::TOP;

    m_pdsCmd = m_rootCli.add_subcommand("pds", "Tweak the PDS device");

    m_pdsCmd->add_option("<CAN_ID>", m_canId, "MAB FD-CAN protocol :: Target device ID")
        ->required();

    m_infoCmd = m_pdsCmd->add_subcommand("info", "Display debug info about PDS device");

    m_canCmd   = m_pdsCmd->add_subcommand("can", "Manage CAN parameters of the PDS device");
    m_canIdCmd = m_canCmd->add_subcommand("id", "Set the CAN ID of the PDS device");

    m_canIdCmdOption = m_canIdCmd->add_option("<NEW_CAN_ID>", m_newCanId, "New CAN ID")->required();

    m_canBaudCmd = m_canCmd->add_subcommand("baud", "Set the CAN baudrate of the PDS device");

    m_canBaudCmdOption =
        m_canBaudCmd->add_option("<NEW_CAN_BAUD>", m_canBaudrate, "New CAN Baudrate")->required();

    m_canTimeoutCmd = m_canCmd->add_subcommand(
        "timeout", "Set the CAN timeout of the PDS device ( Or read if no args )");

    m_canTimeoutCmdOption =
        m_canTimeoutCmd->add_option("<NEW_CAN_TIMEOUT>", m_canBaudrate, "New CAN Timeout");

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

    m_setCanBaudCmd =
        m_pdsCmd->add_subcommand("set_can_baud", "Assign new FD CAN Baudrate to the PDS device");

    m_setCanBaudCmd->add_option("<NEW_CAN_BAUD>", m_canBaudrate, "New CAN Baudrate")->required();

    m_setBatteryLevelCmd =
        m_pdsCmd->add_subcommand("set_battery_level", "Set the battery voltage levels");

    m_setBatteryLevelCmd->add_option("<level1>", m_batteryLevel1, "Battery voltage level 1")
        ->required();
    m_setBatteryLevelCmd->add_option("<level2>", m_batteryLevel2, "Battery voltage level 2")
        ->required();

    m_setShutdownTimeCmd = m_pdsCmd->add_subcommand("set_shutdown_time", "Set the shutdown time");

    m_setShutdownTimeCmd->add_option("<time>", m_shutdownTime, "Shutdown time in ms")->required();

    m_ctrlSetBrCmd =
        m_pdsCmd->add_subcommand("set_br", "Bind PS with the Brake Resistor at given Socket index");
    m_ctrlSetBrCmd->add_option("<socket_index>", m_brSocket, "Brake Resistor Socket index")
        ->required();

    m_ctrlGetBrCmd = m_pdsCmd->add_subcommand("get_br", "Get the Brake Resistor Socket index");

    m_ctrlSetBrTriggerCmd =
        m_pdsCmd->add_subcommand("set_br_trigger", "Set the Brake Resistor Trigger Voltage");

    m_ctrlSetBrTriggerCmd->add_option(
        "<br_trigger>", m_brTrigger, "Brake Resistor Trigger Voltage");

    m_ctrlGetBrTriggerCmd =
        m_pdsCmd->add_subcommand("get_br_trigger", "Get the Brake Resistor Trigger Voltage");

    m_ctrlGetBrTriggerCmd->add_option("<br_trigger>", m_brTrigger, "Brake Resistor Trigger Voltage")
        ->required();

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

    m_psSetBrCmd = m_powerStageCmd->add_subcommand(
        "set_br", "Bind PS with the Brake Resistor at given Socket index");

    m_psSetBrCmd->add_option("<socket_index>", m_brSocket, "Brake Resistor Socket index")
        ->required();

    m_psGetBrCmd = m_powerStageCmd->add_subcommand("get_br", "Get the Brake Resistor Socket index");

    m_psSetBrTriggerCmd =
        m_powerStageCmd->add_subcommand("set_br_trigger", "Set the Brake Resistor Trigger Voltage");

    m_psSetBrTriggerCmd->add_option("<br_trigger>", m_brTrigger, "Brake Resistor Trigger Voltage")
        ->required();

    m_psGetBrTriggerCmd =
        m_powerStageCmd->add_subcommand("get_br_trigger", "Get the Brake Resistor Trigger Voltage");

    m_psSetAutoStartCmd =
        m_powerStageCmd->add_subcommand("set_auto_start", "Set the Auto Start mode");

    m_psSetAutoStartCmd->add_option("<auto_start>", m_autoStart, "Auto Start mode")->required();

    m_psGetAutoStartCmd =
        m_powerStageCmd->add_subcommand("get_auto_start", "Get the Auto Start mode");

    m_psClearCmd = m_powerStageCmd->add_subcommand("clear", "Clear errors status bits");

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

    m_brClearCmd = m_brakeResistorCmd->add_subcommand("clear", "Clear errors status bits");

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

    m_icClearCmd = m_isolatedConverterCmd->add_subcommand("clear", "Clear errors status bits");
}

void PdsCli::clearCliNodeInit(void)
{
    m_clearCmd = m_pdsCmd->add_subcommand("clear", "Clear errors status bits");
    m_clearAllCmd =
        m_clearCmd->add_subcommand("all", "Clear errors status bits for all submodules");
}

void PdsCli::clearCliNodeParse(void)
{
    if (m_clearCmd->parsed())
    {
        PdsModule::error_E result = PdsModule::error_E::OK;
        result                    = mp_pds->clearErrors();
        if (result != PdsModule::error_E::OK)
        {
            m_log.error("Clearing Errors status bits failed [ %s ]",
                        PdsModule::error2String(result));
        }
        else
        {
            m_log.success("Clearing Errors status bits succeeded");
        }

        if (m_clearAllCmd->parsed())
        {
            m_log.info("Clearing error flags for all submodules...");
            m_log.warn("Not implemented yet");
        }
    }
}

static bool isCanIdValid(u16 canId)
{
    return ((canId >= CAN_MIN_ID) || (canId <= CAN_MAX_ID));
}

void PdsCli::parse(Pds* p_pds)
{
    PdsModule::error_E result = PdsModule::error_E::OK;

    if (p_pds == nullptr)
    {
        m_log.error("Internal error...");
        return;
    }

    mp_pds = p_pds;

    if (m_pdsCmd->parsed())
    {
        mp_pds->init(m_canId);

        m_log.info("PDS - Power Distribution System :: CAN ID [ %u ]", m_canId);

        if (m_infoCmd->parsed())
        {
            pdsSetupInfo();
        }

        else if (m_canCmd->parsed())
        {
            m_log.debug("CAN command parsed");
            if (m_canIdCmd->parsed())
            {
                m_log.debug("CAN ID command parsed");

                /*
                     For ID and Baud there is actually no sense to read them
                     so the conditions and handler might be removed in further refactor
                */
                if (!m_canIdCmdOption->empty())
                {
                    if (!isCanIdValid(m_newCanId))
                    {
                        m_log.error(
                            "Given CAN ID ( %u ) is invalid. Acceptable range is [ %u - %u ]",
                            m_newCanId,
                            CAN_MIN_ID,
                            CAN_MAX_ID);
                        return;
                    }
                    result = mp_pds->setCanId(m_newCanId);
                    if (result != PdsModule::error_E::OK)
                        m_log.error("Setting CAN ID failed [ %s ]",
                                    PdsModule::error2String(result));
                    else
                        m_log.success("New CAN ID set [ %u ]", m_newCanId);
                }
                else
                {
                    m_log.success("Current CAN ID [ %u ]", mp_pds->getCanId());
                }
            }
            else if (m_canBaudCmd->parsed())
            {
                m_log.debug("CAN Baudrate command parsed");
                if (!m_canBaudCmdOption->empty())
                {
                    std::optional<CANdleBaudrate_E> baudOpt = stringToBaudrate(m_canBaudrate);
                    if (!baudOpt.has_value())
                    {
                        m_log.error("Invalid baudrate: %s", m_canBaudrate);
                        return;
                    }
                    result = mp_pds->setCanBaudrate(baudOpt.value());
                    if (result != PdsModule::error_E::OK)
                        m_log.error("Setting CAN baudrate failed [ %s ]",
                                    PdsModule::error2String(result));
                    else
                        m_log.success("New CAN baudrate set [ %s ]", m_canBaudrate.c_str());
                }
                else
                {
                    m_log.success("Current CAN Baudrate [ %s ]", m_canBaudrate.c_str());
                }
            }
            else if (m_canTimeoutCmd->parsed())
            {
                m_log.debug("CAN Timeout command parsed");
                if (!m_canTimeoutCmdOption->empty())
                {
                    // TODO:
                    m_log.warn("Setting CAN timeout is not implemented yet");
                }
                else
                {
                    m_log.warn("Setting CAN timeout is not implemented yet");
                }
            }
        }

        else if (m_configSetupCmd->parsed())
        {
            m_log.debug("setup config command parsed");
            pdsSetupConfig(m_cfgFilePath);
        }

        else if (m_interactiveSetupCmd->parsed())
        {
            m_log.warn("This command is under development. Please contact support");
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
            result = mp_pds->setCanId(m_newCanId);
            if (result != PdsModule::error_E::OK)
                m_log.error("Setting CAN ID failed [ %s ]", PdsModule::error2String(result));
            else
                m_log.success("New CAN ID set [ %u ]", m_newCanId);
        }

        else if (m_setCanBaudCmd->parsed())
        {
            std::optional<CANdleBaudrate_E> baudOpt = stringToBaudrate(m_canBaudrate);
            if (!baudOpt.has_value())
            {
                m_log.error("Invalid baudrate: %s", m_canBaudrate);
                return;
            }

            result = mp_pds->setCanBaudrate(baudOpt.value());
            if (result != PdsModule::error_E::OK)
                m_log.error("Setting CAN baudrate failed [ %s ]", PdsModule::error2String(result));
            else
                m_log.success("New CAN baudrate set [ %s ]", m_canBaudrate.c_str());
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
            result = mp_pds->setBatteryVoltageLevels(m_batteryLevel1, m_batteryLevel2);
            if (result != PdsModule::error_E::OK)
                m_log.error("Battery levels setting failed [ %s ]",
                            PdsModule::error2String(result));
            else
                m_log.success("Battery levels set [ %u, %u ]", m_batteryLevel1, m_batteryLevel2);
        }

        else if (m_setShutdownTimeCmd->parsed())
        {
            result = mp_pds->setShutdownTime(m_shutdownTime);
            if (result != PdsModule::error_E::OK)
                m_log.error("Shutdown time setting failed [ %s ]", PdsModule::error2String(result));
            else
                m_log.success("Shutdown time set [ %u ]", m_shutdownTime);
        }

        else if (m_ctrlSetBrCmd->parsed())
        {
            // Notice that the m_brSocket is a numeric value, and brSocket is a enum value
            socketIndex_E brSocket = decodeSocketIndex(m_brSocket);
            if (!mp_pds->verifyModuleSocket(moduleType_E::BRAKE_RESISTOR, brSocket))
            {
                if (m_brSocket == 0)
                {
                    m_log.warn("Unbinding Brake Resistor from the Control Board");
                }
                else
                {
                    m_log.error("Invalid socket number for Brake Resistor submodule");
                    return;
                }
            }

            result = mp_pds->bindBrakeResistor(brSocket);

            if (result != PdsModule::error_E::OK)
                m_log.error("Binding Brake Resistor failed [ %s ]",
                            PdsModule::error2String(result));
            else
                m_log.success("Brake Resistor bound to socket [ %u ]", m_brSocket);
        }

        else if (m_ctrlGetBrCmd->parsed())
        {
            socketIndex_E brSocket = socketIndex_E::UNASSIGNED;
            result                 = mp_pds->getBindBrakeResistor(brSocket);
            if (result != PdsModule::error_E::OK)
                m_log.error("Getting Brake Resistor failed [ %s ]",
                            PdsModule::error2String(result));
            else
                m_log.success("Brake Resistor bound to socket [ %u ]", (u8)brSocket);
        }

        else if (m_ctrlSetBrTriggerCmd->parsed())
        {
            result = mp_pds->setBrakeResistorTriggerVoltage(m_brTrigger);
            if (result != PdsModule::error_E::OK)
                m_log.error("Setting Brake Resistor trigger voltage failed [ %s ]",
                            PdsModule::error2String(result));
            else
                m_log.success("Brake Resistor trigger voltage set [ %u ]", m_brTrigger);
        }

        else if (m_ctrlGetBrTriggerCmd->parsed())
        {
            u32 brTrigger = 0;
            result        = mp_pds->getBrakeResistorTriggerVoltage(brTrigger);
            if (result != PdsModule::error_E::OK)
                m_log.error("Getting Brake Resistor trigger voltage failed [ %s ]",
                            PdsModule::error2String(result));
            else
                m_log.success("Brake Resistor trigger voltage [ %u ]", brTrigger);
        }

        else if (m_disableCmd->parsed())
        {
            mp_pds->shutdown();
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

    if (!mp_pds->verifyModuleSocket(moduleType_E::POWER_STAGE, socketIndex))
    {
        m_log.error("Invalid socket number for Power Stage submodule");
        return;
    }

    auto ps = mp_pds->attachPowerStage(socketIndex);

    if (ps == nullptr)
    {
        m_log.error("Power Stage submodule is not available");
        return;
    }

    m_log.info("Power Stage submodule :: Socket index [ %u ]", m_submoduleSocketNumber);

    if (m_psInfoCmd->parsed())
    {
        // TODO: Create a separate function for submodules info
        powerStageStatus_S   psStatus         = {0};
        u32                  busVoltage       = 0;
        s32                  current          = 0;
        u32                  ovcLevel         = 0;
        u32                  ovcDelay         = 0;
        f32                  temperature      = 0.0f;
        f32                  temperatureLimit = 0.0f;
        socketIndex_E        brSocket         = socketIndex_E::UNASSIGNED;
        u32                  brTrigger        = 0;
        bool                 autoStart        = false;
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

        result = ps->getAutostart(autoStart);
        if (result != PdsModule::error_E::OK)
            m_log.error("Reading Power Stage autostart filed failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.info("\t* Autostart [ %s ]", autoStart ? "ENABLED" : "DISABLED");
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
        // Notice that the m_brSocket is a numeric value, and brSocket is a enum value
        socketIndex_E brSocket = decodeSocketIndex(m_brSocket);
        if (!mp_pds->verifyModuleSocket(moduleType_E::BRAKE_RESISTOR, brSocket))
        {
            if (m_brSocket == 0)
            {
                m_log.warn("Unbinding Brake Resistor from the Power Stage");
            }
            else
            {
                m_log.error("Invalid socket number for Brake Resistor submodule");
                return;
            }
        }

        result = ps->bindBrakeResistor(decodeSocketIndex(m_brSocket));

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
    else if (m_psSetAutoStartCmd->parsed())
    {
        result = ps->setAutostart(m_autoStart);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage set autostart failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.success("Autostart set [ %s ]", m_autoStart ? "ENABLED" : "DISABLED");
    }
    else if (m_psGetAutoStartCmd->parsed())
    {
        bool autoStart = false;
        result         = ps->getAutostart(autoStart);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get autostart failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.info("Autostart [ %s ]", autoStart ? "ENABLED" : "DISABLED");
    }
    else if (m_psClearCmd->parsed())
    {
        powerStageStatus_S psStatus = {0};

        psStatus.OVER_CURRENT     = true;
        psStatus.OVER_TEMPERATURE = true;

        result = ps->clearStatus(psStatus);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage clear errors failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.success("Power Stage errors cleared");
    }

    else
        m_log.error("PS subcommand is missing");
}

void PdsCli::brakeResistorCmdParse(void)
{
    socketIndex_E      socketIndex = decodeSocketIndex(m_submoduleSocketNumber);
    PdsModule::error_E result      = PdsModule::error_E::OK;

    if (!mp_pds->verifyModuleSocket(moduleType_E::BRAKE_RESISTOR, socketIndex))
    {
        m_log.error("Invalid socket number for Brake Resistor submodule");
        return;
    }

    auto br = mp_pds->attachBrakeResistor(socketIndex);

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

    else if (m_brClearCmd->parsed())
    {
        brakeResistorStatus_S brStatus = {0};
        brStatus.OVER_TEMPERATURE      = true;

        result = br->clearStatus(brStatus);
        if (result != PdsModule::error_E::OK)
            m_log.error("Brake Resistor clear errors failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("Brake Resistor errors cleared");
    }

    else
        m_log.error("BR subcommand is missing");
}

void PdsCli::isolatedConverterCmdParse(void)
{
    socketIndex_E      socketIndex = decodeSocketIndex(m_submoduleSocketNumber);
    PdsModule::error_E result      = PdsModule::error_E::OK;

    if (!mp_pds->verifyModuleSocket(moduleType_E::ISOLATED_CONVERTER, socketIndex))
    {
        m_log.error("Invalid socket number for Isolated Converter submodule");
        return;
    }

    auto ic = mp_pds->attachIsolatedConverter(socketIndex);

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

    else if (m_icClearCmd->parsed())
    {
        isolatedConverterStatus_S icStatus = {0};
        icStatus.OVER_TEMPERATURE          = true;
        icStatus.OVER_CURRENT              = true;

        result = ic->clearStatus(icStatus);
        if (result != PdsModule::error_E::OK)
            m_log.error("Isolated Converter clear errors failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("Isolated Converter errors cleared");
    }

    else
        m_log.error("IC subcommand is missing");
}

void PdsCli::pdsSetupInfo()
{
    mab::pdsFwMetadata_S pdsFwMetadata = {0};
    PdsModule::error_E   result        = mp_pds->getFwMetadata(pdsFwMetadata);
    if (result != PdsModule::error_E::OK)
    {
        m_log.error("PDS get firmware metadata failed [ %s ]", PdsModule::error2String(result));
        return;
    }

    mab::Pds::modulesSet_S pdsModules = mp_pds->getModules();

    mab::controlBoardStatus_S pdsStatus     = {0};
    u32                       pdsBusVoltage = 0;
    u32                       shutdownTime  = 0;
    u32                       batteryLvl1   = 0;
    u32                       batteryLvl2   = 0;
    socketIndex_E             brSocket      = socketIndex_E::UNASSIGNED;
    u32                       brTrigger     = 0;

    result = mp_pds->getStatus(pdsStatus);
    if (result != PdsModule::error_E::OK)
        m_log.error("PDS get status failed [ %s ]", PdsModule::error2String(result));

    result = mp_pds->getBusVoltage(pdsBusVoltage);
    if (result != PdsModule::error_E::OK)
        m_log.error("PDS get bus voltage failed [ %s ]", PdsModule::error2String(result));

    result = mp_pds->getShutdownTime(shutdownTime);
    if (result != PdsModule::error_E::OK)
        m_log.error("PDS get shutdown time failed [ %s ]", PdsModule::error2String(result));

    result = mp_pds->getBatteryVoltageLevels(batteryLvl1, batteryLvl2);
    if (result != PdsModule::error_E::OK)
        m_log.error("PDS get battery levels failed [ %s ]", PdsModule::error2String(result));

    result = mp_pds->getBindBrakeResistor(brSocket);
    if (result != PdsModule::error_E::OK)
        m_log.error("Power Stage get brake resistor failed [ %s ]",
                    PdsModule::error2String(result));

    if (brSocket != socketIndex_E::UNASSIGNED)
    {
        result = mp_pds->getBrakeResistorTriggerVoltage(brTrigger);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get brake resistor trigger voltage failed [ %s ]",
                        PdsModule::error2String(result));
    }

    m_log.info("Power Distribution Module");

    m_log.info("Firmware version: %u.%u.%u-%s",
               pdsFwMetadata.version.s.major,
               pdsFwMetadata.version.s.minor,
               pdsFwMetadata.version.s.revision,
               pdsFwMetadata.gitHash);

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
    // m_log.info("\t* OVER_CURRENT      [ %s ]", pdsStatus.OVER_CURRENT ? "YES" : "NO");
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
    m_log.info("\t* shutdown time: [ %u mS ] ", shutdownTime);
    m_log.info("\t* Battery level 1: [ %0.2f V ]", batteryLvl1 / 1000.0f);
    m_log.info("\t* Battery level 2: [ %0.2f V ]", batteryLvl2 / 1000.0f);

    if (brSocket == socketIndex_E::UNASSIGNED)
        m_log.info("\t* Brake resistor is not set");
    else
    {
        m_log.info("\t* Brake resistor socket [ %u ]", (u8)brSocket);
        m_log.info("\t* Brake resistor trigger voltage [ %0.2f V ]", brTrigger / 1000.0f);
    }

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
    bool          autoStart        = false;

    ps.getBindBrakeResistor(brSocket);
    ps.getBrakeResistorTriggerVoltage(brTriggerVoltage);
    ps.getOcdLevel(ocdLevel);
    ps.getOcdDelay(ocdDelay);
    ps.getTemperatureLimit(temperatureLimit);
    ps.getAutostart(autoStart);

    rIni[sectionName][TYPE_INI_KEY] = PdsModule::mType2Str(moduleType_E::POWER_STAGE);
    rIni[sectionName][TEMP_LIMIT_INI_KEY] =
        prettyFloatToString(temperatureLimit) + "\t; Temperature limit [ ^C ]";
    rIni[sectionName][OCD_LEVEL_INI_KEY] =
        prettyFloatToString(ocdLevel, true) + "\t\t; Over-current detection level [ mA ]";
    rIni[sectionName][OCD_DELAY_INI_KEY] =
        prettyFloatToString(ocdDelay, true) + "\t\t; Over-current detection delay [ ms ]";
    rIni[sectionName][BR_SOCKET_INI_KEY] =
        prettyFloatToString((uint8_t)brSocket, true) +
        "\t\t\t; Socket index where corresponding Brake Resistor is connected";
    rIni[sectionName][BR_TRIG_V_INI_KEY] =
        prettyFloatToString(brTriggerVoltage, true) + "\t; Brake resistor trigger voltage [ mV ]";
    rIni[sectionName][AUTOSTART_INI_KEY] =
        std::string(autoStart ? "ON" : "OFF") + "\t\t\t; Autostart [ ON | OFF ]";
}

// Fill Brake resistor Ini structure
static void fillBrIni(BrakeResistor& br, mINI::INIStructure& rIni, std::string sectionName)
{
    f32 temperatureLimit = 0.0f;

    br.getTemperatureLimit(temperatureLimit);

    rIni[sectionName][TYPE_INI_KEY] = PdsModule::mType2Str(moduleType_E::BRAKE_RESISTOR);
    rIni[sectionName][TEMP_LIMIT_INI_KEY] =
        prettyFloatToString(temperatureLimit) + "\t; Temperature limit [ ^C ]";
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
        prettyFloatToString(temperatureLimit) + "\t; Temperature limit [ ^C ]";
    rIni[sectionName][OCD_LEVEL_INI_KEY] =
        prettyFloatToString(ocdLevel, true) + "\t\t; Over-current detection level [ mA ]";
    rIni[sectionName][OCD_DELAY_INI_KEY] =
        prettyFloatToString(ocdDelay, true) + "\t\t; Over-current detection delay [ ms ]";
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

static std::optional<CANdleBaudrate_E> parseCanBaudIniString(std::string_view baudString)
{
    if (baudString == "1M")
        return CANdleBaudrate_E::CAN_BAUD_1M;
    else if (baudString == "2M")
        return CANdleBaudrate_E::CAN_BAUD_2M;
    else if (baudString == "5M")
        return CANdleBaudrate_E::CAN_BAUD_5M;
    else if (baudString == "8M")
        return CANdleBaudrate_E::CAN_BAUD_8M;

    return std::nullopt;
}

std::optional<bool> parseBooleanIniField(std::string_view input)
{
    // Convert to lowercase
    std::string lower(input);
    std::transform(
        lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });

    if (lower == "on")
        return true;
    if (lower == "off")
        return false;

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
        result = mp_pds->setCanId(canId);
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
        std::string_view                canBaudString = iniMap[CAN_BAUD_INI_KEY];
        std::optional<CANdleBaudrate_E> canBaud       = parseCanBaudIniString(canBaudString);
        if (canBaud.has_value())
        {
            result = mp_pds->setCanBaudrate(canBaud.value());
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
        result = mp_pds->setShutdownTime(shutdownTime);
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
        result = mp_pds->setBatteryVoltageLevels(battLvl1, battLvl2);
        if (result != PdsModule::error_E::OK)
            m_log.error("Battery levels setting failed [ %s ]", PdsModule::error2String(result));
        else
            m_log.success("Battery levels set [ %u, %u ]", battLvl1, battLvl2);
    }
    else
    {
        m_log.warn("Battery levels field missing so will be ignored");
    }

    // Brake resistor
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
            result = mp_pds->bindBrakeResistor(brSocket);
            if (result != PdsModule::error_E::OK)
                m_log.error("PDS bind brake resistor failed [ %s ]",
                            PdsModule::error2String(result));
            else
                m_log.success("Brake resistor bind to socket [ %u ]", (u8)brSocket);
        }
    }
    else
    {
        m_log.warn("Brake resistor socket field missing so will be ignored");
    }

    // Brake resistor trigger voltage
    if (iniMap.has(BR_TRIG_V_INI_KEY))
    {
        u32 brTriggerVoltage = atoi(iniMap[BR_TRIG_V_INI_KEY].c_str());
        m_log.debug("Brake resistor trigger voltage field found with value [ %u ]",
                    brTriggerVoltage);
        result = mp_pds->setBrakeResistorTriggerVoltage(brTriggerVoltage);
        if (result != PdsModule::error_E::OK)
            m_log.error("PDS set brake resistor trigger voltage failed [ %s ]",
                        PdsModule::error2String(result));
        else
            m_log.success("Brake resistor trigger voltage set [ %u ]", brTriggerVoltage);
    }
    else
    {
        m_log.warn("Brake resistor trigger voltage field missing so will be ignored");
    }
}

void PdsCli::setupModuleCfg(moduleType_E type, socketIndex_E si, mINI::INIMap<std::string>& iniMap)
{
    if (type == moduleType_E::POWER_STAGE)
    {
        auto powerStage = mp_pds->attachPowerStage(si);
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
        auto isolatedConverter = mp_pds->attachIsolatedConverter(si);
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
        auto brakeResistor = mp_pds->attachBrakeResistor(si);
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

    if (iniMap.has(AUTOSTART_INI_KEY))
    {
        m_log.debug("Autostart field found with value [ %s ]", iniMap[AUTOSTART_INI_KEY].c_str());
        std::optional<bool> autostart =
            parseBooleanIniField(std::string_view(iniMap[AUTOSTART_INI_KEY].c_str()));
        if (autostart.has_value())
        {
            m_log.debug("Autostart field found with value [ %s ]",
                        autostart.value() ? "ON" : "OFF");
            result = ps.setAutostart(autostart.value());
            if (result != PdsModule::error_E::OK)
                m_log.error("Power Stage set autostart failed [ %s ]",
                            PdsModule::error2String(result));
            else
                m_log.success("Power Stage autostart set [ %s ]", autostart.value() ? "ON" : "OFF");
        }
        else
        {
            m_log.error("Given Autostart [ %s ] is INVALID! Acceptable values are: ON, OFF",
                        iniMap[AUTOSTART_INI_KEY]);
        }
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

    m_log.info("Reading .cfg file [ %s ]", cfgPath.c_str());
    if (!pdsCfgFile.read(pdsCfg))
    {
        m_log.error("Failed to read .cfg file [ %s ]", cfgPath.c_str());
        return;
    }

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
    u32           shutDownTime = 0;
    u32           batLvl1      = 0;
    u32           batLvl2      = 0;
    socketIndex_E brSocket     = socketIndex_E::UNASSIGNED;
    u32           brTrigger    = 0;

    Pds::modulesSet_S pdsModules = mp_pds->getModules();

    std::string configName = cfgPath;
    if (configName == "")
        configName = "pds_config.cfg";
    else if (std::filesystem::path(configName).extension() == "")
        configName += ".cfg";

    bool saveConfig = ui::getSaveConfigConfirmation(configName);

    // TODO: Consider error handling here ?
    mp_pds->getShutdownTime(shutDownTime);
    mp_pds->getBatteryVoltageLevels(batLvl1, batLvl2);
    mp_pds->getBindBrakeResistor(brSocket);
    mp_pds->getBrakeResistorTriggerVoltage(brTrigger);

    readIni[CONTROL_BOARD_INI_SECTION][CAN_ID_INI_KEY] = prettyFloatToString(m_canId, true);
    readIni[CONTROL_BOARD_INI_SECTION][CAN_BAUD_INI_KEY] =
        baudrateToString(mp_pds->getCanBaudrate());
    readIni[CONTROL_BOARD_INI_SECTION][SHUTDOWN_TIME_INI_KEY] =
        prettyFloatToString(shutDownTime, true) + "\t\t; Shutdown time [ ms ]";
    readIni[CONTROL_BOARD_INI_SECTION][BATT_LVL_1_INI_KEY] =
        prettyFloatToString(batLvl1, true) + "\t\t; Battery monitor lvl 1 [ mV ]";
    readIni[CONTROL_BOARD_INI_SECTION][BATT_LVL_2_INI_KEY] =
        prettyFloatToString(batLvl2, true) + "\t\t; Battery monitor lvl 2 [ mV ]";
    readIni[CONTROL_BOARD_INI_SECTION][BR_SOCKET_INI_KEY] =
        prettyFloatToString((u8)brSocket, true) +
        "\t\t; Socket index where corresponding Brake Resistor is connected";
    readIni[CONTROL_BOARD_INI_SECTION][BR_TRIG_V_INI_KEY] =
        prettyFloatToString(brTrigger, true) + "\t\t; Brake resistor trigger voltage [ mV ]";

    fullModuleIni(*mp_pds, pdsModules.moduleTypeSocket1, readIni, socketIndex_E::SOCKET_1);
    fullModuleIni(*mp_pds, pdsModules.moduleTypeSocket2, readIni, socketIndex_E::SOCKET_2);
    fullModuleIni(*mp_pds, pdsModules.moduleTypeSocket3, readIni, socketIndex_E::SOCKET_3);
    fullModuleIni(*mp_pds, pdsModules.moduleTypeSocket4, readIni, socketIndex_E::SOCKET_4);
    fullModuleIni(*mp_pds, pdsModules.moduleTypeSocket5, readIni, socketIndex_E::SOCKET_5);
    fullModuleIni(*mp_pds, pdsModules.moduleTypeSocket6, readIni, socketIndex_E::SOCKET_6);

    mINI::INIFile configFile(configName);

    if (saveConfig)
    {
        configFile.write(readIni, true);
    }
}

void PdsCli::pdsStoreConfig(void)
{
    using err_E = mab::PdsModule::error_E;

    err_E result = mp_pds->saveConfig();

    if (result != err_E::OK)
        m_log.error("PDS Configuration save error [ %u ] [ %s:%u ]", result, __FILE__, __LINE__);
    else
        m_log.success("PDS Configuration saved");
}

socketIndex_E PdsCli::decodeSocketIndex(u8 numericSocketIndex)
{
    if (numericSocketIndex > (u8)mab::socketIndex_E::SOCKET_6)
        return socketIndex_E::UNASSIGNED;

    return static_cast<socketIndex_E>(numericSocketIndex);
}
