/*
    MAB Robotics

    Power Distribution System Example 1

    Reading data from PDS Control board:
        * Connected submodules list
        * DC Bus voltage
*/
#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

int main()
{
    Logger _log;
    _log.m_tag = "PDS Example 1";

    Candle candle(mab::CAN_BAUD_1M, true);
    Pds    pds(PDS_CAN_ID, candle);

    Pds::modulesSet_S pdsModules = pds.getModules();

    u32      pdsBusVoltage = 0;
    status_S pdsStatus     = {0};

    pds.getStatus(pdsStatus);
    pds.getBusVoltage(pdsBusVoltage);

    _log.info("PDS have the following set of connected modules:");
    _log.info("\t1\t:: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket1));
    _log.info("\t2\t:: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket2));
    _log.info("\t3\t:: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket3));
    _log.info("\t4\t:: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket4));
    _log.info("\t5\t:: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket5));
    _log.info("\t6\t:: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket6));

    _log.info("PDS Status:");

    _log.info("\t * ENABLED           [ %s ]", pdsStatus.ENABLED ? "YES" : "NO");
    _log.info("\t * OVER_TEMPERATURE  [ %s ]", pdsStatus.OVER_TEMPERATURE ? "YES" : "NO");
    _log.info("\t * OVER_CURRENT      [ %s ]", pdsStatus.OVER_CURRENT ? "YES" : "NO");
    _log.info("\t * STO_1             [ %s ]", pdsStatus.STO_1 ? "YES" : "NO");
    _log.info("\t * STO_2             [ %s ]", pdsStatus.STO_2 ? "YES" : "NO");
    _log.info("\t * FDCAN_TIMEOUT     [ %s ]", pdsStatus.FDCAN_TIMEOUT ? "YES" : "NO");
    _log.info("\t * SUBMODULE_1_ERROR [ %s ]", pdsStatus.SUBMODULE_1_ERROR ? "YES" : "NO");
    _log.info("\t * SUBMODULE_2_ERROR [ %s ]", pdsStatus.SUBMODULE_2_ERROR ? "YES" : "NO");
    _log.info("\t * SUBMODULE_3_ERROR [ %s ]", pdsStatus.SUBMODULE_3_ERROR ? "YES" : "NO");
    _log.info("\t * SUBMODULE_4_ERROR [ %s ]", pdsStatus.SUBMODULE_4_ERROR ? "YES" : "NO");
    _log.info("\t * SUBMODULE_5_ERROR [ %s ]", pdsStatus.SUBMODULE_5_ERROR ? "YES" : "NO");
    _log.info("\t * SUBMODULE_6_ERROR [ %s ]", pdsStatus.SUBMODULE_6_ERROR ? "YES" : "NO");
    _log.info("\t * CHARGER_DETECTED  [ %s ]", pdsStatus.CHARGER_DETECTED ? "YES" : "NO");

    _log.info("Bus voltage: %0.2f", pdsBusVoltage / 1000.0f);

    return EXIT_SUCCESS;
}