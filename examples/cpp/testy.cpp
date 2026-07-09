#include "candle.hpp"
#include "MD.hpp"
#include "logger.hpp"

#include <vector>
#include <thread>
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

int main(int argc, char** argv)
{
    Logger log(Logger::ProgramLayer_E::TOP, "User Program");
    // Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_1;

    auto candle = mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M,
                                    mab::candleTypes::busTypes_t::USB);

    mab::MDRegisters_S regs;
    mab::MD            md(100, candle);

    if (md.init() != mab::MD::Error_t::OK)
    {
        std::cout << "MD not initialized\n";
    }

    md.zero();

    if (md.setMotionMode(mab::MdMode_E::POSITION_PROFILE) != mab::MD::Error_t::OK)
    {
        std::cout << "MD mode setting failed \n";
    }

    md.enable();

    // md.setVelocityPIDparam(0.1, 0.3, 0.0, 1.5);
    // md.setPositionPIDparam(0.1, 0.0, 0.0, 0.0);

    // ori
    md.setVelocityPIDparam(0.05, 0.5, 0.0, 1.5);
    md.setPositionPIDparam(20.0, 0.5, 0.0, 15.0);

    md.setImpedanceParams(0.8, 0.015);

    // md.setTargetVelocity(70.0f);
    bool run = false;

    int   i   = 0;
    float sum = 0.0f;

    while (run)
    {
        md.setTargetVelocity(70.f);
        i += 1;
        if (md.readRegisters(regs.mainEncoderPosition, regs.mainEncoderVelocity) !=
            mab::MD::Error_t::OK)
        {
            log.error("Error reading encoder data!");
            run = false;
        }
        else
        {
            // log.info("MD %u", md.m_canId);
            // log.info("Position: %.4f", regs.mainEncoderPosition.value);
            // log.info("Velocity: %.4f\n", regs.mainEncoderVelocity.value);
            sum += regs.mainEncoderVelocity.value;
            if (i % 50 == 0)
            {
                log.info("MD %u", md.m_canId);
                log.info("Velocity: %.4f\n", sum / 50.0f);
                log.info("----------------------------");
                sum = 0.0f;
            }
        }
        // log.info("----------------------------");

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // constexpr float stepSize       = 1.f;
    float targetPosition = 800.f;
    md.setTargetPosition(targetPosition);
    md.setTargetVelocity(20.f);
    md.setProfileDeceleration(1.5f);
    md.setProfileAcceleration(1.5f);

    // for (float i = 0.0f; i < targetPosition; i += stepSize)
    // {
    //     // Providing new target position
    //     // md.setTargetPosition(i);

    //     log.info("Target position: %.3f | Current position: %.3f",
    //              i,
    //              md.getPosition().first /*Request positional data*/);

    //     usleep(20'000);
    //     if (i > 800.f)
    //     {
    //         md.setTargetVelocity(0.f);
    //         break;
    //     }
    // }

    float max_overshoot = 0.f;

    while (1)
    {
        if (md.getPosition().first > max_overshoot)
        {
            max_overshoot = md.getPosition().first;
        }

        log.info("Target position: %.3f | Current position: %.3f | Maximum overshoot: %.3f",
                 targetPosition,
                 md.getPosition().first /*Request positional data*/,
                 max_overshoot);
    }

    return EXIT_SUCCESS;
}