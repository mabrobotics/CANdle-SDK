#include "candle.hpp"
#include "MD.hpp"

int main()
{
    mab::Candle* candle = mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M,
                                            mab::candleTypes::busTypes_t::USB);

    constexpr mab::canId_t mdId = 100;
    mab::MD                md(mdId, candle);

    if (md.init() != mab::MD::Error_t::OK)
    {
        std::cout << "MD failed to be added!\n";
        return EXIT_FAILURE;
    }

    md.testLatency();

    mab::detachCandle(candle);

    return EXIT_SUCCESS;
}