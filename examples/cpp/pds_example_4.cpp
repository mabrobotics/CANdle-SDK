/*
    MAB Robotics

    Power Distribution System Example 4

    PDS manual turn off procedure
*/
#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

constexpr u32 DESIRED_TURN_OFF_TIME_mS = 3000u;

int main()
{
    Logger _log;
    _log.m_tag = "PDS Example 4";

    Candle   candle(mab::CAN_BAUD_1M, true);
    Pds      pds(PDS_CAN_ID, candle);
    status_S pdsStatus = {0};

    PdsModule::error_E result = PdsModule::error_E::OK;

    _log.info("Setting PDS turnoff time to [ %u ] mS...", DESIRED_TURN_OFF_TIME_mS);

    result = pds.setTurnOffTime(DESIRED_TURN_OFF_TIME_mS);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Unable to set new turnoff time");
        return EXIT_FAILURE;
    }

    _log.success("turnoff time change OK");

    _log.info("Waiting for the user tu turn off the PDS device by holding the power button...");

    while (1)
    {
        usleep(100000);
        result = pds.getStatus(pdsStatus);
        if (result != PdsModule::error_E::OK)
        {
            _log.error("Unable to read PDS status property");
            return EXIT_FAILURE;
        }

        if (pdsStatus.TURN_OFF_SCHEDULED)
        {
            _log.success("PDS turn off procedure started. Disabling the HOST device");
            // << YOUR CALL TO TURN OFF HOST DEVICE >>
            return EXIT_SUCCESS;
        }
    }

    return EXIT_SUCCESS;
}