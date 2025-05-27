/*
    MAB Robotics

    Power Distribution System Example: Basic

    Reading data from PDS Control board:
        * Connected submodules list
        * Control board status word
        * DC Bus voltage
        * Submodules Info
*/
#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

int main()
{
    auto candle =
        mab::attachCandle(mab::CANdleBaudrate_E::CAN_BAUD_1M, mab::candleTypes::busTypes_t::USB);
    Pds pds(PDS_CAN_ID, candle);

    pds.init();

    Pds::modulesSet_S pdsModules = pds.getModules();

    Logger _log;
    _log.m_tag = "PDS Example";
    _log.info("PDS have the following set of connected modules:");
    _log.info("\tSocket 1 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket1));
    _log.info("\tSocket 2 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket2));
    _log.info("\tSocket 3 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket3));
    _log.info("\tSocket 4 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket4));
    _log.info("\tSocket 5 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket5));
    _log.info("\tSocket 6 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket6));

    controlBoardStatus_S pdsStatus     = {0};
    u32                  pdsBusVoltage = 0;
    f32                  temperature   = 0.0f;

    pds.getStatus(pdsStatus);
    pds.getBusVoltage(pdsBusVoltage);
    pds.getTemperature(temperature);

    _log.info("Enabled : %s", pdsStatus.ENABLED ? "YES" : "NO");
    _log.info("STO1 : %s", pdsStatus.STO_1 ? "YES" : "NO");
    _log.info("STO2 : %s", pdsStatus.STO_2 ? "YES" : "NO");
    _log.info("CAN Timeout : %s", pdsStatus.FDCAN_TIMEOUT ? "YES" : "NO");
    _log.info("Over temperature : %s", pdsStatus.OVER_TEMPERATURE ? "YES" : "NO");
    _log.info("Bus voltage : %u", pdsBusVoltage);

    return 0;
}