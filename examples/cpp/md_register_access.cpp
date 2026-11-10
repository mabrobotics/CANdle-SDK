#include "candle.hpp"
#include "MD.hpp"

int main()
{
    Logger log(Logger::ProgramLayer_E::TOP, "User Program");

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
    md.m_timeout = 2 /*ms*/;  // sometimes reading registers can take more than default 1ms timeout

    // Buffer holding all MD registers, each entry knows its address and type
    mab::MDRegisters_S regs;
    mab::MD::Error_t   err = mab::MD::Error_t::OK;

    // 1. Read an integer register
    err = md.readRegisters(regs.motorPolePairs);
    if (err != mab::MD::Error_t::OK)
    {
        log.error("Error reading motorPolePairs: %u", (u8)err);
        mab::detachCandle(candle);
        return EXIT_FAILURE;
    }
    log.info("Motor pole pairs: %u", regs.motorPolePairs.value);

    // 2. Read a float register
    err = md.readRegisters(regs.targetVelocity);
    if (err != mab::MD::Error_t::OK)
    {
        log.error("Error reading targetVelocity: %u", (u8)err);
        mab::detachCandle(candle);
        return EXIT_FAILURE;
    }
    const float originalTargetVelocity = regs.targetVelocity.value;
    log.info("Target velocity: %.3f rad/s", originalTargetVelocity);

    // 3. Write a float register. The drive is not enabled, so changing the target does not move
    // the motor.
    regs.targetVelocity = originalTargetVelocity + 1.0f;
    err                 = md.writeRegisters(regs.targetVelocity);
    if (err != mab::MD::Error_t::OK)
    {
        log.error("Error writing targetVelocity: %u", (u8)err);
        mab::detachCandle(candle);
        return EXIT_FAILURE;
    }
    log.info("Target velocity set to: %.3f rad/s", regs.targetVelocity.value);

    // 4. Read two registers in one request
    err = md.readRegisters(regs.mainEncoderPosition, regs.mainEncoderVelocity);
    if (err != mab::MD::Error_t::OK)
    {
        log.error("Error reading position and velocity: %u", (u8)err);
        mab::detachCandle(candle);
        return EXIT_FAILURE;
    }
    log.info("Position: %.4f rad | Velocity: %.4f rad/s",
             regs.mainEncoderPosition.value,
             regs.mainEncoderVelocity.value);

    // 5. Read three registers in one request, targetVelocity shows the value written in step 3
    err = md.readRegisters(regs.targetPosition, regs.targetVelocity, regs.targetTorque);
    if (err != mab::MD::Error_t::OK)
    {
        log.error("Error reading targets: %u", (u8)err);
        mab::detachCandle(candle);
        return EXIT_FAILURE;
    }
    log.info("Targets - position: %.3f | velocity: %.3f | torque: %.3f",
             regs.targetPosition.value,
             regs.targetVelocity.value,
             regs.targetTorque.value);

    // 6. Write three registers in one request, restoring the original target velocity
    regs.targetVelocity = originalTargetVelocity;
    err = md.writeRegisters(regs.targetPosition, regs.targetVelocity, regs.targetTorque);
    if (err != mab::MD::Error_t::OK)
    {
        log.error("Error writing targets: %u", (u8)err);
        mab::detachCandle(candle);
        return EXIT_FAILURE;
    }
    log.info("Targets restored, target velocity: %.3f rad/s", regs.targetVelocity.value);

    mab::detachCandle(candle);
    return EXIT_SUCCESS;
}
