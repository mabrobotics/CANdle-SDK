/*
    MAB Robotics

    Power Distribution System Example 3

    Reading data from power stage

*/

#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

constexpr socketIndex_E POWER_STAGE_SOCKET_INDEX = socketIndex_E::SOCKET_5;

int main()
{
    Logger _log;
    _log.m_tag = "PDS Example 3";

    Candle candle(mab::CAN_BAUD_1M, true);
    Pds    pds(PDS_CAN_ID, candle);

    auto powerStage = pds.attachPowerStage(POWER_STAGE_SOCKET_INDEX);

    if (powerStage == nullptr)
        exit(EXIT_FAILURE);

    powerStage->setTemperatureLimit(90.0f);             // 90 Celsius degrees
    powerStage->setOcdLevel(25000);                     // 25 A OCD level
    powerStage->setOcdDelay(1000);                      // 1 mS delay
    powerStage->setBrakeResistorTriggerVoltage(30000);  // 30V DC

    powerStage->enable();
    usleep(200000);  // Wait 2 seconds until power stage is enabled

    // PowerStage::status_S powerStageStatus = {};
    float temperature   = 0.0f;
    u32   outputVoltage = 0;
    s32   outputCurrent = 0;

    // powerStage->getStatus(powerStageStatus);
    powerStage->getOutputVoltage(outputVoltage);
    powerStage->getLoadCurrent(outputCurrent);

    _log.info("Power stage");
    // _log.info("Enabled :: [ %s ]", powerStageStatus.ENABLED ? "ON" : "OFF");
    // _log.info("Over current event :: [ %s ]", powerStageStatus.OCD_EVENT ? "YES" : "NO");
    // _log.info("Over temperature event :: [ %s ]", powerStageStatus.OVT_EVENT ? "YES" : "NO");
    _log.info("Voltage :: [ %.2f ]", static_cast<float>(outputVoltage / 1000.0f));
    _log.info("Temperature :: [ %.2f ]", temperature);
    _log.info("Current :: [ %.2f ]", static_cast<float>(outputCurrent / 1000.0f));

    while (1)
    {
        powerStage->getOutputVoltage(outputVoltage);
        powerStage->getLoadCurrent(outputCurrent);
        _log.info("V :: [ %.2f V ] I :: [ %.2f A ] ",
                  static_cast<float>(outputVoltage / 1000.0f),
                  static_cast<float>(outputCurrent / 1000.0f));
        usleep(500000);  // Wait 2 seconds until power stage is enabled
    }

    return EXIT_SUCCESS;
}