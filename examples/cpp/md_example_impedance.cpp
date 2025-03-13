
#include "candle_v2.hpp"

int main()
{
    // Logger is a standalone builtin class for handling output from different modules
    Logger log(Logger::ProgramLayer_E::TOP, "User Program");

    // This parameters sets global internal logging level
    Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_1;

    // Attach Candle is an AIO method to get ready to use candle handle that corresponds to the real
    // CANdle USB-CAN converter. Its a main object so should have the longest lifetime of all
    // objects from the library.
    auto candle =
        mab::attachCandle(mab::CANdleBaudrate_E::CAN_BAUD_1M, mab::candleTypes::busTypes_t::USB);

    // Look for MAB devices present on the can network
    candle->discoverDevices();

    // This method provides you with memory-safe map handle which you can assign MDs too manually
    // via addMD method or via discoverDevices
    auto mdMap = candle->getMDmapHandle();

    if (mdMap->size() == 0)
    {
        log.error("No MDs found!");
        return EXIT_FAILURE;
    }

    // Get first md that was detected. This is not a memory safe handle so it should not be moved a
    // lot in this form.
    mab::MD& md = mdMap->begin()->second;

    // Zero out the position of the drive
    md.zero();

    // Selecting motion mode to be impedance. Important note is that motion mode resets every time
    // MD is disabled or timed out
    md.setMotionMode(mab::Md80Mode_E::IMPEDANCE);

    // Enable the drive. From this point on it is our objective to send regular messages to the MD,
    // otherwise it will timeout via watchdog and disable itself.
    md.enable();

    constexpr float stepSize       = 0.05f;
    float           targetPosition = 0.0f;

    for (u16 i = 0; i < 100; i++)
    {
        targetPosition += stepSize;
        // Providing new target position
        md.setTargetPosition(targetPosition);
        ;
        if (i % 10 == 0)
            log.info("Target position: %.3f | Current position: %.3f",
                     targetPosition,
                     md.getPosition().first /*Request positional data*/);
        usleep(20'000);
    }

    md.disable();
    return EXIT_SUCCESS;
}
