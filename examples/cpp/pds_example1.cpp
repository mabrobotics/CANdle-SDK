/*
    MAB Robotics

    Power Distribution System Example 1

    This example simply Lists the number of particular
    pluggable modules connected to the PDS device

*/

#include "candle.hpp"
#include "pds.hpp"
#include "time.h"

using namespace mab;

int main()
{
    Logger log;
    log.m_tag = "PDS Example";

    std::shared_ptr<Candle> pCandle = std::make_shared<Candle>(mab::CAN_BAUD_1M, true);
    Pds                     pds(100, pCandle);

    Pds::modules_S pdsModules = {0};

    pds.getModules(pdsModules);

    log.info("PDS have the following numbers of connected modules:");
    log.info("\t PS V1 :: [ %u ]", pdsModules.powerStageV1);
    log.info("\t PS V2 :: [ %u ]", pdsModules.powerStageV2);
    log.info("\t IC 12 :: [ %u ]", pdsModules.isolatedConv12V);
    log.info("\t PS 5  :: [ %u ]", pdsModules.isolatedConverter5V);
    log.info("\t BR    :: [ %u ]", pdsModules.brakeResistor);

    return EXIT_SUCCESS;
}