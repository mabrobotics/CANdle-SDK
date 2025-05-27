#include "MD.hpp"

namespace mab
{

    MD::Error_t MD::init()
    {
        // TODO: add new hw struct support CS-36
        //  m_mdRegisters.hardwareType.value.deviceType = deviceType_E::UNKNOWN_DEVICE;
        //  auto mfDataResult                           = readRegister(m_mdRegisters.hardwareType);

        // if (mfDataresult == Error_t::OK)
        // {
        //     m_mdRegisters.hardwareType = mfDataResult.first;

        //     auto devType = m_mdRegisters.hardwareType.value.deviceType;

        //     if (devType != deviceType_E::UNKNOWN_DEVICE)
        //         return Error_t::OK;
        // }
        m_mdRegisters.legacyHardwareVersion = 0;

        auto mfLegacydataResult = readRegister(m_mdRegisters.legacyHardwareVersion);

        if (mfLegacydataResult != Error_t::OK)
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
        if (readRegisters(m_mdRegisters.motionModeStatus) != MD::Error_t::OK)
        {
            m_log.error("Motion status check failed");
            return MD::Error_t::TRANSFER_FAILED;
        }
        if (m_mdRegisters.motionModeStatus.value == mab::MdMode_E::IDLE)
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

    MD::Error_t MD::setMotionMode(mab::MdMode_E mode)
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
            case mab::MdMode_E::IDLE:
                modeDisplay = "IDLE";
                break;
            case mab::MdMode_E::IMPEDANCE:
                modeDisplay = "IMPEDANCE";
                break;
            case mab::MdMode_E::POSITION_PID:
                modeDisplay = "POSITION PID";
                break;
            case mab::MdMode_E::POSITION_PROFILE:
                modeDisplay = "POSITION PROFILE";
                break;
            case mab::MdMode_E::RAW_TORQUE:
                modeDisplay = "RAW TORQUE";
                break;
            case mab::MdMode_E::VELOCITY_PID:
                modeDisplay = "VELOCITY PID";
                break;
            case mab::MdMode_E::VELOCITY_PROFILE:
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

    std::pair<const std::unordered_map<const MDStatus::QuickStatusBits, MDStatus::StatusItem_S>,
              MD::Error_t>
    MD::getQuickStatus()
    {
        auto result = readRegister(m_mdRegisters.quickStatus);
        if (result != Error_t::OK)
        {
            m_log.error("Could not read quick status vector!");
            return std::make_pair(m_status.quickStatus, result);
        }
        MDStatus::toMap(m_mdRegisters.quickStatus.value, m_status.quickStatus);
        return std::make_pair(m_status.quickStatus, result);
    }

    std::pair<const std::unordered_map<const MDStatus::EncoderStatusBits, MDStatus::StatusItem_S>,
              MD::Error_t>
    MD::getMainEncoderStatus()
    {
        auto result = readRegister(m_mdRegisters.mainEncoderStatus);
        if (result != Error_t::OK)
        {
            m_log.error("Could not read main encoder errors!");
            return std::make_pair(m_status.encoderStatus, result);
        }
        MDStatus::toMap(m_mdRegisters.mainEncoderStatus.value, m_status.encoderStatus);
        return std::make_pair(m_status.encoderStatus, result);
    }

    std::pair<const std::unordered_map<const MDStatus::EncoderStatusBits, MDStatus::StatusItem_S>,
              MD::Error_t>
    MD::getOutputEncoderStatus()
    {
        auto result = readRegister(m_mdRegisters.auxEncoderStatus);
        if (result != Error_t::OK)
        {
            m_log.error("Could not read output encoder errors!");
            return std::make_pair(m_status.encoderStatus, result);
        }
        MDStatus::toMap(m_mdRegisters.auxEncoderStatus.value, m_status.encoderStatus);
        return std::make_pair(m_status.encoderStatus, result);
    }

    std::pair<
        const std::unordered_map<const MDStatus::CalibrationStatusBits, MDStatus::StatusItem_S>,
        MD::Error_t>
    MD::getCalibrationStatus()
    {
        auto result = readRegister(m_mdRegisters.calibrationStatus);
        if (result != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(m_status.calibrationStatus, result);
        }
        MDStatus::toMap(m_mdRegisters.calibrationStatus.value, m_status.calibrationStatus);
        return std::make_pair(m_status.calibrationStatus, result);
    }

    std::pair<const std::unordered_map<const MDStatus::BridgeStatusBits, MDStatus::StatusItem_S>,
              MD::Error_t>
    MD::getBridgeStatus()
    {
        auto result = readRegister(m_mdRegisters.bridgeStatus);
        if (result != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(m_status.bridgeStatus, result);
        }
        MDStatus::toMap(m_mdRegisters.bridgeStatus.value, m_status.bridgeStatus);
        return std::make_pair(m_status.bridgeStatus, result);
    }

    std::pair<const std::unordered_map<const MDStatus::HardwareStatusBits, MDStatus::StatusItem_S>,
              MD::Error_t>
    MD::getHardwareStatus()
    {
        auto result = readRegister(m_mdRegisters.hardwareStatus);
        if (result != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(m_status.hardwareStatus, result);
        }
        MDStatus::toMap(m_mdRegisters.hardwareStatus.value, m_status.hardwareStatus);
        return std::make_pair(m_status.hardwareStatus, result);
    }

    std::pair<
        const std::unordered_map<const MDStatus::CommunicationStatusBits, MDStatus::StatusItem_S>,
        MD::Error_t>
    MD::getCommunicationStatus()
    {
        auto result = readRegister(m_mdRegisters.communicationStatus);
        if (result != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(m_status.communicationStatus, result);
        }
        MDStatus::toMap(m_mdRegisters.communicationStatus.value, m_status.communicationStatus);
        return std::make_pair(m_status.communicationStatus, result);
    }

    std::pair<const std::unordered_map<const MDStatus::MotionStatusBits, MDStatus::StatusItem_S>,
              MD::Error_t>
    MD::getMotionStatus()
    {
        auto result = readRegister(m_mdRegisters.motionStatus);
        if (result != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(m_status.motionStatus, result);
        }
        MDStatus::toMap(m_mdRegisters.motionStatus.value, m_status.motionStatus);
        return std::make_pair(m_status.motionStatus, result);
    }

    std::pair<float, MD::Error_t> MD::getPosition()
    {
        auto result = readRegister(m_mdRegisters.mainEncoderPosition);
        if (result != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(0, result);
        }
        return std::make_pair(m_mdRegisters.mainEncoderPosition.value, result);
    }

    std::pair<float, MD::Error_t> MD::getVelocity()
    {
        auto result = readRegister(m_mdRegisters.mainEncoderVelocity);
        if (result != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(0, result);
        }
        return std::make_pair(m_mdRegisters.mainEncoderVelocity.value, result);
    }

    std::pair<float, MD::Error_t> MD::getTorque()
    {
        auto result = readRegister(m_mdRegisters.motorTorque);
        if (result != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(0, result);
        }
        return std::make_pair(m_mdRegisters.motorTorque.value, result);
    }

    std::pair<float, MD::Error_t> MD::getOutputEncoderPosition()
    {
        auto result = readRegister(m_mdRegisters.auxEncoderPosition);
        if (result != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(0, result);
        }
        return std::make_pair(m_mdRegisters.auxEncoderPosition.value, result);
    }

    std::pair<float, MD::Error_t> MD::getOutputEncoderVelocity()
    {
        auto result = readRegister(m_mdRegisters.auxEncoderVelocity);
        if (result != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(0, result);
        }
        return std::make_pair(m_mdRegisters.auxEncoderVelocity.value, result);
    }

    std::pair<u8, MD::Error_t> MD::getTemperature()
    {
        auto result = readRegister(m_mdRegisters.motorTemperature);
        if (result != Error_t::OK)
        {
            m_log.error("Could not read ");
            return std::make_pair(0, result);
        }
        return std::make_pair(m_mdRegisters.motorTemperature.value, result);
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

    std::vector<canId_t> MD::discoverMDs(Candle* candle)
    {
        constexpr canId_t MIN_VAILID_ID = 10;     // ids less than that are reserved for special
        constexpr canId_t MAX_VAILID_ID = 0x7FF;  // 11-bit value (standard can ID max)

        Logger               log(Logger::ProgramLayer_E::TOP, "MD_DISCOVERY");
        std::vector<canId_t> ids;

        if (candle == nullptr)
        {
            log.error("Candle is empty!");
            return std::vector<canId_t>();
        }

        log.info("Looking for MDs");

        for (canId_t id = MIN_VAILID_ID; id < MAX_VAILID_ID; id++)
        {
            log.debug("Trying to bind MD with id %d", id);
            log.progress(float(id) / float(MAX_VAILID_ID));
            // workaround for ping error spam
            Logger::Verbosity_E prevVerbosity =
                Logger::g_m_verbosity.value_or(Logger::Verbosity_E::VERBOSITY_1);
            Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;
            MD md(id, candle);
            if (md.init() == MD::Error_t::OK)
                ids.push_back(id);

            Logger::g_m_verbosity = prevVerbosity;
        }
        for (canId_t id : ids)
        {
            log.info("Discovered MD device with ID: %d", id);
        }
        if (ids.size() > 0)
            return ids;

        log.warn("Have not found any MD devices on the CAN bus!");
        return ids;
    }
}  // namespace mab
