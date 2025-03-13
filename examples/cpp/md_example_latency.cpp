#include "candle_v2.hpp"

int main()
{
    auto candle =
        mab::attachCandle(mab::CANdleBaudrate_E::CAN_BAUD_1M, mab::candleTypes::busTypes_t::USB);

    constexpr mab::canId_t mdId = 100;
    candle->addMD(mdId);

    auto mdMap = candle->getMDmapHandle();

    if (mdMap->size() == 0)
    {
        std::cout << "MD failed to be added!\n";
        return EXIT_FAILURE;
    }

    auto& md = (*mdMap).at(mdId);

    md.testLatency();

    return EXIT_SUCCESS;
}