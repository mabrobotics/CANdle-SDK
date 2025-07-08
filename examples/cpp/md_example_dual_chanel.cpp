#include "candle.hpp"
#include "MD.hpp"

int main()
{
    const std::string candleSerial1 = "ENTER YOUR FIRST CANDLE SERIAL NUMBER";
    const std::string candleSerial2 = "ENTER YOUR SECOND CANDLE SERIAL NUMBER";

    std::unique_ptr<mab::I_CommunicationInterface> usb1 =
        std::make_unique<mab::USB>(mab::Candle::CANDLE_VID, mab::Candle::CANDLE_PID, candleSerial1);
    usb1->connect();
    std::unique_ptr<mab::I_CommunicationInterface> usb2 =
        std::make_unique<mab::USB>(mab::Candle::CANDLE_VID, mab::Candle::CANDLE_PID, candleSerial2);
    usb2->connect();

    auto candle1 = mab::attachCandle(mab::CANdleBaudrate_E::CAN_BAUD_1M, std::move(usb1));
    auto candle2 = mab::attachCandle(mab::CANdleBaudrate_E::CAN_BAUD_1M, std::move(usb2));

    auto ids1 = mab::MD::discoverMDs(candle1);
    auto ids2 = mab::MD::discoverMDs(candle2);

    std::cout << "CANdle 1 devices:\n";
    for (const auto& id : ids1)
    {
        std::cout << "    - " << id << "\n";
    }

    std::cout << "CANdle 2 devices:\n";
    for (const auto& id : ids2)
    {
        std::cout << "    - " << id << "\n";
    }

    mab::detachCandle(candle1);
    mab::detachCandle(candle2);

    return EXIT_SUCCESS;
}