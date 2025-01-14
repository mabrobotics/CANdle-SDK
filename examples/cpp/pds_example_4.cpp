/*
    MAB Robotics

    Power Distribution System Example 4

    Writes Control board properties
*/
#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

int main()
{
    Logger _log;
    _log.m_tag = "PDS Example 4";

    Candle             candle(mab::CAN_BAUD_1M, true);
    Pds                pds(PDS_CAN_ID, candle);
    f32                temperatureLimit = 12.0f;
    PdsModule::error_E result           = PdsModule::error_E::OK;

    result = pds.getTemperatureLimit(temperatureLimit);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Unable to get initial temperature limit");
        return EXIT_FAILURE;
    }

    _log.success("Initial temperature limit read OK [ %.2f ^C ]", temperatureLimit);

    _log.info("Changing temperature limit to 50.0 ^C...");

    result = pds.setTemperatureLimit(50.0f);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Unable to set new temperature limit");
        return EXIT_FAILURE;
    }

    _log.success("Temperature limit change OK");

    result = pds.getTemperatureLimit(temperatureLimit);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Unable to get temperature limit after change...");
        return EXIT_FAILURE;
    }

    _log.success("Temperature limit read after change OK [ %.2f ^C ]", temperatureLimit);

    return EXIT_SUCCESS;
}