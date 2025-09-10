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
    mab::Candle* candle = mab::attachCandle(
        mab::CANdleDatarate_E::CAN_DATARATE_1M, mab::candleTypes::busTypes_t::USB, true);

    // Look for MAB devices present on the can network
    auto ids = mab::MDCO::discoverOpenMDs(candle);

    if (ids.size() == 0)
    {
        log.error("No driver found with CANopen communication");
        return EXIT_SUCCESS;
    }

    i32 desiredSpeed = 0;
    i32 targetPos    = 1;
    i16 torque       = 0;
    f32 kp           = 10;
    f32 kd           = 1;

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
    MyMotorParam.MaxTorque = 2000;
    // Max speed is RPM
    MyMotorParam.MaxSpeed = 1000;
    // Max acceleration in RPM/s
    MyMotorParam.accLimit = 1000;
    // Max deceleration in RPM/s
    MyMotorParam.dccLimit = 1000;

    // Send the motor parameter to the MD
    mdco.setProfileParameters(MyMotorParam);

    // set the motor in the impedance mode
    mdco.enableDriver(Impedance);

    // kp
    u32 kp_bits;
    memcpy(&kp_bits, &(kp), sizeof(float));
    mdco.writeOpenRegisters("Kp_impedance", kp_bits);
    // kd
    u32 kd_bits;
    memcpy(&kd_bits, &(kd), sizeof(float));
    mdco.writeOpenRegisters("Kd_impedance", kd_bits);
    mdco.writeOpenRegisters("Position Demand Value", targetPos);
    mdco.writeOpenRegisters("Velocity Demand Value", desiredSpeed);
    mdco.writeOpenRegisters("Torque Demand Value", torque);
    auto start   = std::chrono::steady_clock::now();
    auto timeout = std::chrono::seconds((5));
    while (std::chrono::steady_clock::now() - start < timeout)
    {
        mdco.writeOpenRegisters("Torque Demand Value", torque);
    }
    mdco.writeOpenRegisters("Torque Demand Value", 0);
    mdco.disableDriver();

    return EXIT_SUCCESS;
}
