/*
    MAB Robotics

    Power Distribution System Example 1

*/

#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

Logger _log;

std::unique_ptr<PowerStage>    p_powerStage    = nullptr;
std::unique_ptr<BrakeResistor> p_brakeResistor = nullptr;

/*
Regardless that the software is detecting what modules are
connected to which sockets, user should now it explicitly too.
If user is using more then one module of the same type, the particular sockets
should be known to properly assign the module pointers when using the "<module>Attach" method.
*/
constexpr socketIndex_E POWER_STAGE_SOCKET_INDEX    = socketIndex_E::SOCKET_1;
constexpr socketIndex_E BRAKE_RESISTOR_SOCKET_INDEX = socketIndex_E::SOCKET_2;

constexpr u16 PDS_CAN_ID = 100;

int main()
{
    _log.m_tag = "PDS Example";

    Candle candle(mab::CAN_BAUD_1M, true);
    Pds    pds(PDS_CAN_ID, candle);

    Pds::modulesSet_t pdsModules = pds.getModules();

    _log.info("PDS have the following numbers of connected modules:");
    _log.info("\t1\t:: %s", Pds::moduleTypeToString(pdsModules[0]));
    _log.info("\t2\t:: %s", Pds::moduleTypeToString(pdsModules[1]));
    _log.info("\t3\t:: %s", Pds::moduleTypeToString(pdsModules[2]));
    _log.info("\t4\t:: %s", Pds::moduleTypeToString(pdsModules[3]));
    _log.info("\t5\t:: %s", Pds::moduleTypeToString(pdsModules[4]));
    _log.info("\t6\t:: %s", Pds::moduleTypeToString(pdsModules[5]));

    return EXIT_SUCCESS;
}