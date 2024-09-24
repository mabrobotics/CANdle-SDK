/*
    MAB Robotics

    Power Distribution System Example 2

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
constexpr PdsModule::socket_E POWER_STAGE_SOCKET_INDEX = PdsModule::socket_E::SOCKET_2;

int main()
{
    logger log;
    log.tag = "PDS Example";

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

    std::unique_ptr<PowerStageV1> p_powerStage = pds.attachPowerStageV1(POWER_STAGE_SOCKET_INDEX);
    if (p_powerStage == nullptr)
        log.error("Unable to attach power stage module!");

    return EXIT_SUCCESS;
}