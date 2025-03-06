/*
    MAB Robotics

    Power Distribution System Example

    Power stage basics

*/

#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

constexpr socketIndex_E POWER_STAGE_SOCKET_INDEX = socketIndex_E::SOCKET_2;

int main()
{
    Logger _log;
    _log.m_tag = "PDS Example";

    Candle candle(mab::CAN_BAUD_1M, true);
    Pds    pds(PDS_CAN_ID, candle);

    pds.init();

    auto powerStage = pds.attachPowerStage(POWER_STAGE_SOCKET_INDEX);

    if (powerStage == nullptr)
        exit(EXIT_FAILURE);

    powerStage->setTemperatureLimit(90.0f);  // 90 Celsius degrees
    powerStage->setOcdLevel(50000);          // 50 A OCD level
    powerStage->setOcdDelay(1000);           // 1 mS delay

    powerStage->enable();

    float temperature   = 0.0f;
    u32   outputVoltage = 0;
    s32   outputCurrent = 0;

    // for (int i = 0; i < 30; ++i)
    while (1)
    {
        powerStage->getOutputVoltage(outputVoltage);
        powerStage->getLoadCurrent(outputCurrent);
        powerStage->getTemperature(temperature);

        _log.info("Voltage :: [ %.2f ]", static_cast<float>(outputVoltage / 1000.0f));
        _log.info("Current :: [ %.2f ]", static_cast<float>(outputCurrent / 1000.0f));
        _log.info("Temperature :: [ %.2f ]", temperature);

        usleep(100000);  // sleep for 100 milliseconds
    }

    powerStage->disable();

    return EXIT_SUCCESS;
}