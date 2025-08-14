/*
    MAB Robotics

    Power Distribution System Example

    Configuration: Battery levels and shutdown time
*/

#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 120;

constexpr u32 BATTERY_LEVEL_1 = 20000;  // 20V
constexpr u32 BATTERY_LEVEL_2 = 24000;  // 24V

constexpr u32 SHUTDOWN_TIME = 5000;  // 5 seconds

int main()
{
    Logger _log;
    _log.m_tag = "PDS Example";

    auto candle = attachCandle(CANdleBaudrate_E::CAN_BAUD_1M, candleTypes::busTypes_t::USB);
    Pds  pds(PDS_CAN_ID, candle);

    PdsModule::error_E result = PdsModule::error_E::OK;

    pds.init();

    // Set battery voltage thresholds
    _log.info("Setting battery voltage levels to [ %0.2f V ] and [ %0.2f V ]",
              BATTERY_LEVEL_1 / 1000.0f,
              BATTERY_LEVEL_2 / 1000.0f);
    result = pds.setBatteryVoltageLevels(BATTERY_LEVEL_1, BATTERY_LEVEL_2);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Setting battery voltage levels failed [ %s ]", PdsModule::error2String(result));
        return EXIT_FAILURE;
    }

    // Set shutdown time
    _log.info("Setting shutdown time to [ %0.2f s ]", SHUTDOWN_TIME / 1000.0f);
    result = pds.setShutdownTime(SHUTDOWN_TIME);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Setting shutdown time failed [ %s ]", PdsModule::error2String(result));
        return EXIT_FAILURE;
    }

    // Save configuration to non-volatile memory
    _log.info("Saving configuration to device");
    result = pds.saveConfig();
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Saving configuration failed [ %s ]", PdsModule::error2String(result));
        return EXIT_FAILURE;
    }

    // Initiate shutdown
    _log.info("Sending shutdown request...");
    pds.shutdown();

    // Wait until shutdown is scheduled
    controlBoardStatus_S status = {};
    while (!status.SHUTDOWN_SCHEDULED)
    {
        pds.getStatus(status);
    }

    _log.info("Shutdown scheduled successfully.");

    return EXIT_SUCCESS;
}
