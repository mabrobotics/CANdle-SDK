/*
    MAB Robotics

    Power Distribution System Example 1

    Reading data from PDS Control board:
        * Connected submodules list
        * DC Bus voltage
*/
#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

int main()
{
    Logger _log;
    _log.m_tag = "PDS Example 1";

    Candle candle(mab::CAN_BAUD_2M, true);
    Pds    pds(PDS_CAN_ID, candle);

    Pds::modulesSet_S pdsModules = pds.getModules();

    u32 pdsBusVoltage = 0;
    pds.getBusVoltage(pdsBusVoltage);
    // PdsModule::error_E result        = pds.getBusVoltage(pdsBusVoltage);

    _log.info("PDS have the following numbers of connected modules:");
    _log.info("\t1\t:: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket1));
    _log.info("\t2\t:: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket2));
    _log.info("\t3\t:: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket3));
    _log.info("\t4\t:: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket4));
    _log.info("\t5\t:: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket5));
    _log.info("\t6\t:: %s", Pds::moduleTypeToString(pdsModules.moduleTypeSocket6));

    _log.info("Bus voltage: %0.2f", pdsBusVoltage / 1000.0f);

    return EXIT_SUCCESS;
}