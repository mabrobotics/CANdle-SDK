#include "MD.hpp"

namespace mab
{

    MD::Error_t MD::init()
    {
        // TODO: add new hw struct support
        //  m_mdRegisters.hardwareType.value.deviceType = deviceType_E::UNKNOWN_DEVICE;
        //  auto mfDataResult                           = readRegister(m_mdRegisters.hardwareType);

        // if (mfDataResult.second == Error_t::OK)
        // {
        //     m_mdRegisters.hardwareType = mfDataResult.first;

        //     auto devType = m_mdRegisters.hardwareType.value.deviceType;

        //     if (devType != deviceType_E::UNKNOWN_DEVICE)
        //         return Error_t::OK;
        // }
        m_mdRegisters.legacyHardwareVersion = 0;

        auto mfLegacydataResult = readRegister(m_mdRegisters.legacyHardwareVersion);

        if (mfLegacydataResult.second != Error_t::OK)
            return Error_t::NOT_CONNECTED;

        if (m_mdRegisters.legacyHardwareVersion.value != 0)
            return Error_t::OK;

        return Error_t::NOT_CONNECTED;
    }

    MD::Error_t MD::blink()
    {
        m_mdRegisters.runBlink = 1;

        auto result = writeRegisters(m_mdRegisters.runBlink);
        if (result != Error_t::OK)
        {
            m_log.error("Blink failed!");
            return result;
        }
        return MD::Error_t::OK;
    }
    MD::Error_t MD::enable()
    {
        m_mdRegisters.state = 39;
        if (writeRegisters(m_mdRegisters.state) != MD::Error_t::OK)
        {
            m_log.error("Enabling failed");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info("Driver enabled");
        m_mdRegisters.motionModeStatus = 0;
        if (readRegisters(m_mdRegisters.motionModeStatus).second != MD::Error_t::OK)
        {
            m_log.error("Motion status check failed");
            return MD::Error_t::TRANSFER_FAILED;
        }
        if (m_mdRegisters.motionModeStatus.value == mab::Md80Mode_E::IDLE)
            m_log.warn("Motion mode not set");

        return MD::Error_t::OK;
    }
    MD::Error_t MD::disable()
    {
        m_mdRegisters.state = 64;
        if (writeRegisters(m_mdRegisters.state) != MD::Error_t::OK)
        {
            m_log.error("Disabling failed");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info("Driver disabled");
        return MD::Error_t::OK;
    }

    MD::Error_t MD::reset()
    {
        m_mdRegisters.runReset = 0x1;
        if (writeRegisters(m_mdRegisters.runReset) != MD::Error_t::OK)
        {
            m_log.error("Reset failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info("Driver reset");
        return MD::Error_t::OK;
    }

    MD::Error_t MD::clearErrors()
    {
        m_mdRegisters.runClearErrors = 0x1;
        if (writeRegisters(m_mdRegisters.runClearErrors) != MD::Error_t::OK)
        {
            m_log.error("Clear errors failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info("Driver errors are cleared");
        return MD::Error_t::OK;
    }

    MD::Error_t MD::save()
    {
        m_mdRegisters.runSaveCmd = 0x1;
        if (writeRegisters(m_mdRegisters.runSaveCmd) != MD::Error_t::OK)
        {
            m_log.error("Save failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info("Driver config saved");
        return MD::Error_t::OK;
    }

    MD::Error_t MD::zero()
    {
        m_mdRegisters.runZero = 0x1;
        if (writeRegisters(m_mdRegisters.runZero) != MD::Error_t::OK)
        {
            m_log.error("Zeroing failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info("Driver position is zeroed");
        return MD::Error_t::OK;
    }

    MD::Error_t MD::setCurrentLimit(float currentLimit /*A*/)
    {
        m_mdRegisters.motorIMax = currentLimit;
        if (writeRegisters(m_mdRegisters.motorIMax))
        {
            m_log.error("Current limit setting failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info("Current limit set to value %.2f", currentLimit);
        return MD::Error_t::OK;
    }

    MD::Error_t MD::setTorqueBandwidth(u16 torqueBandwidth /*Hz*/)
    {
        m_mdRegisters.motorTorqueBandwidth = torqueBandwidth;
        if (writeRegisters(m_mdRegisters.motorTorqueBandwidth))
        {
            m_log.error("Torque bandwidth setting failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info(" set to value %.2f", torqueBandwidth);
        return MD::Error_t::OK;
    }

    MD::Error_t MD::setMotionMode(mab::Md80Mode_E mode)
    {
        m_mdRegisters.motionModeCommand = mode;
        if (writeRegisters(m_mdRegisters.motionModeCommand))
        {
            m_log.error("setting failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        std::string modeDisplay;
        switch (mode)
        {
            case mab::Md80Mode_E::IDLE:
                modeDisplay = "IDLE";
                break;
            case mab::Md80Mode_E::IMPEDANCE:
                modeDisplay = "IMPEDANCE";
                break;
            case mab::Md80Mode_E::POSITION_PID:
                modeDisplay = "POSITION PID";
                break;
            case mab::Md80Mode_E::POSITION_PROFILE:
                modeDisplay = "POSITION PROFILE";
                break;
            case mab::Md80Mode_E::RAW_TORQUE:
                modeDisplay = "RAW TORQUE";
                break;
            case mab::Md80Mode_E::VELOCITY_PID:
                modeDisplay = "VELOCITY PID";
                break;
            case mab::Md80Mode_E::VELOCITY_PROFILE:
                modeDisplay = "VELOCITY PROFILE";
                break;
            default:
                modeDisplay = "UNKOWN";
                break;
        }
        m_log.info("Motion mode set to %s", modeDisplay.c_str());
        return MD::Error_t::OK;
    }

    MD::Error_t MD::setPositionPIDparam(float kp, float ki, float kd, float integralMax)
    {
        m_mdRegisters.motorPosPidKp     = kp;
        m_mdRegisters.motorPosPidKi     = ki;
        m_mdRegisters.motorPosPidKd     = kd;
        m_mdRegisters.motorPosPidWindup = integralMax;
        if (writeRegisters(m_mdRegisters.motorPosPidKp,
                           m_mdRegisters.motorPosPidKi,
                           m_mdRegisters.motorPosPidKd,
                           m_mdRegisters.motorPosPidWindup))
        {
            m_log.error("Position PID setting failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info("Position pid set to values: kp - %.2f, ki - %.2f, kd - %.2f, max - %.2f",
                   kp,
                   ki,
                   kd,
                   integralMax);
        return MD::Error_t::OK;
    }

    MD::Error_t MD::setVelocityPIDparam(float kp, float ki, float kd, float integralMax)
    {
        m_mdRegisters.motorVelPidKp     = kp;
        m_mdRegisters.motorVelPidKi     = ki;
        m_mdRegisters.motorVelPidKd     = kd;
        m_mdRegisters.motorVelPidWindup = integralMax;
        if (writeRegisters(m_mdRegisters.motorVelPidKp,
                           m_mdRegisters.motorVelPidKi,
                           m_mdRegisters.motorVelPidKd,
                           m_mdRegisters.motorVelPidWindup))
        {
            m_log.error("Velocity PID setting failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info("Velocity pid set to values: kp - %.2f, ki - %.2f, kd - %.2f, max - %.2f",
                   kp,
                   ki,
                   kd,
                   integralMax);
        return MD::Error_t::OK;
    }

    MD::Error_t MD::setImpedanceParams(float kp, float kd)
    {
        m_mdRegisters.motorImpPidKp = kp;
        m_mdRegisters.motorImpPidKd = kd;
        if (writeRegisters(m_mdRegisters.motorImpPidKp, m_mdRegisters.motorImpPidKd))
        {
            m_log.error("Impedance control parameters setting failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info("Impedance parameters set to value: kp - %.2f, kd - %.2f", kp, kd);
        return MD::Error_t::OK;
    }

    MD::Error_t MD::setMaxTorque(float maxTorque /*Nm*/)
    {
        m_mdRegisters.maxTorque = maxTorque;
        if (writeRegisters(m_mdRegisters.maxTorque))
        {
            m_log.error("Maximal torque setting failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info("Maximal torque set to value %.2f", maxTorque);
        return MD::Error_t::OK;
    }

    MD::Error_t MD::setProfileVelocity(float profileVelocity /*s^-1*/)
    {
        m_mdRegisters.profileVelocity = profileVelocity;
        if (writeRegisters(m_mdRegisters.profileVelocity))
        {
            m_log.error("Velocity for profiles setting failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info("Profile velocity set to value %.2f", profileVelocity);
        return MD::Error_t::OK;
    }

    MD::Error_t MD::setProfileAcceleration(float profileAcceleration /*s^-2*/)
    {
        m_mdRegisters.profileAcceleration = profileAcceleration;
        if (writeRegisters(m_mdRegisters.profileAcceleration))
        {
            m_log.error("Profile acceleration setting failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info("Profile acceleration set to value %.2f", profileAcceleration);
        return MD::Error_t::OK;
    }

    MD::Error_t MD::setTargetPosition(float position /*rad*/)
    {
        m_mdRegisters.targetPosition = position;
        if (writeRegisters(m_mdRegisters.targetPosition))
        {
            m_log.error("Target position setting failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.debug("Position target set to value %.2f", position);
        return MD::Error_t::OK;
    }

    MD::Error_t MD::setTargetVelocity(float velocity /*rad/s*/)
    {
        m_mdRegisters.targetVelocity = velocity;
        if (writeRegisters(m_mdRegisters.targetVelocity))
        {
            m_log.error("Velocity target setting failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.debug("Velocity target set to value %.2f", velocity);
        return MD::Error_t::OK;
    }

    MD::Error_t MD::setTargetTorque(float torque /*Nm*/)
    {
        m_mdRegisters.targetTorque = torque;
        if (writeRegisters(m_mdRegisters.targetTorque))
        {
            m_log.error("Torque target setting failed!");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.debug("Torque target set to value %.2f", torque);
        return MD::Error_t::OK;
    }

    std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, MD::Error_t>
    MD::getQuickStatus()
    {
        auto result = readRegister(m_mdRegisters.quickStatus);
        if (result.second != Error_t::OK)
        {
            m_log.error("Could not read quick status vector!");
            return std::make_pair(m_status.quickStatus, result.second);
        }
        MDStatus::toMap(m_mdRegisters.quickStatus.value, m_status.quickStatus);
        return std::make_pair(m_status.quickStatus, result.second);
    }

    std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, MD::Error_t>
    MD::getMainEncoderErrors()
    {
        auto result = readRegister(m_mdRegisters.mainEncoderErrors);
        if (result.second != Error_t::OK)
        {
            m_log.error("Could not read main encoder errors!");
            return std::make_pair(m_status.encoderError, result.second);
        }
        MDStatus::toMap(m_mdRegisters.mainEncoderErrors.value, m_status.encoderError);
        return std::make_pair(m_status.encoderError, result.second);
    }

    std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, MD::Error_t>
    MD::getOutputEncoderErrors()
    {
        auto result = readRegister(m_mdRegisters.outputEncoderErrors);
        if (result.second != Error_t::OK)
        {
            m_log.error("Could not read output encoder errors!");
            return std::make_pair(m_status.encoderError, result.second);
        }
        MDStatus::toMap(m_mdRegisters.outputEncoderErrors.value, m_status.encoderError);
        return std::make_pair(m_status.encoderError, result.second);
    }

    std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, MD::Error_t>
    MD::getCalibrationErrors()
    {
        auto result = readRegister(m_mdRegisters.calibrationErrors);
        if (result.second != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(m_status.calibrationError, result.second);
        }
        MDStatus::toMap(m_mdRegisters.calibrationErrors.value, m_status.calibrationError);
        return std::make_pair(m_status.calibrationError, result.second);
    }

    std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, MD::Error_t>
    MD::getBridgeErrors()
    {
        auto result = readRegister(m_mdRegisters.bridgeErrors);
        if (result.second != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(m_status.bridgeError, result.second);
        }
        MDStatus::toMap(m_mdRegisters.bridgeErrors.value, m_status.bridgeError);
        return std::make_pair(m_status.bridgeError, result.second);
    }

    std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, MD::Error_t>
    MD::getHardwareErrors()
    {
        auto result = readRegister(m_mdRegisters.hardwareErrors);
        if (result.second != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(m_status.hardwareError, result.second);
        }
        MDStatus::toMap(m_mdRegisters.hardwareErrors.value, m_status.hardwareError);
        return std::make_pair(m_status.hardwareError, result.second);
    }

    std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, MD::Error_t>
    MD::getCommunicationErrors()
    {
        auto result = readRegister(m_mdRegisters.communicationErrors);
        if (result.second != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(m_status.communicationError, result.second);
        }
        MDStatus::toMap(m_mdRegisters.communicationErrors.value, m_status.communicationError);
        return std::make_pair(m_status.communicationError, result.second);
    }

    std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, MD::Error_t>
    MD::getMotionErrors()
    {
        auto result = readRegister(m_mdRegisters.motionErrors);
        if (result.second != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(m_status.motionErrors, result.second);
        }
        MDStatus::toMap(m_mdRegisters.motionErrors.value, m_status.motionErrors);
        return std::make_pair(m_status.motionErrors, result.second);
    }

    std::pair<float, MD::Error_t> MD::getPosition()
    {
        auto result = readRegister(m_mdRegisters.mainEncoderPosition);
        if (result.second != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(0, result.second);
        }
        return std::make_pair(m_mdRegisters.mainEncoderPosition.value, result.second);
    }

    std::pair<float, MD::Error_t> MD::getVelocity()
    {
        auto result = readRegister(m_mdRegisters.mainEncoderVelocity);
        if (result.second != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(0, result.second);
        }
        return std::make_pair(m_mdRegisters.mainEncoderVelocity.value, result.second);
    }

    std::pair<float, MD::Error_t> MD::getTorque()
    {
        auto result = readRegister(m_mdRegisters.motorTorque);
        if (result.second != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(0, result.second);
        }
        return std::make_pair(m_mdRegisters.motorTorque.value, result.second);
    }

    std::pair<float, MD::Error_t> MD::getOutputEncoderPosition()
    {
        auto result = readRegister(m_mdRegisters.outputEncoderPosition);
        if (result.second != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(0, result.second);
        }
        return std::make_pair(m_mdRegisters.outputEncoderPosition.value, result.second);
    }

    std::pair<float, MD::Error_t> MD::getOutputEncoderVelocity()
    {
        auto result = readRegister(m_mdRegisters.outputEncoderVelocity);
        if (result.second != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(0, result.second);
        }
        return std::make_pair(m_mdRegisters.outputEncoderVelocity.value, result.second);
    }

    std::pair<u8, MD::Error_t> MD::getTemperature()
    {
        auto result = readRegister(m_mdRegisters.motorTemperature);
        if (result.second != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(0, result.second);
        }
        return std::make_pair(m_mdRegisters.motorTemperature.value, result.second);
    }

    /// @brief This test should be performed with 1M datarate on CAN network
    void MD::testLatency()
    {
        u64 latencyTransmit = 0;  // us
        // u64 latencyReceive  = 0;  // us

        constexpr u64 transmitSamples = 1000;
        // constexpr u64 receiveSamples  = 1000;

        constexpr u64 transmissionFramesTime = 407;
        // constexpr u64 receptionFramesTime    = 1;

        m_mdRegisters.userGpioConfiguration = 0;
        auto transmitParameter =
            std::make_tuple(std::reference_wrapper(m_mdRegisters.userGpioConfiguration));
        // auto receiveParameter =
        //     std::make_tuple(std::reference_wrapper(m_mdRegisters.canTermination));

        for (u32 i = 0; i < transmitSamples; i++)
        {
            auto start = std::chrono::high_resolution_clock::now();
            writeRegisters(transmitParameter);
            auto end = std::chrono::high_resolution_clock::now();
            latencyTransmit +=
                std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        }
        latencyTransmit /= transmitSamples;
        m_log.info("Overall transmission time: %d us", latencyTransmit);
        m_log.info("Overall transmission frequency: %.6f kHz",
                   1.0f / (static_cast<float>(latencyTransmit) / 1'000.0f));
        u64 latencyTransmitCropped = latencyTransmit - transmissionFramesTime;
        m_log.info("Only USB transmission time: %d us", latencyTransmitCropped);
        m_log.info("Can bus utilization %.2f%%",
                   100.0f - (static_cast<float>(latencyTransmitCropped) /
                             static_cast<float>(latencyTransmit)) *
                                100.0f);
    }
}  // namespace mab