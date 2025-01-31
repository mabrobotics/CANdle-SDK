
#include "pds_cli.hpp"

PdsCli::PdsCli(CLI::App& rootCli, CandleTool& candletool)
    : m_rootCli(rootCli), m_candleTool(candletool)
{
    m_log.m_tag   = "CANDLETOOL";
    m_log.m_layer = Logger::ProgramLayer_E::TOP;

    m_pds = m_rootCli.add_subcommand("pds", "Tweak the PDS device");

    m_pds->add_option("<CAN_ID>", m_canId, "MAB FD-CAN protocol :: Target device ID")->required();

    m_infoCmd        = m_pds->add_subcommand("info", "Display debug info about PDS device");
    m_configSetupCmd = m_pds->add_subcommand("setup", "Configure PDS device with the .cfg file");

    m_configSetupCmd->add_option("<config_file>", m_cfgFilePath, "PDS configuration .cfg file.")
        ->required();

    m_configReadCmd =
        m_pds->add_subcommand("read_cfg", "Read device configuration and save to file");

    m_configReadCmd->add_option("<config_file>", m_cfgFilePath, "PDS configuration .cfg file.")
        ->required();

    m_configSaveCmd = m_pds->add_subcommand("save", "Store current configuration in device memory");

    m_powerStageCmd = m_pds->add_subcommand("ps", "Manage the Power Stage submodule");
    m_powerStageCmd
        ->add_option("<socket_index>", m_submoduleSocketNumber, "Submodule socket number")
        ->required();

    // m_brakeResistorCmd = m_pds->add_subcommand("ps", "Manage the Brake Resistor submodule");
    // m_brakeResistorCmd
    //     ->add_option("<socket_index>", m_submoduleSocketNumber, "Submodule socket number")
    //     ->required();
    //
    // m_isolatedConverterCmd = m_pds->add_subcommand("ps", "Manage the Isolated Converter submodule");
    // m_isolatedConverterCmd
    //     ->add_option("<socket_index>", m_submoduleSocketNumber, "Submodule socket number")
    //     ->required();
}

void PdsCli::parse(void)
{
    if (m_pds->parsed())
    {
        if (m_infoCmd->parsed())
        {
            m_candleTool.pdsSetupInfo(m_canId);
        }

        if (m_configSetupCmd->parsed())
        {
            m_candleTool.pdsSetupConfig(m_canId, m_cfgFilePath);
        }

        if (m_configReadCmd->parsed())
        {
            m_candleTool.pdsReadConfig(m_canId, m_cfgFilePath);
        }

        if (m_configSaveCmd->parsed())
        {
            m_candleTool.pdsStoreConfig(m_canId);
        }

        if (m_powerStageCmd->parsed())
        {
            // TODO: Separate power stage subcommands / options parsing method
        }
    }
}

socketIndex_E PdsCli::decodeSocketIndex(u8 numericSocketIndex)
{
    // m_log.debug("Checking if given socket index [ %u ] is Valid")
    // TODO:
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
