/*
    MAB Robotics

    Power Distribution System Example

    Power stage and braking resistor binding
*/

#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

constexpr socketIndex_E POWER_STAGE_SOCKET_INDEX    = socketIndex_E::SOCKET_1;
constexpr socketIndex_E BRAKE_RESISTOR_SOCKET_INDEX = socketIndex_E::SOCKET_3;

int main()
{
    Logger _log;
    _log.m_tag = "PDS Example";

    auto candle = mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M,
                                    mab::candleTypes::busTypes_t::USB);
    Pds  pds(PDS_CAN_ID, candle);

    if (pds.init() != PdsModule::error_E::OK)
    {
        _log.error("No PDSs found!");
        return EXIT_FAILURE;
    }

    auto powerStage    = pds.attachPowerStage(POWER_STAGE_SOCKET_INDEX);
    auto brakeResistor = pds.attachBrakeResistor(BRAKE_RESISTOR_SOCKET_INDEX);

    if (powerStage == nullptr)
        exit(EXIT_FAILURE);

    // Configuration
    powerStage->setTemperatureLimit(90.0f);  // 90 Celsius degrees
    powerStage->setOcdLevel(25000);          // 25 A OCD level
    powerStage->setOcdDelay(1000);           // 1 mS delay

    // Braking resistor configuration
    powerStage->setBrakeResistorTriggerVoltage(30000);  // 30V DC
    powerStage->bindBrakeResistor(brakeResistor->getSocketIndex());

    powerStage->enable();

    sleep(1);  // Wait 1 second until power stage is enabled

    // Variables for status
    powerStageStatus_S powerStageStatus = {};
    float              temperature      = 0.0f;
    u32                outputVoltage    = 0;
    s32                outputCurrent    = 0;

    // Read status
    powerStage->getStatus(powerStageStatus);
    powerStage->getOutputVoltage(outputVoltage);
    powerStage->getLoadCurrent(outputCurrent);

    // Logging
    _log.info("Power stage");
    _log.info("Enabled :: [ %s ]", powerStageStatus.ENABLED ? "YES" : "NO");
    _log.info("Over current :: [ %s ]", powerStageStatus.OVER_CURRENT ? "YES" : "NO");
    _log.info("Over temperature :: [ %s ]", powerStageStatus.OVER_TEMPERATURE ? "YES" : "NO");
    _log.info("Voltage :: [ %.2f ]", static_cast<float>(outputVoltage / 1000.0f));
    _log.info("Temperature :: [ %.2f ]", temperature);
    _log.info("Current :: [ %.2f ]", static_cast<float>(outputCurrent / 1000.0f));

    powerStage->disable();

    detachCandle(candle);

    return EXIT_SUCCESS;
}
