#pragma once

#include "CLI/CLI.hpp"
#include "logger.hpp"
#include "MDCO.hpp"

using namespace mab;

class MdcoCli
{
  public:
    MdcoCli() = delete;
    MdcoCli(CLI::App& rootCli);
    ~MdcoCli() = default;

    void parse(mab::CANdleBaudrate_E baud);
    bool mdcoParse();

  private:
    Logger    m_log;
    CLI::App& m_rootCli;

    // Sous-commandes principales
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

    // Sous-commandes de configco
    CLI::App* configBandco = nullptr;
    CLI::App* configCanco  = nullptr;
    CLI::App* configSaveco = nullptr;
    CLI::App* configZeroco = nullptr;

    // Sous-commandes de eds
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

    // Sous-commandes de nmt
    CLI::App* nmtRead               = nullptr;
    CLI::App* nmtOperational        = nullptr;
    CLI::App* nmtStop               = nullptr;
    CLI::App* nmtPreOperational     = nullptr;
    CLI::App* nmtResetNode          = nullptr;
    CLI::App* nmtResetCommunication = nullptr;

    // Sous-commandes de register
    CLI::App* regReadco  = nullptr;
    CLI::App* regWriteco = nullptr;

    // Sous-commandes de pdoTestco
    CLI::App* PdoSpeedco    = nullptr;
    CLI::App* PdoPositionco = nullptr;
    CLI::App* PdoCustomco   = nullptr;

    // Sous-commandes de setupco
    CLI::App*    setupCalibco       = nullptr;
    CLI::App*    setupCalibOutco    = nullptr;
    CLI::App*    setupInfoco        = nullptr;
    CLI::App*    setupMotorco       = nullptr;
    CLI::App*    setupReadCfgco     = nullptr;
    CLI::Option* setupInfoAllFlagco = nullptr;

    // Sous-commandes de SDOsegmentedco
    CLI::App* SDOsegmentedReadco  = nullptr;
    CLI::App* SDOsegmentedWriteco = nullptr;

    // Sous-commandes de testco
    CLI::App* testEncoderco     = nullptr;
    CLI::App* testEncoderMainco = nullptr;
    CLI::App* testEncoderOutco  = nullptr;
    CLI::App* testLatencyco     = nullptr;
    CLI::App* testMoveco        = nullptr;
    CLI::App* testMoveAbsco     = nullptr;
    CLI::App* testMoveRelco     = nullptr;
    CLI::App* testMoveSpeedco   = nullptr;
    CLI::App* testImpedanceco   = nullptr;

    // Ajoute ici les propriétés/options nécessaires pour chaque commande
};