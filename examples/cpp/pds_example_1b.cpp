/*
    MAB Robotics

    Power Distribution System Example 1b

    Writing data to PDS Control board:
        * Battery voltage levels ( For energy level determination )
        * Shutdown time
*/
#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

constexpr u32 BATTERY_LVL_1 = 20000;  // 20V
constexpr u32 BATTERY_LVL_2 = 24000;  // 24V

constexpr u32 SHUTDOWN_TIME = 5000;  // 5s

int main()
{
    Logger _log;
    _log.m_tag = "PDS Example 1b";

    Candle candle(mab::CAN_BAUD_1M, true);
    Pds    pds(PDS_CAN_ID, candle);

    PdsModule::error_E result = PdsModule::error_E::OK;

    result = pds.setBatteryVoltageLevels(BATTERY_LVL_1, BATTERY_LVL_2);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Unable to set battery voltage levels");
        return EXIT_FAILURE;
    }

    _log.success("Battery levels set OK");

    result = pds.setShutdownTime(SHUTDOWN_TIME);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Unable to set Shutdown time!");
        return EXIT_FAILURE;
    }

    _log.success("shutdown time set OK");

    result = pds.saveConfig();
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Unable to save current configuration on target!");
        return EXIT_FAILURE;
    }

    _log.success("configuration save OK");

    return EXIT_SUCCESS;
}