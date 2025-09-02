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

int main()
{
    Logger m_log;
    m_log.m_tag   = "PDS";
    m_log.m_layer = Logger::ProgramLayer_E::TOP;

    auto candle    = attachCandle(CANdleDatarate_E::CAN_DATARATE_1M, candleTypes::busTypes_t::USB);
    auto findPdses = Pds::discoverPDS(candle);

    if (findPdses.size() == 0)
    {
        m_log.error("No PDSs found!");
        return EXIT_FAILURE;
    }

    Pds pds(findPdses[0], candle);

    pds.init();

    mab::Pds::modulesSet_S pdsModules = pds.getModules();

    // Configuration variables
    mab::controlBoardStatus_S pdsStatus     = {0};
    u32                       pdsBusVoltage = 0;
    u32                       shutdownTime  = 0;
    u32                       batteryLvl1   = 0;
    u32                       batteryLvl2   = 0;
    socketIndex_E             brSocket      = socketIndex_E::UNASSIGNED;
    u32                       brTrigger     = 0;
    moduleVersion_E           ctrlBoardVersion;
    pdsFwMetadata_S           fwVersion;

    PdsModule::error_E result = pds.getStatus(pdsStatus);
    if (result != PdsModule::error_E::OK)
        m_log.error("PDS get status failed [ %s ]", PdsModule::error2String(result));

    result = pds.getBusVoltage(pdsBusVoltage);
    if (result != PdsModule::error_E::OK)
        m_log.error("PDS get bus voltage failed [ %s ]", PdsModule::error2String(result));

    result = pds.getShutdownTime(shutdownTime);
    if (result != PdsModule::error_E::OK)
        m_log.error("PDS get shutdown time failed [ %s ]", PdsModule::error2String(result));

    result = pds.getBatteryVoltageLevels(batteryLvl1, batteryLvl2);
    if (result != PdsModule::error_E::OK)
        m_log.error("PDS get battery levels failed [ %s ]", PdsModule::error2String(result));

    result = pds.getBindBrakeResistor(brSocket);
    if (result != PdsModule::error_E::OK)
        m_log.error("Power Stage get brake resistor failed [ %s ]",
                    PdsModule::error2String(result));

    result = pds.getFwMetadata(fwVersion);
    if (result != PdsModule::error_E::OK)
        m_log.error("FW version read failed [ %s ]", PdsModule::error2String(result));

    result = pds.getBoardVersion(ctrlBoardVersion);
    if (result != PdsModule::error_E::OK)
        m_log.error("HW version read failed [ %s ]", PdsModule::error2String(result));

    if (brSocket != socketIndex_E::UNASSIGNED)
    {
        result = pds.getBrakeResistorTriggerVoltage(brTrigger);
        if (result != PdsModule::error_E::OK)
            m_log.error("Power Stage get brake resistor trigger voltage failed [ %s ]",
                        PdsModule::error2String(result));
    }

    // Logging
    m_log.info("Power Distribution Module");
    m_log.info("CTRL board version: [%d]", ctrlBoardVersion);

    m_log.info("Firmware version: %u.%u.%u-%s",
               fwVersion.version.s.major,
               fwVersion.version.s.minor,
               fwVersion.version.s.revision,
               fwVersion.gitHash);
    m_log.info("Submodules:");
    m_log.info("  1 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket1));
    m_log.info("  2 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket2));
    m_log.info("  3 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket3));
    m_log.info("  4 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket4));
    m_log.info("  5 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket5));
    m_log.info("  6 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket6));
    m_log.info("PDS Status:");
    m_log.info("  * ENABLED           [ %s ]", pdsStatus.ENABLED ? "YES" : "NO");
    m_log.info("  * OVER_TEMPERATURE  [ %s ]", pdsStatus.OVER_TEMPERATURE ? "YES" : "NO");
    m_log.info("  * OVER_CURRENT      [ %s ]", pdsStatus.OVER_CURRENT ? "YES" : "NO");
    m_log.info("  * STO_1             [ %s ]", pdsStatus.STO_1 ? "YES" : "NO");
    m_log.info("  * STO_2             [ %s ]", pdsStatus.STO_2 ? "YES" : "NO");
    m_log.info("  * FDCAN_TIMEOUT     [ %s ]", pdsStatus.FDCAN_TIMEOUT ? "YES" : "NO");
    m_log.info("  * SUBMODULE_1_ERROR [ %s ]", pdsStatus.SUBMODULE_1_ERROR ? "YES" : "NO");
    m_log.info("  * SUBMODULE_2_ERROR [ %s ]", pdsStatus.SUBMODULE_2_ERROR ? "YES" : "NO");
    m_log.info("  * SUBMODULE_3_ERROR [ %s ]", pdsStatus.SUBMODULE_3_ERROR ? "YES" : "NO");
    m_log.info("  * SUBMODULE_4_ERROR [ %s ]", pdsStatus.SUBMODULE_4_ERROR ? "YES" : "NO");
    m_log.info("  * SUBMODULE_5_ERROR [ %s ]", pdsStatus.SUBMODULE_5_ERROR ? "YES" : "NO");
    m_log.info("  * SUBMODULE_6_ERROR [ %s ]", pdsStatus.SUBMODULE_6_ERROR ? "YES" : "NO");
    m_log.info("  * CHARGER_DETECTED  [ %s ]", pdsStatus.CHARGER_DETECTED ? "YES" : "NO");
    m_log.info("---------------------------------");
    m_log.info("Config data:");
    m_log.info("  * shutdown time: [ %u mS ] ", shutdownTime);
    m_log.info("  * Battery level 1: [ %0.2f V ]", batteryLvl1 / 1000.0f);
    m_log.info("  * Battery level 2: [ %0.2f V ]", batteryLvl2 / 1000.0f);
    if (brSocket == socketIndex_E::UNASSIGNED)
        m_log.info("  * Brake resistor is not set");
    else
    {
        m_log.info("  * Brake resistor socket [ %u ]", (u8)brSocket);
        m_log.info("  * Brake resistor trigger voltage [ %0.2f V ]", brTrigger / 1000.0f);
    }
    m_log.info("---------------------------------");
    m_log.info("Metrology data:");
    m_log.info("Bus voltage: %0.2f V", pdsBusVoltage / 1000.0f);

    return EXIT_SUCCESS;
}
