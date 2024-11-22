/*
    MAB Robotics

    Power Distribution System Example 2

    Basic submodules operations

*/

#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

constexpr socketIndex_E POWER_STAGE_SOCKET_INDEX        = socketIndex_E::SOCKET_1;
constexpr socketIndex_E BRAKE_RESISTOR_SOCKET_INDEX     = socketIndex_E::SOCKET_2;
constexpr socketIndex_E ISOLATED_CONVERTER_SOCKET_INDEX = socketIndex_E::SOCKET_3;

int main()
{
    Logger _log;
    _log.m_tag = "PDS Example 2";

    Candle candle(mab::CAN_BAUD_1M, true);
    Pds    pds(PDS_CAN_ID, candle);

    auto powerStage        = pds.attachPowerStage(POWER_STAGE_SOCKET_INDEX);
    auto brakeResistor     = pds.attachBrakeResistor(BRAKE_RESISTOR_SOCKET_INDEX);
    auto isolatedConverter = pds.attachIsolatedConverter12(ISOLATED_CONVERTER_SOCKET_INDEX);

    if (powerStage == nullptr)
        exit(EXIT_FAILURE);

    if (brakeResistor == nullptr)
        exit(EXIT_FAILURE);

    if (isolatedConverter == nullptr)
        exit(EXIT_FAILURE);

    return EXIT_SUCCESS;
}