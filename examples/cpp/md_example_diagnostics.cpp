#include "candle.hpp"
#include "MD.hpp"
#include "MDStatus.hpp"

int main()
{
    auto candle = mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M,
                                    mab::candleTypes::busTypes_t::USB);

    constexpr mab::canId_t mdId = 100;

    mab::MD md(100, candle);
    if (md.init() != mab::MD::Error_t::OK)
    {
        std::cout << "MD not initialized\n";
    }
    md.m_timeout = 2 /*ms*/;
    mab::MDRegisters_S registerBuffer;

    std::cout << "ID: " << mdId << "\n";

    mab::MD::Error_t err = md.readRegisters(registerBuffer.motorName,
                                            registerBuffer.canBaudrate,
                                            registerBuffer.motorGearRatio,
                                            registerBuffer.motorIMax);
    if (err != mab::MD::Error_t::OK)
    {
        std::cout << "Error reading registers: " << static_cast<u8>(err) << "\n";
        return EXIT_FAILURE;
    }

    std::string canDatarateString = registerBuffer.canBaudrate.value == 1'000'000   ? "1M\n"
                                    : registerBuffer.canBaudrate.value == 2'000'000 ? "2M\n"
                                    : registerBuffer.canBaudrate.value == 5'000'000 ? "5M\n"
                                    : registerBuffer.canBaudrate.value == 8'000'000 ? "8M\n"
                                                                                    : "UNKNOWN\n";

    std::cout << "Motor name: " << std::string(registerBuffer.motorName.value) << "\n"
              << "CAN datarate: " << canDatarateString
              << "Motor gear ratio: " << registerBuffer.motorGearRatio.value << "\n"
              << "Motor max current: " << registerBuffer.motorIMax.value;

    err = md.readRegisters(
        registerBuffer.quickStatus, registerBuffer.calibrationStatus, registerBuffer.motionStatus);
    if (err != mab::MD::Error_t::OK)
    {
        std::cout << "Error reading registers: " << static_cast<u8>(err) << "\n";
        return EXIT_FAILURE;
    }

    // Decode status bits
    mab::MDStatus statuses;
    mab::MDStatus::decode(registerBuffer.quickStatus.value, statuses.quickStatus);
    mab::MDStatus::decode(registerBuffer.calibrationStatus.value, statuses.calibrationStatus);
    mab::MDStatus::decode(registerBuffer.motionStatus.value, statuses.motionStatus);

    std::cout << "\nQuick status:\n";
    for (const auto& [bit, status] : statuses.quickStatus)
    {
        std::cout << " - " << status.name << ": " << (status.isSet() ? "SET" : "NOT SET") << "\n";
    }
    std::cout << "\nCalibration status:\n";
    for (const auto& [bit, status] : statuses.calibrationStatus)
    {
        std::cout << " - " << status.name << ": " << (status.isSet() ? "SET" : "NOT SET") << "\n";
    }
    std::cout << "\nMotion status:\n";
    for (const auto& [bit, status] : statuses.motionStatus)
    {
        std::cout << " - " << status.name << ": " << (status.isSet() ? "SET" : "NOT SET") << "\n";
    }

    mab::detachCandle(candle);

    return EXIT_SUCCESS;
}