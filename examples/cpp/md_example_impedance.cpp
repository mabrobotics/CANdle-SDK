
#include "MD.hpp"
#include "candle_v2.hpp"
#include "USB_v2.hpp"
#include "I_communication_interface.hpp"

int main()
{
    Logger log(Logger::ProgramLayer_E::TOP, "User Program");
    Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_1;

    auto candle =
        mab::attachCandle(mab::CANdleBaudrate_E::CAN_BAUD_1M, mab::candleTypes::busTypes_t::USB);

    candle->discoverDevices();

    auto mdMap = candle->getMDmapHandle();

    if (mdMap->size() == 0)
    {
        log.error("No MDs found!");
        return EXIT_FAILURE;
    }

    mab::MD& md = mdMap->begin()->second;

    md.zero();
    md.setMotionMode(mab::Md80Mode_E::IMPEDANCE);
    md.enable();

    constexpr float stepSize       = 0.05f;
    float           targetPosition = 0.0f;

    for (u16 i = 0; i < 100; i++)
    {
        targetPosition += stepSize;
        md.setTargetPosition(targetPosition);
        ;
        if (i % 10 == 0)
            log.info("Target position: %.3f | Current position: %.3f",
                     targetPosition,
                     md.getPosition().first);
        usleep(20'000);
    }

    md.disable();
}
