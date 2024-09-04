#include "candle.hpp"
#include "pds.hpp"
#include "time.h"

using namespace mab;
int main()
{
    // Create CANdle object and ping FDCAN bus in search of drives.
    // Any found drives will be printed out by the ping() method.

    std::shared_ptr<Candle> pCandle = std::make_shared<Candle>(mab::CAN_BAUD_1M, true);

    // std::vector<mab::BusDevice_S> busDevices =
    // pCandle->pingNew(mab::CANdleBaudrate_E::CAN_BAUD_1M);

    Pds examplePds(100, pCandle);

    while (1)
    {
        examplePds.getPdsInfo();
        usleep(500000);
    }

    return EXIT_SUCCESS;
}