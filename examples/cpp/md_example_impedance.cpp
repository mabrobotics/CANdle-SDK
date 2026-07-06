
#include "candle.hpp"
#include "MD.hpp"

int main()
{
    // Logger is a standalone builtin class for handling output from different modules
    Logger log(Logger::ProgramLayer_E::TOP, "User Program");

    mab::Candle* candle = mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M,
                                            mab::candleTypes::busTypes_t::USB);

    // Get first md that was detected. This is not a memory safe handle so it should not be moved a
    // lot in this form.
    mab::MD md(100, candle);

    // Selecting motion mode to be impedance. Important note is that motion mode resets every time
    // MD is disabled or timed out
    md.setMotionMode(mab::MdMode_E::IMPEDANCE);
    md.setImpedanceParams(0., 0.);

    sleep(3);

    // Enable the drive. From this point on it is our objective to send regular messages to the MD,
    // otherwise it will timeout via watchdog and disable itself.
    md.enable();

    f32 targetTrq = 0.;
    f32 time = 0.f;
    while(1)
    {
        if (time < 3.f)
            targetTrq += 0.01f;
        if (time > 6.f)
            targetTrq -= 0.01f;
        if (targetTrq > 1.f)
            targetTrq = 1.f;
        if (targetTrq < 0.)
            break;
        md.setTargetTorque(targetTrq);
        usleep(10'000);
        time += 0.010;
    }

    md.disable();
    mab::detachCandle(candle);
    return EXIT_SUCCESS;
}
