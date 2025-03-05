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

    // Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;
    std::cout << "Statring diagnostics\n"
              << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

    auto& md     = (*mdMap).at(mdId);
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

    return EXIT_SUCCESS;
}