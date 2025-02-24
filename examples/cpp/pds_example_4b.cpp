/*
    MAB Robotics

    Power Distribution System Example 4b

    PDS shutdown command
*/
#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

constexpr u32 DESIRED_SHUTDOWN_TIME_mS = 3000u;

int main()
{
    Logger _log;
    _log.m_tag = "PDS Example 4b";

    Candle candle(mab::CAN_BAUD_1M, true);
    Pds    pds(PDS_CAN_ID, candle);

    pds.init();

    PdsModule::error_E result = PdsModule::error_E::OK;

    _log.info("Setting PDS shutdown time to [ %u ] mS...", DESIRED_SHUTDOWN_TIME_mS);

    result = pds.setShutdownTime(DESIRED_SHUTDOWN_TIME_mS);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Unable to set new shutdown time");
        return EXIT_FAILURE;
    }

    _log.success("shutdown time change OK");

    _log.info("Sending shutdown commmand to PDS device");

    result = pds.shutdown();
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Unable to shutdown PDS device");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}