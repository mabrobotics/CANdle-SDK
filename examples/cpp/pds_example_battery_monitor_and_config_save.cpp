/*
    MAB Robotics

    Power Distribution System Example 3

    Reading data from power stage

*/

#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

constexpr u32 BATTERY_LEVEL_1 = 20000;  // 20V
constexpr u32 BATTERY_LEVEL_2 = 24000;  // 24V

constexpr u32 SHUTDOWN_TIME = 5000;  // 5 seconds

int main()
{
    Logger _log;
    _log.m_tag = "PDS Example";

    auto candle =
        mab::attachCandle(mab::CANdleBaudrate_E::CAN_BAUD_1M, mab::candleTypes::busTypes_t::USB);
    Pds pds(PDS_CAN_ID, candle);

    PdsModule::error_E result = PdsModule::error_E::OK;

    pds.init();

    _log.info("Setting battery voltage levels to %u and %u", BATTERY_LEVEL_1, BATTERY_LEVEL_2);
    result = pds.setBatteryVoltageLevels(BATTERY_LEVEL_1, BATTERY_LEVEL_2);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Setting battery voltage levels failed [ %s ]", PdsModule::error2String(result));
        return EXIT_FAILURE;
    }

    _log.info("Setting shutdown time to %u", SHUTDOWN_TIME);
    result = pds.setShutdownTime(SHUTDOWN_TIME);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Setting shutdown time Failed [ %s ]", PdsModule::error2String(result));
        return EXIT_FAILURE;
    }

    _log.info("Saving configuration");
    result = pds.saveConfig();
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Saving configuration [ %s ]", PdsModule::error2String(result));
        return EXIT_FAILURE;
    }

    pds.shutdown();

    controlBoardStatus_S status = {};
    while (!status.SHUTDOWN_SCHEDULED)
    {
        pds.getStatus(status);
    }

    return EXIT_SUCCESS;
}