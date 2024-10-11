/*
    MAB Robotics

    Power Distribution System Example 3

    This example shows the usage of Power Stage module.
    It assumes that at least one Power Stage module is connected
    to the control board.

*/

#include "candle.hpp"
#include "pds.hpp"
#include "time.h"

using namespace mab;

/*
Regardless that the software is detecting what modules are
connected to which sockets, user should now it explicitly too.
If user is using more then one module of the same type, the particular sockets
should be known to properly assign the module pointers when using the "<module>Attach" method.
*/
constexpr socketIndex_E POWER_STAGE_SOCKET_INDEX = socketIndex_E::SOCKET_1;

int main()
{
    Logger log;
    log.m_tag = "PDS Example";

    u32 vbusVoltage = 0;

    std::shared_ptr<Candle> pCandle = std::make_shared<Candle>(mab::CAN_BAUD_1M, true);
    Pds                     pds(100, pCandle);

    Pds::modules_S pdsModules = {0};

    PdsModule::error_E result = PdsModule::error_E::OK;

    pds.getModules(pdsModules);

    log.info("PDS have the following numbers of connected modules:");
    log.info("\t PS    :: [ %u ]", pdsModules.powerStage);
    log.info("\t IC 12 :: [ %u ]", pdsModules.isolatedConv12V);
    log.info("\t PS 5  :: [ %u ]", pdsModules.isolatedConverter5V);
    log.info("\t BR    :: [ %u ]", pdsModules.brakeResistor);

    std::unique_ptr<PowerStage> p_powerStage = pds.attachPowerStage(POWER_STAGE_SOCKET_INDEX);
    if (p_powerStage == nullptr)
    {
        log.error("Unable to attach power stage module!");
        exit(EXIT_FAILURE);
    }

    result = p_powerStage->enable();
    if (result != PdsModule::error_E::OK)
    {
        log.error("Power stage enable error [ %u ]", static_cast<u8>(result));
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        usleep(100000);
        result = p_powerStage->getOutputVoltage(vbusVoltage);
        if (result != PdsModule::error_E::OK)
            log.error("Reading VBus voltage error [ %u ]", static_cast<u8>(result));
        else
            log.info("Power stage VBus voltage [ %u V ]", vbusVoltage);
    }

    return EXIT_SUCCESS;
}