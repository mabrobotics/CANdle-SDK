#include "candle.hpp"
#include "MD.hpp"

int main()
{
    auto candle =
        mab::attachCandle(mab::CANdleBaudrate_E::CAN_BAUD_1M, mab::candleTypes::busTypes_t::USB);

    constexpr mab::canId_t mdId = 100;

    mab::MD md(100, candle);
    if (md.init() != mab::MD::Error_t::OK)
    {
        std::cout << "MD not initialized\n";
    }
    md.m_timeout = 2 /*ms*/;
    mab::MDRegisters_S registerBuffer;

    std::cout << "ID: " << mdId << "\n";

    md.readRegisters(registerBuffer.motorName,
                     registerBuffer.canBaudrate,
                     registerBuffer.motorGearRatio,
                     registerBuffer.motorIMax);

    std::string canBaudrateString = registerBuffer.canBaudrate.value == 1'000'000   ? "1M\n"
                                    : registerBuffer.canBaudrate.value == 2'000'000 ? "2M\n"
                                    : registerBuffer.canBaudrate.value == 5'000'000 ? "5M\n"
                                    : registerBuffer.canBaudrate.value == 8'000'000 ? "8M\n"
                                                                                    : "UNKNOWN\n";

    std::cout << "Motor name: " << std::string(registerBuffer.motorName.value) << "\n"
              << "CAN baudrate: " << canBaudrateString
              << "Motor gear ratio: " << registerBuffer.motorGearRatio.value << "\n"
              << "Motor max current: " << registerBuffer.motorIMax.value;

    mab::detachCandle(candle);

    return EXIT_SUCCESS;
}