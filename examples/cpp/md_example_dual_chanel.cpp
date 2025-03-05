#include "candle_v2.hpp"

int main()
{
    const std::string candleSerial1 = "2084377B4B31";
    const std::string candleSerial2 = "208337814B31";

    std::unique_ptr<mab::I_CommunicationInterface> usb1 = std::make_unique<mab::USBv2>(
        mab::CandleV2::CANDLE_VID, mab::CandleV2::CANDLE_PID, candleSerial1);
    usb1->connect();
    std::unique_ptr<mab::I_CommunicationInterface> usb2 = std::make_unique<mab::USBv2>(
        mab::CandleV2::CANDLE_VID, mab::CandleV2::CANDLE_PID, candleSerial2);
    usb2->connect();

    auto candle1 = mab::attachCandle(mab::CANdleBaudrate_E::CAN_BAUD_1M, std::move(usb1));
    auto candle2 = mab::attachCandle(mab::CANdleBaudrate_E::CAN_BAUD_1M, std::move(usb2));

    candle1->discoverDevices();
    candle2->discoverDevices();

    std::cout << "CANdle 1 devices:\n";
    for (auto& devPair : *(candle1->getMDmapHandle()))
    {
        std::cout << "    - " << (int)devPair.first << "\n";
    }

    std::cout << "CANdle 2 devices:\n";
    for (auto& devPair : *(candle2->getMDmapHandle()))
    {
        std::cout << "    - " << (int)devPair.first << "\n";
    }

    return EXIT_SUCCESS;
}