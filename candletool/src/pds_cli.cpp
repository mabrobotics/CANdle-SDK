#include "pds_cli.hpp"
#include "configHelpers.hpp"

PdsCli::PdsCli(CLI::App& rootCli, mab::Candle& candle) : m_rootCli(rootCli), m_candle(candle)
{
    m_log.m_tag   = "CANDLETOOL";
    m_log.m_layer = Logger::ProgramLayer_E::TOP;

    m_pdsCmd = m_rootCli.add_subcommand("pds", "Tweak the PDS device");

    m_pdsCmd->add_option("<CAN_ID>", m_canId, "MAB FD-CAN protocol :: Target device ID")
        ->required();

    m_infoCmd = m_pdsCmd->add_subcommand("info", "Display debug info about PDS device");
    m_configSetupCmd =
        m_pdsCmd->add_subcommand("setup_cfg", "Configure PDS device with the .cfg file");

    m_configSetupCmd->add_option("<config_file>", m_cfgFilePath, "PDS configuration .cfg file.")
        ->required();

    m_configReadCmd =
        m_pdsCmd->add_subcommand("read_cfg", "Read device configuration and save to file");

    m_configReadCmd->add_option("<config_file>", m_cfgFilePath, "PDS configuration .cfg file.")
        ->required();

    m_configSaveCmd =
        m_pdsCmd->add_subcommand("save", "Store current configuration in device memory");

    // POWER STAGE commands set
    m_powerStageCmd = m_pdsCmd->add_subcommand("ps", "Manage the Power Stage submodule");

    m_psInfoCmd = m_powerStageCmd->add_subcommand("info", "Display debug info about Power Stage");

    m_powerStageCmd
        ->add_option("<socket_index>", m_submoduleSocketNumber, "Submodule socket number")
        ->required();

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

void PdsCli::parse(void)
{
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
            pdsSetupConfig(m_cfgFilePath);
        }

        else if (m_configReadCmd->parsed())
        {
            pdsReadConfig(m_cfgFilePath);
        }

        else if (m_configSaveCmd->parsed())
        {
            pdsStoreConfig();
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

// Fill Isolated Converter Ini structure
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

void PdsCli::pdsSetupConfig(const std::string& cfgPath)
{
    using err_E = mab::PdsModule::error_E;

    mINI::INIFile      pdsCfgFile(cfgPath);
    mINI::INIStructure pdsCfg;
    pdsCfgFile.read(pdsCfg);

    u32 shutdownTime = atoi(pdsCfg["Control board"]["shutdown time"].c_str());
    u32 battLvl1     = atoi(pdsCfg["Control board"]["battery level 1"].c_str());
    u32 battLvl2     = atoi(pdsCfg["Control board"]["battery level 2"].c_str());

    err_E result = m_pds.setShutdownTime(shutdownTime);
    if (result != err_E::OK)
        m_log.error("PDS Config error [ %u ] [ %s:%u ]", result, __FILE__, __LINE__);

    result = m_pds.setBatteryVoltageLevels(battLvl1, battLvl2);
    if (result != err_E::OK)
        m_log.error("PDS Config error [ %u ] [ %s:%u ]", result, __FILE__, __LINE__);
}

void PdsCli::pdsReadConfig(const std::string& cfgPath)
{
    mINI::INIStructure readIni; /**< mINI structure for read data */
    u32                shutDownTime = 0;
    u32                batLvl1      = 0;
    u32                batLvl2      = 0;
    Pds::modulesSet_S  pdsModules   = m_pds.getModules();

    std::string configName = cfgPath;
    if (std::filesystem::path(configName).extension() == "")
        configName += ".cfg";

    m_pds.getShutdownTime(shutDownTime);
    m_pds.getBatteryVoltageLevels(batLvl1, batLvl2);

    readIni["Control board"]["CAN ID"]          = floatToString(m_canId);
    readIni["Control board"]["CAN BAUD"]        = "";
    readIni["Control board"]["shutdown time"]   = floatToString(shutDownTime);
    readIni["Control board"]["battery level 1"] = floatToString(batLvl1);
    readIni["Control board"]["battery level 2"] = floatToString(batLvl2);
    fullModuleIni(m_pds, pdsModules.moduleTypeSocket1, readIni, socketIndex_E::SOCKET_1);
    fullModuleIni(m_pds, pdsModules.moduleTypeSocket2, readIni, socketIndex_E::SOCKET_2);
    fullModuleIni(m_pds, pdsModules.moduleTypeSocket3, readIni, socketIndex_E::SOCKET_3);
    fullModuleIni(m_pds, pdsModules.moduleTypeSocket4, readIni, socketIndex_E::SOCKET_4);
    fullModuleIni(m_pds, pdsModules.moduleTypeSocket5, readIni, socketIndex_E::SOCKET_5);
    fullModuleIni(m_pds, pdsModules.moduleTypeSocket6, readIni, socketIndex_E::SOCKET_6);

    mINI::INIFile configFile(configName);
    configFile.write(readIni);
}

void PdsCli::pdsStoreConfig(void)
{
    using err_E = mab::PdsModule::error_E;

    err_E result = m_pds.saveConfig();

    if (result != err_E::OK)
        m_log.error("PDS Configuration save error [ %u ] [ %s:%u ]", result, __FILE__, __LINE__);
}

socketIndex_E PdsCli::decodeSocketIndex(u8 numericSocketIndex)
{
    if (numericSocketIndex < 7)
        return static_cast<socketIndex_E>(numericSocketIndex);
    else
        return socketIndex_E::UNASSIGNED;
}
