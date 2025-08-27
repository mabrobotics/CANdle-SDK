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
    void parse(mab::CANdleBaudrate_E baud);

  private:
    Logger                               m_log;
    CLI::App&                            m_rootCli;
    const std::shared_ptr<CandleBuilder> m_candleBuilder;

    //  principal subcommands
    CLI::App* mdco           = nullptr;
    CLI::App* blinkco        = nullptr;
    CLI::App* clearco        = nullptr;
    CLI::App* configco       = nullptr;
    CLI::App* encoderco      = nullptr;
    CLI::App* heartbeatco    = nullptr;
    CLI::App* nmt            = nullptr;
    CLI::App* pdoTestco      = nullptr;
    CLI::App* pingco         = nullptr;
    CLI::App* registrco      = nullptr;
    CLI::App* resetco        = nullptr;
    CLI::App* setupco        = nullptr;
    CLI::App* Sync           = nullptr;
    CLI::App* SDOsegmentedco = nullptr;
    CLI::App* testco         = nullptr;
    CLI::App* timeStamp      = nullptr;
    CLI::App* eds            = nullptr;

    // configco subcommands
    CLI::App* configBandco = nullptr;
    CLI::App* configCanco  = nullptr;
    CLI::App* configSaveco = nullptr;
    CLI::App* configZeroco = nullptr;

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
    CLI::App* regReadco  = nullptr;
    CLI::App* regWriteco = nullptr;

    // pdoTestco subcommands
    CLI::App* PdoSpeedco    = nullptr;
    CLI::App* PdoPositionco = nullptr;
    CLI::App* PdoCustomco   = nullptr;

    // setupco subcommands
    CLI::App*    setupCalibco       = nullptr;
    CLI::App*    setupCalibOutco    = nullptr;
    CLI::App*    setupInfoco        = nullptr;
    CLI::App*    setupMotorco       = nullptr;
    CLI::App*    setupReadCfgco     = nullptr;
    CLI::Option* setupInfoAllFlagco = nullptr;

    // SDOsegmentedco subcommands
    CLI::App* SDOsegmentedReadco  = nullptr;
    CLI::App* SDOsegmentedWriteco = nullptr;

    // testco subcommands
    CLI::App* testEncoderco     = nullptr;
    CLI::App* testEncoderMainco = nullptr;
    CLI::App* testEncoderOutco  = nullptr;
    CLI::App* testLatencyco     = nullptr;
    CLI::App* testMoveco        = nullptr;
    CLI::App* testMoveAbsco     = nullptr;
    CLI::App* testMoveRelco     = nullptr;
    CLI::App* testMoveSpeedco   = nullptr;
    CLI::App* testImpedanceco   = nullptr;
};