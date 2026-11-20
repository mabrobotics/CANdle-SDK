#include "candle.hpp"
#include "MD.hpp"

int main()
{
    // Logger is a standalone builtin class for handling output from different modules
    Logger log(Logger::ProgramLayer_E::TOP, "User Program");

    // This parameters sets global internal logging level
    Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_1;

    // Attach Candle is an AIO method to get ready to use candle handle that corresponds to the real
    // CANdle USB-CAN converter. Its a main object so should have the longest lifetime of all
    // objects from the library.
    mab::Candle* candle = mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M,
                                            mab::candleTypes::busTypes_t::USB);

    // Look for MAB devices present on the can network, stop at the first one (only one is used)
    auto ids = mab::MD::discoverMDs(candle, 10, 0x7FF, 1);
    if (ids.empty())
    {
        log.error("No MDs found!");
        mab::detachCandle(candle);
        return EXIT_FAILURE;
    }

    mab::MD md(ids[0], candle);
    if (md.init() != mab::MD::Error_t::OK)
    {
        log.error("Could not initialize MD %u!", ids[0]);
        mab::detachCandle(candle);
        return EXIT_FAILURE;
    }

    // Read the current position, the movement is planned relative to it
    auto startPosition = md.getPosition();
    if (startPosition.second != mab::MD::Error_t::OK)
    {
        log.error("Could not read MD position!");
        mab::detachCandle(candle);
        return EXIT_FAILURE;
    }
    log.info("Start position: %.3f rad", startPosition.first);

    // Selecting motion mode to be impedance. Important note is that motion mode resets every time
    // MD is disabled or timed out
    md.setMotionMode(mab::MdMode_E::IMPEDANCE);

    // Hold the current position first, so the drive does not jump to a stale target when enabled
    md.setTargetPosition(startPosition.first);

    // Enable the drive. From this point on it is our objective to send regular messages to the MD,
    // otherwise it will timeout via watchdog and disable itself.
    md.enable();

    constexpr float distance = 5.0f;  // rad, relative to the start position
    constexpr u16   steps    = 100;   // one step every 20 ms -> 2 s movement

    for (u16 i = 1; i <= steps; i++)
    {
        float targetPosition = startPosition.first + distance * i / steps;
        // Providing new target position
        md.setTargetPosition(targetPosition);
        if (i % 10 == 0)
            log.info("Target position: %.3f | Current position: %.3f",
                     targetPosition,
                     md.getPosition().first /*Request positional data*/);
        usleep(20'000);
    }

    md.disable();
    mab::detachCandle(candle);
    return EXIT_SUCCESS;
}
