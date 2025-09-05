#include "candle.hpp"
#include "MDCO.hpp"

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
    mab::Candle* candle = mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M,
                                            mab::candleTypes::busTypes_t::USB);

    // Look for MAB devices present on the can network
    auto ids = mab::MDCO::discoverOpenMDs(candle);

    if (ids.size() == 0)
    {
        log.error("No driver found with CANopen communication");
        return EXIT_SUCCESS;
    }

    // Get first md that was detected (the one with the lowest id).
    MDCO mdco(ids[0], candle);

    // Enter the limit of your motor
    moveParameter MyMotorParam;
    // Rated Current is in mA
    MyMotorParam.RatedCurrent = 1000;
    // Max current is in Rated Current *10^(-3)
    MyMotorParam.MaxCurrent = 500;
    // Rated torque is in mN*m
    MyMotorParam.RatedTorque = 1000;
    // Max torque is in rated torque *10^(-3)
    MyMotorParam.MaxTorque = 500;
    // Max speed is RPM
    MyMotorParam.MaxSpeed = 1000;

    // Send the motor parameter to the MD
    mdco.setProfileParameters(MyMotorParam);

    // set the motor in the cyclic velocity mode
    mdco.enableDriver(CyclicSyncVelocity);

    // loop parameter
    auto start      = std::chrono::steady_clock::now();
    auto lastSend   = start;
    auto timeout    = std::chrono::seconds(5);
    auto sendPeriod = std::chrono::milliseconds(10);

    while (std::chrono::steady_clock::now() - start < timeout)
    {
        auto now = std::chrono::steady_clock::now();
        if (now - lastSend >= sendPeriod)
        {
            mdco.writeOpenRegisters("Motor Target Velocity", 20);
            lastSend = now;
        }
    }

    // enter idle mode and leave Enable operation state
    mdco.disableDriver();

    return EXIT_SUCCESS;
}
