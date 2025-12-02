#include "candle.hpp"
#include "MD.hpp"

int main()
{
    auto candle = mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M,
                                    mab::candleTypes::busTypes_t::USB);

    constexpr mab::canId_t MD_ID = 100;  // DEFINE YOUR MD CAN ID HERE

    mab::MD md(MD_ID, candle);
    if (md.init() != mab::MD::Error_t::OK)
    {
        std::cout << "MD not initialized\n";
    }
    md.m_timeout = 2 /*ms*/;  // sometimes reading registers can take more than default 1ms timeout

    // Create buffer to hold registers
    mab::MDRegisters_S registerBuffer;

    // Read register with motor name
    mab::MD::Error_t err = md.readRegisters(registerBuffer.motorName);

    auto previousMotorName = registerBuffer.motorName;
    if (err != mab::MD::Error_t::OK)
    {
        std::cout << "Error reading registers: " << static_cast<u8>(err) << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "Motor name: " << std::string(registerBuffer.motorName.value) << "\n";

    // Change motor name
    const char newName[24]   = "MY_MOTOR";
    registerBuffer.motorName = newName;

    err = md.writeRegisters(registerBuffer.motorName);

    // Read back motor name to verify it was changed
    // and read the position at the same time
    err = md.readRegisters(registerBuffer.motorName, registerBuffer.mainEncoderPosition);
    if (err != mab::MD::Error_t::OK)
    {
        std::cout << "Error reading registers: " << static_cast<u8>(err) << "\n";
        return EXIT_FAILURE;
    }
    std::cout << "New motor name: " << std::string(registerBuffer.motorName.value) << "\n"
              << "Position: " << std::dec << std::fixed << std::setprecision(4)
              << registerBuffer.mainEncoderPosition.value << "\n";

    // Now write back the original motor name, and blink the motor
    registerBuffer.motorName = previousMotorName;
    registerBuffer.runBlink  = 1;  // set blink request
    err                      = md.writeRegisters(registerBuffer.motorName, registerBuffer.runBlink);
    if (err != mab::MD::Error_t::OK)
    {
        std::cout << "Error writing registers: " << static_cast<u8>(err) << "\n";
        return EXIT_FAILURE;
    }

    mab::detachCandle(candle);

    return EXIT_SUCCESS;
}