/*
    MAB Robotics

    Power Distribution System Example

    Power stage and braking resistor
*/

#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

constexpr socketIndex_E BRAKE_RESISTOR_SOCKET_INDEX = socketIndex_E::SOCKET_3;

int main()
{
    Logger _log;
    _log.m_tag = "PDS Example";

    auto candle = attachCandle(CANdleBaudrate_E::CAN_BAUD_1M, candleTypes::busTypes_t::USB);
    Pds  pds(PDS_CAN_ID, candle);

    if (pds.init() != PdsModule::error_E::OK)
    {
        _log.error("No PDSs found!");
        return EXIT_FAILURE;
    }

    auto brakeResistor = pds.attachBrakeResistor(BRAKE_RESISTOR_SOCKET_INDEX);

    // Configuration
    brakeResistor->setTemperatureLimit(90);

    // Variables for status
    brakeResistorStatus_S status;
    f32                   temperature;
    f32                   temperatureLimit;

    // Read status
    brakeResistor->getStatus(status);
    brakeResistor->getTemperature(temperature);
    brakeResistor->getTemperatureLimit(temperatureLimit);

    // Logging
    _log.info("Braking resistor");
    _log.info("Enabled :: [ %s ]", status.ENABLED ? "YES" : "NO");
    _log.info("Temperature :: [ %.2f °C ]", temperature);
    _log.info("Temperature limit :: [ %.2f °C ]", temperatureLimit);

    return EXIT_SUCCESS;
}