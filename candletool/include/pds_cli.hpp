#pragma once

#include "CLI/CLI.hpp"
#include "pds_types.hpp"
#include "candletool.hpp"

using namespace mab;
class PdsCli
{
  public:
    PdsCli() = delete;
    PdsCli(CLI::App& rootCli, CandleTool& candletool);
    ~PdsCli() = default;

    void parse(void);

  private:
    Logger    m_log;
    CLI::App& m_rootCli;

    CLI::App* m_pds = nullptr;

    CLI::App* m_infoCmd        = nullptr;
    CLI::App* m_configSetupCmd = nullptr;
    CLI::App* m_configReadCmd  = nullptr;
    CLI::App* m_configSaveCmd  = nullptr;

    CLI::App* m_powerStageCmd        = nullptr;
    CLI::App* m_brakeResistorCmd     = nullptr;
    CLI::App* m_isolatedConverterCmd = nullptr;

    // Power stage commands
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

    // Brake resistor commands

    u16         m_canId                 = 0;
    std::string m_cfgFilePath           = "";
    u8          m_submoduleSocketNumber = 0;

    CandleTool& m_candleTool;

    socketIndex_E decodeSocketIndex(u8 numericSocketIndex);

    void powerStageCmdParse(void);
    void brakeResistorCmdParse(void);
    void isolatedConverterCmdParse(void);
};