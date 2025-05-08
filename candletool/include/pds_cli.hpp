#pragma once

#include "CLI/CLI.hpp"
#include "pds_types.hpp"
/*#include "candletool.hpp"*/
#include "pds.hpp"

using namespace mab;
class PdsCli
{
  public:
    PdsCli() = delete;
    PdsCli(CLI::App& rootCli);
    ~PdsCli() = default;

    void parse(Pds* p_pds);

  private:
    Logger    m_log;
    CLI::App& m_rootCli;

    CLI::App* m_pdsCmd = nullptr;

    CLI::App* m_infoCmd = nullptr;

    // "can" commands node
    CLI::App*    m_canCmd         = nullptr;
    CLI::App*    m_canIdCmd       = nullptr;
    CLI::Option* m_canIdCmdOption = nullptr;

    CLI::App*    m_canBaudCmd       = nullptr;
    CLI::Option* m_canBaudCmdOption = nullptr;

    CLI::App*    m_canTimeoutCmd       = nullptr;
    CLI::Option* m_canTimeoutCmdOption = nullptr;

    // "clear" commands node
    CLI::App* m_clearCmd    = nullptr;
    CLI::App* m_clearAllCmd = nullptr;

    // Deprecated can comands ( But still usable )
    CLI::App* m_setCanIdCmd   = nullptr;
    CLI::App* m_setCanBaudCmd = nullptr;

    CLI::App* m_setBatteryLevelCmd = nullptr;
    CLI::App* m_setShutdownTimeCmd = nullptr;

    CLI::App* m_ctrlSetBrCmd        = nullptr;
    CLI::App* m_ctrlGetBrCmd        = nullptr;
    CLI::App* m_ctrlSetBrTriggerCmd = nullptr;
    CLI::App* m_ctrlGetBrTriggerCmd = nullptr;

    CLI::App* m_configSetupCmd      = nullptr;
    CLI::App* m_interactiveSetupCmd = nullptr;
    CLI::App* m_configReadCmd       = nullptr;
    CLI::App* m_configSaveCmd       = nullptr;

    CLI::App* m_disableCmd = nullptr;

    CLI::App* m_powerStageCmd        = nullptr;
    CLI::App* m_brakeResistorCmd     = nullptr;
    CLI::App* m_isolatedConverterCmd = nullptr;

    // Power stage commands
    CLI::App* m_psInfoCmd         = nullptr;
    CLI::App* m_psEnableCmd       = nullptr;
    CLI::App* m_psDisableCmd      = nullptr;
    CLI::App* m_psSetOvcLevelCmd  = nullptr;
    CLI::App* m_psGetOvcLevelCmd  = nullptr;
    CLI::App* m_psSetOvcDelayCmd  = nullptr;
    CLI::App* m_psGetOvcDelayCmd  = nullptr;
    CLI::App* m_psSetTempLimitCmd = nullptr;
    CLI::App* m_psGetTempLimitCmd = nullptr;
    CLI::App* m_psSetBrCmd        = nullptr;
    CLI::App* m_psGetBrCmd        = nullptr;
    CLI::App* m_psSetBrTriggerCmd = nullptr;
    CLI::App* m_psGetBrTriggerCmd = nullptr;
    CLI::App* m_psSetAutoStartCmd = nullptr;
    CLI::App* m_psGetAutoStartCmd = nullptr;
    CLI::App* m_psClearCmd        = nullptr;

    // Brake resistor commands
    CLI::App* m_brInfoCmd         = nullptr;
    CLI::App* m_brSetTempLimitCmd = nullptr;
    CLI::App* m_brGetTempLimitCmd = nullptr;
    CLI::App* m_brClearCmd        = nullptr;

    // Isolated converter commands
    CLI::App* m_icInfoCmd         = nullptr;
    CLI::App* m_icEnableCmd       = nullptr;
    CLI::App* m_icDisableCmd      = nullptr;
    CLI::App* m_icSetOvcLevelCmd  = nullptr;
    CLI::App* m_icGetOvcLevelCmd  = nullptr;
    CLI::App* m_icSetOvcDelayCmd  = nullptr;
    CLI::App* m_icGetOvcDelayCmd  = nullptr;
    CLI::App* m_icSetTempLimitCmd = nullptr;
    CLI::App* m_icGetTempLimitCmd = nullptr;
    CLI::App* m_icClearCmd        = nullptr;

    // Properties
    u16         m_canId         = 0u;     // PDS CAN ID
    u16         m_newCanId      = 0u;     // PDS CAN ID
    std::string m_canBaudrate   = "";     // PDS CAN baudrate
    u32         m_batteryLevel1 = 0u;     // Battery level 1 in mV
    u32         m_batteryLevel2 = 0u;     // Battery level 2 in mV
    u32         m_shutdownTime  = 0u;     // Shutdown time in ms
    u32         m_ovcLevel      = 0u;     // Overcurrent detection level in mA
    u32         m_ovcDelay      = 0u;     // Overcurrent detection delay in ms
    f32         m_tempLimit     = 0.0f;   // Temperature limit in degrees Celsius
    u32         m_brSocket      = 0u;     // Brake resistor socket index
    u32         m_brTrigger     = 0u;     // Brake resistor trigger voltage in mV
    bool        m_autoStart     = false;  // Power stage auto start

    // Brake resistor commands

    std::string m_cfgFilePath           = "";
    u8          m_submoduleSocketNumber = 0;

    Pds* mp_pds;

    socketIndex_E decodeSocketIndex(u8 numericSocketIndex);

    // "clear" node subcommands and options adding
    void clearCliNodeInit(void);
    void clearCliNodeParse(void);

    void powerStageCmdParse(void);
    void brakeResistorCmdParse(void);
    void isolatedConverterCmdParse(void);

    void pdsSetupInfo(void);
    void pdsSetupConfig(const std::string& cfgPath);
    void setupCtrlConfig(mINI::INIMap<std::string>& iniMap);
    void setupModuleCfg(moduleType_E type, socketIndex_E si, mINI::INIMap<std::string>& iniMap);
    void setupPsCfg(PowerStage& ps, mINI::INIMap<std::string>& iniMap);
    void setupIcCfg(IsolatedConv& ic, mINI::INIMap<std::string>& iniMap);
    void setupBrCfg(BrakeResistor& br, mINI::INIMap<std::string>& iniMap);
    void pdsStoreConfig(void);
    void pdsReadConfig(const std::string& cfgPath);
};
