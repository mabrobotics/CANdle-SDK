#include <span>
#include "Candle.hpp"
#include "CandleInterface.hpp"
#include "UsbHandler.hpp"

int main(int argc, char **argv)
{
    std::unique_ptr<IBusHandler> busHandler = std::make_unique<UsbHandler>();
    std::unique_ptr<ICommunication> candleInterface = std::make_unique<CandleInterface>(busHandler.get());
    Candle candle(candleInterface.get());

    if (!candle.init(Candle::Baud::BAUD_1M))
    {
        return 0;
    }

    auto drives = candle.ping();
    std::cout << "Found drives: " << std::endl;

    for (auto &md80 : drives)
    {
        std::cout << md80 << std::endl;
    }

    std::cout << " --------------------- " << std::endl;

    candle.enterOperational(1);
    candle.setModeOfOperation(1, Candle::ModesOfOperation::PROFILE_POSITION);
    candle.setTargetPosition(1, 10000.0f);

    std::this_thread::sleep_for(std::chrono::seconds(3));

    candle.setTargetPosition(1, 0.0f);

    uint32_t status{};
    for (int i = 1; i < 9; i++)
    {
        candle.canopenStack->readSDO(1, 0x2004, i, status);
        std::cout << std::hex << status << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    candle.enterSwitchOnDisabled(1);

    candle.deInit();

    return 0;
}
