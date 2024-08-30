#include "candle.hpp"

int main()
{
    // Create CANdle object and ping FDCAN bus in search of drives.
    // Any found drives will be printed out by the ping() method.

    mab::Candle                   candle(mab::CAN_BAUD_1M, true);
    std::vector<mab::BusDevice_S> busDevices = candle.pingNew(mab::CANdleBaudrate_E::CAN_BAUD_1M);

    return EXIT_SUCCESS;
}