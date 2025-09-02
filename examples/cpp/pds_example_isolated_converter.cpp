/*
    MAB Robotics

    Power Distribution System Example

    Isolated converter
*/

#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

constexpr socketIndex_E ISOLATED_CONVERTER_SOCKET_INDEX = socketIndex_E::SOCKET_2;

int main()
{
    Logger _log;
    _log.m_tag = "PDS Example";

    auto candle =
        mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M, mab::candleTypes::busTypes_t::USB);

    Pds pds(PDS_CAN_ID, candle);

    if (pds.init() != PdsModule::error_E::OK)
    {
        _log.error("No PDSs found!");
        return EXIT_FAILURE;
    }

    auto isolatedConverter = pds.attachIsolatedConverter(ISOLATED_CONVERTER_SOCKET_INDEX);

    if (isolatedConverter == nullptr)
        exit(EXIT_FAILURE);

    // Configuration
    isolatedConverter->setTemperatureLimit(70.0f);  // 70 Celsius
    isolatedConverter->setOcdLevel(4000);           // 4 A (4000 mA)

    isolatedConverter->enable();

    sleep(1);  // Allow time for converter to stabilize

    // Variables for status
    bool enabled          = false;
    u32  outputVoltage    = 0;  // mV
    s32  loadCurrent      = 0;  // mA
    f32  temperature      = 0;  // Celsius
    u32  ocdLevel         = 0;  // mA
    u32  ocdDelay         = 0;  // µs
    f32  temperatureLimit = 0;  // Celsius

    // Read status
    isolatedConverter->getEnabled(enabled);
    isolatedConverter->getOutputVoltage(outputVoltage);
    isolatedConverter->getLoadCurrent(loadCurrent);
    isolatedConverter->getTemperature(temperature);
    isolatedConverter->getOcdLevel(ocdLevel);
    isolatedConverter->getOcdDelay(ocdDelay);
    isolatedConverter->getTemperatureLimit(temperatureLimit);

    // Logging
    _log.info("Isolated converter");
    _log.info("Enabled :: [ %s ]", enabled ? "YES" : "NO");
    _log.info("Output voltage :: [ %.2f V ]", static_cast<float>(outputVoltage) / 1000.0f);
    _log.info("Load current :: [ %.2f A ]", static_cast<float>(loadCurrent) / 1000.0f);
    _log.info("Temperature :: [ %.2f °C ]", temperature);
    _log.info("OCD level :: [ %u mA ]", ocdLevel);
    _log.info("OCD delay :: [ %u µs ]", ocdDelay);
    _log.info("Temperature limit :: [ %.2f °C ]", temperatureLimit);

    isolatedConverter->disable();

    return EXIT_SUCCESS;
}
