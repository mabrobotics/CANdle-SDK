/*
    MAB Robotics

    Power Distribution System Example 1

    Reading data from PDS Control board:
        * Connected submodules list
        * Control board status word
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

    u32      shutdownTime  = 0;
    u32      batteryLvl1   = 0;
    u32      batteryLvl2   = 0;
    u32      pdsBusVoltage = 0;
    status_S pdsStatus     = {0};

    version_ut pdsFwVersion = {0};

    pds.getFwVersion(pdsFwVersion);
    pds.getStatus(pdsStatus);
    pds.getBusVoltage(pdsBusVoltage);
    pds.getShutdownTime(shutdownTime);
    pds.getBatteryVoltageLevels(batteryLvl1, batteryLvl2);

    _log.info("Firmware Version: %u.%u.%u",
              pdsFwVersion.s.major,
              pdsFwVersion.s.minor,
              pdsFwVersion.s.revision);

    _log.info("Submodules:");
    _log.info("\t1 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket1));
    _log.info("\t2 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket2));
    _log.info("\t3 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket3));
    _log.info("\t4 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket4));
    _log.info("\t5 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket5));
    _log.info("\t6 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket6));

    _log.info("PDS Status:");

    _log.info("\t* ENABLED           [ %s ]", pdsStatus.ENABLED ? "YES" : "NO");
    _log.info("\t* OVER_TEMPERATURE  [ %s ]", pdsStatus.OVER_TEMPERATURE ? "YES" : "NO");
    _log.info("\t* OVER_CURRENT      [ %s ]", pdsStatus.OVER_CURRENT ? "YES" : "NO");
    _log.info("\t* STO_1             [ %s ]", pdsStatus.STO_1 ? "YES" : "NO");
    _log.info("\t* STO_2             [ %s ]", pdsStatus.STO_2 ? "YES" : "NO");
    _log.info("\t* FDCAN_TIMEOUT     [ %s ]", pdsStatus.FDCAN_TIMEOUT ? "YES" : "NO");
    _log.info("\t* SUBMODULE_1_ERROR [ %s ]", pdsStatus.SUBMODULE_1_ERROR ? "YES" : "NO");
    _log.info("\t* SUBMODULE_2_ERROR [ %s ]", pdsStatus.SUBMODULE_2_ERROR ? "YES" : "NO");
    _log.info("\t* SUBMODULE_3_ERROR [ %s ]", pdsStatus.SUBMODULE_3_ERROR ? "YES" : "NO");
    _log.info("\t* SUBMODULE_4_ERROR [ %s ]", pdsStatus.SUBMODULE_4_ERROR ? "YES" : "NO");
    _log.info("\t* SUBMODULE_5_ERROR [ %s ]", pdsStatus.SUBMODULE_5_ERROR ? "YES" : "NO");
    _log.info("\t* SUBMODULE_6_ERROR [ %s ]", pdsStatus.SUBMODULE_6_ERROR ? "YES" : "NO");
    _log.info("\t* CHARGER_DETECTED  [ %s ]", pdsStatus.CHARGER_DETECTED ? "YES" : "NO");

    _log.info("---------------------------------");

    _log.info("Config data:");
    _log.info("\t* shutdown time: [ %u mS ] ", shutdownTime);
    _log.info("\t* Battery level 1: %0.2f", batteryLvl1 / 1000.0f);
    _log.info("\t* Battery level 2: %0.2f", batteryLvl2 / 1000.0f);

    _log.info("---------------------------------");

    _log.info("Metrology data:");
    _log.info("\t* Bus voltage: %0.2f", pdsBusVoltage / 1000.0f);

    return EXIT_SUCCESS;
}