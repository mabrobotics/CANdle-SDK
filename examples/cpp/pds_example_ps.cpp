/*
    MAB Robotics

    Power Distribution System Example: Power stage

*/
#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

Logger _log;

int main()
{
    _log.m_tag = "PDS Example PS";
    Candle candle(mab::CAN_BAUD_1M, true);
    Pds    pds(PDS_CAN_ID, candle);

    auto powerStage1 = pds.attachPowerStage(socketIndex_E::SOCKET_3);

    u32 psV = 0;
    s32 psI = 0;

    powerStage1->bindBrakeResistor(socketIndex_E::SOCKET_4);
    powerStage1->setBrakeResistorTriggerVoltage(26000);
    powerStage1->setOcdDelay(2000);
    powerStage1->setOcdLevel(1000);
    powerStage1->enable();

    sleep(1);

    powerStage1->getOutputVoltage(psV);
    powerStage1->getLoadCurrent(psI);

    _log.info("V :: [ %u mV]", psV);
    _log.info("I :: [ %u mA]", psI);

    return EXIT_SUCCESS;
}
