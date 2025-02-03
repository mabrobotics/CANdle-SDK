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

void printModuleInfo(moduleType_E type, socketIndex_E socket);
void printPowerStageInfo(PowerStage& powerStage);
void printBrakeResistorInfo(BrakeResistor& brakeResistor);
void printIsolatedConverterInfo(IsolatedConv& isolatedConverter);

Logger _log;

Candle candle(mab::CAN_BAUD_1M, true);
Pds    pds(PDS_CAN_ID, candle);

int main()
{
    _log.m_tag                   = "PDS Example 1";
    Pds::modulesSet_S pdsModules = pds.getModules();

    u32                  shutdownTime  = 0;
    u32                  batteryLvl1   = 0;
    u32                  batteryLvl2   = 0;
    u32                  pdsBusVoltage = 0;
    controlBoardStatus_S pdsStatus     = {0};

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
    printModuleInfo(pdsModules.moduleTypeSocket1, socketIndex_E::SOCKET_1);

    _log.info("\t2 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket2));
    printModuleInfo(pdsModules.moduleTypeSocket2, socketIndex_E::SOCKET_2);

    _log.info("\t3 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket3));
    printModuleInfo(pdsModules.moduleTypeSocket3, socketIndex_E::SOCKET_3);

    _log.info("\t4 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket4));
    printModuleInfo(pdsModules.moduleTypeSocket4, socketIndex_E::SOCKET_4);

    _log.info("\t5 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket5));
    printModuleInfo(pdsModules.moduleTypeSocket5, socketIndex_E::SOCKET_5);

    _log.info("\t6 :: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket6));
    printModuleInfo(pdsModules.moduleTypeSocket6, socketIndex_E::SOCKET_6);

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

void printModuleInfo(moduleType_E type, socketIndex_E socket)
{
    switch (type)
    {
        case moduleType_E::OUT_OF_RANGE:
        case moduleType_E::CONTROL_BOARD:
        case moduleType_E::UNDEFINED:
        default:
            return;

        case moduleType_E::POWER_STAGE:
        {
            auto powerStage = pds.attachPowerStage(socket);
            printPowerStageInfo(*powerStage);
            break;
        }

        case moduleType_E::ISOLATED_CONVERTER:
        {
            auto isolatedConverter = pds.attachIsolatedConverter(socket);
            printIsolatedConverterInfo(*isolatedConverter);
            break;
        }

        case moduleType_E::BRAKE_RESISTOR:
        {
            auto brakeResistor = pds.attachBrakeResistor(socket);
            printBrakeResistorInfo(*brakeResistor);
            break;
        }
    }
}

void printPowerStageInfo(PowerStage& powerStage)
{
    powerStageStatus_S status;
    moduleVersion_E    boardVersion;
    socketIndex_E      bindedBrSocket;
    u32                brTriggerVoltage;

    float temperature = 0.0f;
    s32   current     = 0;
    u32   voltage     = 0;

    powerStage.getStatus(status);
    powerStage.getBoardVersion(boardVersion);
    powerStage.getBindBrakeResistor(bindedBrSocket);
    powerStage.getBrakeResistorTriggerVoltage(brTriggerVoltage);
    powerStage.getTemperature(temperature);
    powerStage.getLoadCurrent(current);
    powerStage.getOutputVoltage(voltage);

    _log.info("\t\t* HW Version [ %u ]", boardVersion);
    _log.info("\t\t* Status:");
    _log.info("\t\t\t* Enabled\t[ %s ]", status.ENABLED ? "YES" : "NO");
    _log.info("\t\t\t* OVT\t\t[ %s ]", status.OVER_TEMPERATURE ? "YES" : "NO");
    _log.info("\t\t\t* OVC\t\t[ %s ]", status.OVER_CURRENT ? "YES" : "NO");
    _log.info("\t\t* BR Socket [ %u ] ( 0 == BR not binded )", (u8)bindedBrSocket);
    _log.info("\t\t* BR Trig. V. [ %.2f ][ V ]", (float)brTriggerVoltage / 1000.0f);
    _log.info("\t\t* Measurements:");
    _log.info("\t\t\t* Temperature\t[ %.2f ][ ^C ]", temperature);
    _log.info("\t\t\t* Current\t[ %.2f ][ A ]", (float)current / 1000.0f);
    _log.info("\t\t\t* Voltage\t[ %.2f ][ V ]", (float)voltage / 1000.0f);
}

void printBrakeResistorInfo(BrakeResistor& brakeResistor)
{
    brakeResistorStatus_S status       = {0};
    moduleVersion_E       boardVersion = moduleVersion_E::UNKNOWN;
    float                 temperature  = 0.0f;
    ;
    brakeResistor.getStatus(status);
    brakeResistor.getBoardVersion(boardVersion);
    brakeResistor.getTemperature(temperature);

    _log.info("\t\t* HW Version [ %u ]", boardVersion);
    _log.info("\t\t* Status:");
    _log.info("\t\t\t* Enabled\t[ %s ]", status.ENABLED ? "YES" : "NO");
    _log.info("\t\t\t* OVT\t\t[ %s ]", status.OVER_TEMPERATURE ? "YES" : "NO");
    _log.info("\t\t\t* OVC\t\t[ %s ]", status.OVER_CURRENT ? "YES" : "NO");
    _log.info("\t\t* Measurements:");
    _log.info("\t\t\t* Temperature [ %.2f ][ ^C ]", temperature);
}

void printIsolatedConverterInfo(IsolatedConv& isolatedConverter)
{
    _log.info("Isolated Converter module info:");
}