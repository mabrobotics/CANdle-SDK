#pragma once

#include "CLI/CLI.hpp"
#include "logger.hpp"
#include "MDCO.hpp"

using namespace mab;

class MdcoCli
{
  public:
    MdcoCli() = delete;
    MdcoCli(CLI::App& rootCli, const std::shared_ptr<CandleBuilder> candleBuilder);
    ~MdcoCli() = default;

    /// @brief Parse the command line arguments for the MDCO CLI
    /// @param baud The baud rate for the CANdle device
    void parse(mab::CANdleDatarate_E baud);

  private:
    Logger                               m_log;
    CLI::App&                            m_rootCli;
    const std::shared_ptr<CandleBuilder> m_candleBuilder;

    //  principal subcommands
    CLI::App* mdco         = nullptr;
    CLI::App* blink        = nullptr;
    CLI::App* clear        = nullptr;
    CLI::App* can          = nullptr;
    CLI::App* encoder      = nullptr;
    CLI::App* heartbeat    = nullptr;
    CLI::App* nmt          = nullptr;
    CLI::App* pdoTest      = nullptr;
    CLI::App* ping         = nullptr;
    CLI::App* registr      = nullptr;
    CLI::App* reset        = nullptr;
    CLI::App* setup        = nullptr;
    CLI::App* Sync         = nullptr;
    CLI::App* SDOsegmented = nullptr;
    CLI::App* test         = nullptr;
    CLI::App* timeStamp    = nullptr;
    CLI::App* eds          = nullptr;

    // eds subcommands
    CLI::App* edsLoad             = nullptr;
    CLI::App* edsUnload           = nullptr;
    CLI::App* edsDisplay          = nullptr;
    CLI::App* edsGen              = nullptr;
    CLI::App* edsGenMd            = nullptr;
    CLI::App* edsGenHtml          = nullptr;
    CLI::App* edsGencpp           = nullptr;
    CLI::App* edsGet              = nullptr;
    CLI::App* edsFind             = nullptr;
    CLI::App* edsModify           = nullptr;
    CLI::App* edsModifyAdd        = nullptr;
    CLI::App* edsModifyDel        = nullptr;
    CLI::App* edsModifyCorrection = nullptr;

    // nmt subcommands
    CLI::App* nmtRead               = nullptr;
    CLI::App* nmtOperational        = nullptr;
    CLI::App* nmtStop               = nullptr;
    CLI::App* nmtPreOperational     = nullptr;
    CLI::App* nmtResetNode          = nullptr;
    CLI::App* nmtResetCommunication = nullptr;

    // register subcommands
    CLI::App* regRead  = nullptr;
    CLI::App* regWrite = nullptr;

    // pdoTestco subcommands
    CLI::App* PdoSpeed    = nullptr;
    CLI::App* PdoPosition = nullptr;
    CLI::App* PdoCustom   = nullptr;

    // setupco subcommands
    CLI::App*    setupCalib       = nullptr;
    CLI::App*    setupCalibOut    = nullptr;
    CLI::App*    setupInfo        = nullptr;
    CLI::App*    setupupload      = nullptr;
    CLI::App*    setupdownload    = nullptr;
    CLI::Option* setupInfoAllFlag = nullptr;

    // SDOsegmentedco subcommands
    CLI::App* SDOsegmentedRead  = nullptr;
    CLI::App* SDOsegmentedWrite = nullptr;

    // testco subcommands
    CLI::App* testEncoder     = nullptr;
    CLI::App* testEncoderMain = nullptr;
    CLI::App* testEncoderOut  = nullptr;
    CLI::App* testLatency     = nullptr;
    CLI::App* testMove        = nullptr;
    CLI::App* testMoveAbs     = nullptr;
    CLI::App* testMoveRel     = nullptr;
    CLI::App* testMoveSpeed   = nullptr;
    CLI::App* testImpedance   = nullptr;
};