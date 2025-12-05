#include "candle.hpp"
#include "MDCO.hpp"

#define PIx2 6.28318530718f

using namespace mab;

int main()
{
    // Logger is a standalone builtin class for handling output from different modules
    Logger log(Logger::ProgramLayer_E::TOP, "User Program");

    // This parameters sets global internal logging level
    Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_1;

    // Attach Candle is an AIO method to get ready to use candle handle that corresponds to the real
    // CANdle USB-CAN converter. Its a main object so should have the longest lifetime of all
    // objects from the library.
    mab::Candle* candle = mab::attachCandle(
        mab::CANdleDatarate_E::CAN_DATARATE_1M, mab::candleTypes::busTypes_t::USB, true);

    // Look for MAB devices present on the can network
    auto ids = mab::MDCO::discoverOpenMDs(candle);

    if (ids.size() == 0)
    {
        log.error("No driver found with CANopen communication");
        return EXIT_SUCCESS;
    }

    // Get first md that was detected (the one with the lowest id).
    MDCO mdco(ids[0], candle);

    mdco.openZero();
    // set the motor in the impedance mode
    mdco.enableDriver(Impedance);

    static constexpr i32 countPerRotation = 16384;
    static constexpr f32 countToRadian    = PIx2 / countPerRotation;
    static constexpr f32 radianToCount    = 1.f / countToRadian;

    constexpr float stepSize       = 0.05f * radianToCount;
    float           targetPosition = 0.f;

    for (u16 i = 0; i < 100; i++)
    {
        targetPosition += stepSize;
        // Providing new target position
        mdco.writeOpenRegisters("Target Position", static_cast<i32>(targetPosition));

        if (i % 10 == 0)
        {
            i32 posRead = mdco.getValueFromOpenRegister(0x6064, 0x0);
            log.info("Target position: %.3f | Current position: %.3f",
                     targetPosition * countToRadian,
                     static_cast<f32>(posRead) * countToRadian /*Request positional data*/);
        }
        usleep(20'000);
    }

    mdco.disableDriver();

    return EXIT_SUCCESS;
}
