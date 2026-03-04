#include "MDCO.hpp"
#include <unistd.h>
#include <cmath>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include "MDStatus.hpp"
#include "candle_types.hpp"
#include "edsEntry.hpp"
#include "mab_types.hpp"

namespace mab
{

    MDCO::Error_t MDCO::init()
    {
        return readSDO((*m_od)[0x1000]);
    }

    MDCO::Error_t MDCO::enable()
    {
        // set the mode of operation and enable the driver, log an error message if transfer failed
        Error_t err;
        (*m_od)[0x6040] = (open_types::UNSIGNED16_t)6;
        err             = writeSDO((*m_od)[0x6040]);
        if (err != Error_t::OK)
        {
            m_log.error("Error sending shutdown cmd!");
            return err;
        }
        usleep(2'000);
        (*m_od)[0x6040] = (open_types::UNSIGNED16_t)15;
        err             = writeSDO((*m_od)[0x6040]);
        if (err != Error_t::OK)
        {
            m_log.error("Error sending switch on and enable cmd!");
            return err;
        }
        usleep(2'000);
        return Error_t::OK;
    }

    MDCO::Error_t MDCO::disable()
    {
        // disable the driver, log an error message if transfer failed
        Error_t err;
        (*m_od)[0x6040] = (open_types::UNSIGNED16_t)6;
        err             = writeSDO((*m_od)[0x6040]);
        if (err != Error_t::OK)
        {
            m_log.error("Error sending disable operation cmd!");
            return err;
        }
        return Error_t::OK;
    }

    MDCO::Error_t MDCO::blink()
    {
        // blink the motor led, log an error message if transfer failed
        Error_t                      err       = enterConfigMode();
        static constexpr std::string blinkName = "Blink LEDs";
        auto                         blinkOpt  = m_od->getEntryByName(blinkName);
        if (!blinkOpt.has_value())
        {
            m_log.error("Coudl not locate %s object!", blinkName.c_str());
            return Error_t::UNKNOWN_OBJECT;
        }
        auto& blinkObj = blinkOpt.value().get();
        blinkObj       = (open_types::BOOLEAN_t)1;
        err            = writeSDO(blinkObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting Blink LEDs");
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::save()
    {
        Error_t err     = MDCO::Error_t::OK;
        (*m_od)[0x6060] = (open_types::INTEGER8_t)6;
        err             = writeSDO((*m_od)[0x6060]);
        if (err != Error_t::OK)
        {
            m_log.error("Error sending shutdown cmd!");
            return err;
        }
        (*m_od)[0x1010][0x1] =
            (open_types::UNSIGNED32_t)0x65766173;  // 0x65766173="save" in ASCII and little endian
        err = writeSDO((*m_od)[0x1010][0x1]);
        if (err != Error_t::OK)
        {
            m_log.error("Error saving all parameters");
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::zero()
    {
        // set the motor zero position to the actual position via SDO message, log an error message
        // if transfer failed
        Error_t                           err      = enterConfigMode();
        static constexpr std::string_view zeroName = "Set Zero";
        auto                              zeroOpt  = m_od->getEntryByName(zeroName);
        if (!zeroOpt.has_value())
        {
            m_log.error("Coudl not locate %s object!", zeroName.data());
            return Error_t::UNKNOWN_OBJECT;
        }
        auto& zeroObj = zeroOpt.value().get();
        zeroObj       = (open_types::BOOLEAN_t)1;
        err           = writeSDO(zeroObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting %s", zeroObj.getEntryMetaData().parameterName.c_str());
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::reset()
    {
        Error_t                           err       = enterConfigMode();
        static constexpr std::string_view resetName = "Reset Controller";
        auto                              resetOpt  = m_od->getEntryByName(resetName);
        if (!resetOpt.has_value())
        {
            m_log.error("Coudl not locate %s object!", resetName.data());
            return Error_t::UNKNOWN_OBJECT;
        }
        auto& resetObj = resetOpt.value().get();
        resetObj       = (open_types::BOOLEAN_t)1;
        err            = writeSDO(resetObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting %s", resetObj.getEntryMetaData().parameterName.c_str());
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::clearErrors()
    {
        Error_t                           err            = enterConfigMode();
        static constexpr std::string_view clearErrorName = "Clear Errors";
        auto                              clearErrorOpt  = m_od->getEntryByName(clearErrorName);
        if (!clearErrorOpt.has_value())
        {
            m_log.error("Coudl not locate %s object!", clearErrorName.data());
            return Error_t::UNKNOWN_OBJECT;
        }
        auto& clearErrorObj = clearErrorOpt.value().get();
        clearErrorObj       = (open_types::BOOLEAN_t)1;
        err                 = writeSDO(clearErrorObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting %s", clearErrorObj.getEntryMetaData().parameterName.c_str());
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::setCurrentLimit(float currentLimit /*A*/)
    {
        static constexpr std::string_view         setCurrentLimitName    = "Motor Rated Current";
        static constexpr std::string_view         setCurrentMaxLimitName = "Max Current";
        static constexpr open_types::UNSIGNED16_t setMaxLimit            = 1'000;
        auto setCurrentLimitOpt = m_od->getEntryByName(setCurrentLimitName);
        if (!setCurrentLimitOpt.has_value())
        {
            m_log.error("Could not locate %s object!", setCurrentLimitName.data());
            return Error_t::UNKNOWN_OBJECT;
        }
        auto& setCurrentLimitObj = setCurrentLimitOpt.value().get();
        setCurrentLimitObj       = (open_types::UNSIGNED16_t)(currentLimit * 1'000);
        Error_t err              = writeSDO(setCurrentLimitObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting %s",
                        setCurrentLimitObj.getEntryMetaData().parameterName.c_str());
            return err;
        }

        auto setCurrentMaxLimitOpt = m_od->getEntryByName(setCurrentMaxLimitName);
        if (!setCurrentMaxLimitOpt.has_value())
        {
            m_log.error("Could not locate %s object!", setCurrentMaxLimitName.data());
            return Error_t::UNKNOWN_OBJECT;
        }
        auto& setCurrentMaxLimitObj = setCurrentMaxLimitOpt.value().get();
        setCurrentMaxLimitObj       = setMaxLimit;
        err                         = writeSDO(setCurrentMaxLimitObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting %s",
                        setCurrentMaxLimitObj.getEntryMetaData().parameterName.c_str());
            return err;
        }

        return err;
    }

    MDCO::Error_t MDCO::setTorqueBandwidth(u16 torqueBandwidth /*Hz*/)
    {
        Error_t                           err                    = enterConfigMode();
        static constexpr std::string_view setTorqueBandwidthName = "Torque Bandwidth";
        static constexpr std::string_view reconfigureCANName     = "Run Can Reinit";
        auto setTorqueBandwidthOpt = m_od->getEntryByName(setTorqueBandwidthName);
        if (!setTorqueBandwidthOpt.has_value())
        {
            m_log.error("Could not locate %s object!", setTorqueBandwidthName.data());
            return Error_t::UNKNOWN_OBJECT;
        }
        auto& setTorqueBandwidthObj = setTorqueBandwidthOpt.value().get();
        setTorqueBandwidthObj       = (open_types::UNSIGNED16_t)torqueBandwidth;
        err                         = writeSDO(setTorqueBandwidthObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting %s",
                        setTorqueBandwidthObj.getEntryMetaData().parameterName.c_str());
            return err;
        }
        auto reconfigureCANOpt = m_od->getEntryByName(reconfigureCANName);
        if (!reconfigureCANOpt.has_value())
        {
            m_log.error("Could not locate %s object!", setTorqueBandwidthName.data());
            return Error_t::UNKNOWN_OBJECT;
        }
        auto& reconfigureCANObj = reconfigureCANOpt.value().get();
        reconfigureCANObj       = (open_types::BOOLEAN_t)1;
        err                     = writeSDO(reconfigureCANObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting %s",
                        setTorqueBandwidthObj.getEntryMetaData().parameterName.c_str());
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::setOperationMode(mab::ModesOfOperation mode)
    {
        constexpr std::string_view operationModeName = "Modes Of Operation";
        auto                       operationModeOpt  = m_od->getEntryByName(operationModeName);
        if (!operationModeOpt.has_value())
        {
            m_log.error("Could not locate %s object!", operationModeName.data());
            return Error_t::UNKNOWN_OBJECT;
        }
        auto& operationModeObj = operationModeOpt.value().get();
        operationModeObj       = (open_types::INTEGER8_t)mode;
        Error_t err            = writeSDO(operationModeObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting %s",
                        operationModeObj.getEntryMetaData().parameterName.c_str());
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::setPositionPIDparam(float kp, float ki, float kd, float integralMax)
    {
        Error_t   err     = enterConfigMode();
        const u16 address = m_od->getAdressByName("Position PID Controller").value().first;

        (*m_od)[address][0x1] = (open_types::REAL32_t)kp;
        err                   = writeSDO((*m_od)[address][0x1]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting position PID kp");
            return err;
        }

        (*m_od)[address][0x2] = (open_types::REAL32_t)ki;
        err                   = writeSDO((*m_od)[address][0x2]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting position PID ki");
            return err;
        }

        (*m_od)[address][0x3] = (open_types::REAL32_t)kd;
        err                   = writeSDO((*m_od)[address][0x3]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting position PID kd");
            return err;
        }

        (*m_od)[address][0x4] = (open_types::REAL32_t)integralMax;
        err                   = writeSDO((*m_od)[address][0x4]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting position PID integralMax");
            return err;
        }

        return err;
    }

    MDCO::Error_t MDCO::setVelocityPIDparam(float kp, float ki, float kd, float integralMax)
    {
        Error_t   err     = enterConfigMode();
        const u16 address = m_od->getAdressByName("Velocity PID Controller").value().first;

        (*m_od)[address][0x1] = (open_types::REAL32_t)kp;
        err                   = writeSDO((*m_od)[address][0x1]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting velocity PID kp");
            return err;
        }

        (*m_od)[address][0x2] = (open_types::REAL32_t)ki;
        err                   = writeSDO((*m_od)[address][0x2]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting velocity PID ki");
            return err;
        }

        (*m_od)[address][0x3] = (open_types::REAL32_t)kd;
        err                   = writeSDO((*m_od)[address][0x3]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting velocity PID kd");
            return err;
        }

        (*m_od)[address][0x4] = (open_types::REAL32_t)integralMax;
        err                   = writeSDO((*m_od)[address][0x4]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting velocity PID integralMax");
            return err;
        }

        return err;
    }

    MDCO::Error_t MDCO::setImpedanceParams(float kp, float kd)
    {
        Error_t   err     = enterConfigMode();
        const u16 address = m_od->getAdressByName("Impedance PD Controller").value().first;

        (*m_od)[address][0x1] = (open_types::REAL32_t)kp;
        err                   = writeSDO((*m_od)[address][0x1]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting impedance kp");
            return err;
        }

        (*m_od)[address][0x2] = (open_types::REAL32_t)kd;
        err                   = writeSDO((*m_od)[address][0x2]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting impedance kd");
            return err;
        }

        return err;
    }

    MDCO::Error_t MDCO::setMaxTorque(float maxTorque /*Nm*/)
    {
        (*m_od)[0x6076] = (open_types::UNSIGNED16_t)(maxTorque * 1000);
        (*m_od)[0x6072] = (open_types::UNSIGNED16_t)1000;
        Error_t err     = writeSDO((*m_od)[0x6072]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting Max Torque");
            return err;
        }
        err = writeSDO((*m_od)[0x6076]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting Max Torque");
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::setProfileVelocity(float profileVelocity /*s^-1*/)
    {
        (*m_od)[0x6081] = (open_types::UNSIGNED32_t)(profileVelocity * 1000);
        Error_t err     = writeSDO((*m_od)[0x6081]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting Profile Velocity");
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::setProfileAcceleration(float profileAcceleration /*s^-2*/)
    {
        (*m_od)[0x6083] = (open_types::UNSIGNED32_t)(profileAcceleration * 1000);
        Error_t err     = writeSDO((*m_od)[0x6083]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting Profile Acceleration");
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::setProfileDeceleration(float profileDeceleration /*s^-2*/)
    {
        (*m_od)[0x6084] = (open_types::UNSIGNED32_t)(profileDeceleration * 1000);
        Error_t err     = writeSDO((*m_od)[0x6084]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting Profile Deceleration");
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::setPositionWindow(u32 windowSize /*encode tics*/)
    {
        (*m_od)[0x6067] = (open_types::UNSIGNED32_t)(windowSize);
        Error_t err     = writeSDO((*m_od)[0x6067]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting Position Window");
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::setTargetPosition(i32 position /*encoder ticks*/)
    {
        (*m_od)[0x607A] = (open_types::INTEGER32_t)(position);
        Error_t err     = writeSDO((*m_od)[0x607A]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting Target Position");
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::setTargetVelocity(float velocity /*rad/s*/)
    {
        (*m_od)[0x60FF] = (open_types::INTEGER32_t)(velocity * 60 / (M_PI * 2));
        Error_t err     = writeSDO((*m_od)[0x60FF]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting Target Velocity");
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::setTargetTorque(float torque /*Nm*/)
    {
        (*m_od)[0x6074] = (open_types::INTEGER16_t)(torque * 1000);
        Error_t err     = writeSDO((*m_od)[0x6074]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting Target Torque");
            return err;
        }
        return err;
    }

    std::pair<const std::unordered_map<MDStatus::EncoderStatusBits, MDStatus::StatusItem_S>,
              MDCO::Error_t>
    MDCO::getMainEncoderStatus()
    {
        mab::MDStatus              statuses;
        constexpr std::string_view encoderStatusName = "Main Encoder Status";
        auto                       encoderStatusOpt  = m_od->getEntryByName(encoderStatusName);
        if (!encoderStatusOpt.has_value())
        {
            m_log.error("Could not locate %s object!", encoderStatusName.data());
            return {statuses.encoderStatus, Error_t::UNKNOWN_OBJECT};
        }
        auto&   encoderStatusObj = encoderStatusOpt.value().get();
        Error_t err              = readSDO(encoderStatusObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error reading %s",
                        encoderStatusObj.getEntryMetaData().parameterName.c_str());
        }
        mab::MDStatus::decode((open_types::UNSIGNED32_t)encoderStatusObj, statuses.encoderStatus);
        return {statuses.encoderStatus, err};
    }

    std::pair<const std::unordered_map<MDStatus::EncoderStatusBits, MDStatus::StatusItem_S>,
              MDCO::Error_t>
    MDCO::getOutputEncoderStatus()
    {
        mab::MDStatus              statuses;
        constexpr std::string_view encoderStatusName = "Output Encoder Status";
        auto                       encoderStatusOpt  = m_od->getEntryByName(encoderStatusName);
        if (!encoderStatusOpt.has_value())
        {
            m_log.error("Could not locate %s object!", encoderStatusName.data());
            return {statuses.encoderStatus, Error_t::UNKNOWN_OBJECT};
        }
        auto&   encoderStatusObj = encoderStatusOpt.value().get();
        Error_t err              = readSDO(encoderStatusObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error reading %s",
                        encoderStatusObj.getEntryMetaData().parameterName.c_str());
        }
        mab::MDStatus::decode((open_types::UNSIGNED32_t)encoderStatusObj, statuses.encoderStatus);
        return {statuses.encoderStatus, err};
    }

    std::pair<const std::unordered_map<MDStatus::CalibrationStatusBits, MDStatus::StatusItem_S>,
              MDCO::Error_t>
    MDCO::getCalibrationStatus()
    {
        mab::MDStatus              statuses;
        constexpr std::string_view calibrationStatusName = "Calibration Status";
        auto calibrationStatusOpt = m_od->getEntryByName(calibrationStatusName);
        if (!calibrationStatusOpt.has_value())
        {
            m_log.error("Could not locate %s object!", calibrationStatusName.data());
            return {statuses.calibrationStatus, Error_t::UNKNOWN_OBJECT};
        }
        auto&   calibrationStatusObj = calibrationStatusOpt.value().get();
        Error_t err                  = readSDO(calibrationStatusObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error reading %s",
                        calibrationStatusObj.getEntryMetaData().parameterName.c_str());
        }
        mab::MDStatus::decode((open_types::UNSIGNED32_t)calibrationStatusObj,
                              statuses.calibrationStatus);
        return {statuses.calibrationStatus, err};
    }

    std::pair<const std::unordered_map<MDStatus::BridgeStatusBits, MDStatus::StatusItem_S>,
              MDCO::Error_t>
    MDCO::getBridgeStatus()
    {
        mab::MDStatus              statuses;
        constexpr std::string_view bridgeStatusName = "Bridge Status";
        auto                       bridgeStatusOpt  = m_od->getEntryByName(bridgeStatusName);
        if (!bridgeStatusOpt.has_value())
        {
            m_log.error("Could not locate %s object!", bridgeStatusName.data());
            return {statuses.bridgeStatus, Error_t::UNKNOWN_OBJECT};
        }
        auto&   bridgeStatusObj = bridgeStatusOpt.value().get();
        Error_t err             = readSDO(bridgeStatusObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error reading %s",
                        bridgeStatusObj.getEntryMetaData().parameterName.c_str());
        }
        mab::MDStatus::decode((open_types::UNSIGNED32_t)bridgeStatusObj, statuses.bridgeStatus);
        return {statuses.bridgeStatus, err};
    }

    std::pair<const std::unordered_map<MDStatus::HardwareStatusBits, MDStatus::StatusItem_S>,
              MDCO::Error_t>
    MDCO::getHardwareStatus()
    {
        mab::MDStatus              statuses;
        constexpr std::string_view hardwareStatusName = "Hardware Status";
        auto                       hardwareStatusOpt  = m_od->getEntryByName(hardwareStatusName);
        if (!hardwareStatusOpt.has_value())
        {
            m_log.error("Could not locate %s object!", hardwareStatusName.data());
            return {statuses.hardwareStatus, Error_t::UNKNOWN_OBJECT};
        }
        auto&   hardwareStatusObj = hardwareStatusOpt.value().get();
        Error_t err               = readSDO(hardwareStatusObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error reading %s",
                        hardwareStatusObj.getEntryMetaData().parameterName.c_str());
        }
        mab::MDStatus::decode((open_types::UNSIGNED32_t)hardwareStatusObj, statuses.hardwareStatus);
        return {statuses.hardwareStatus, err};
    }

    std::pair<const std::unordered_map<MDStatus::CommunicationStatusBits, MDStatus::StatusItem_S>,
              MDCO::Error_t>
    MDCO::getCommunicationStatus()
    {
        mab::MDStatus              statuses;
        constexpr std::string_view communicationStatusName = "Communication Status";
        auto communicationStatusOpt = m_od->getEntryByName(communicationStatusName);
        if (!communicationStatusOpt.has_value())
        {
            m_log.error("Could not locate %s object!", communicationStatusName.data());
            return {statuses.communicationStatus, Error_t::UNKNOWN_OBJECT};
        }
        auto&   communicationStatusObj = communicationStatusOpt.value().get();
        Error_t err                    = readSDO(communicationStatusObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error reading %s",
                        communicationStatusObj.getEntryMetaData().parameterName.c_str());
        }
        mab::MDStatus::decode((open_types::UNSIGNED32_t)communicationStatusObj,
                              statuses.communicationStatus);
        return {statuses.communicationStatus, err};
    }

    std::pair<const std::unordered_map<MDStatus::MotionStatusBits, MDStatus::StatusItem_S>,
              MDCO::Error_t>
    MDCO::getMotionStatus()
    {
        mab::MDStatus              statuses;
        constexpr std::string_view motionStatusName = "Motion Status";
        auto                       motionStatusOpt  = m_od->getEntryByName(motionStatusName);
        if (!motionStatusOpt.has_value())
        {
            m_log.error("Could not locate %s object!", motionStatusName.data());
            return {statuses.motionStatus, Error_t::UNKNOWN_OBJECT};
        }
        auto&   motionStatusObj = motionStatusOpt.value().get();
        Error_t err             = readSDO(motionStatusObj);
        if (err != Error_t::OK)
        {
            m_log.error("Error reading %s",
                        motionStatusObj.getEntryMetaData().parameterName.c_str());
        }
        mab::MDStatus::decode((open_types::UNSIGNED32_t)motionStatusObj, statuses.motionStatus);
        return {statuses.motionStatus, err};
    }

    std::pair<i32, MDCO::Error_t> MDCO::getPosition()
    {
        Error_t err = readSDO((*m_od)[0x6064]);
        if (err != Error_t::OK)
        {
            m_log.error("Error reading Position");
            return {0.0f, err};
        }

        i32 positionRaw = (i32)(open_types::INTEGER32_t)(*m_od)[0x6064];

        return {positionRaw, err};
    }

    std::pair<float, MDCO::Error_t> MDCO::getVelocity()
    {
        Error_t err = readSDO((*m_od)[0x606C]);
        if (err != Error_t::OK)
        {
            m_log.error("Error reading Velocity");
            return {0.0f, err};
        }

        i32   velocityRaw = (i32)(open_types::INTEGER32_t)(*m_od)[0x606C];
        float velocity    = velocityRaw;

        return {velocity, err};
    }

    std::pair<float, MDCO::Error_t> MDCO::getTorque()
    {
        Error_t err = readSDO((*m_od)[0x6077]);
        if (err != Error_t::OK)
        {
            m_log.error("Error reading Torque");
            return {0.0f, err};
        }

        i16   torqueRaw = (i16)(open_types::INTEGER16_t)(*m_od)[0x6077];
        float torque    = torqueRaw / 1000.0f;

        return {torque, err};
    }

    std::pair<float, MDCO::Error_t> MDCO::getOutputEncoderPosition()
    {
        Error_t err = readSDO((*m_od)[0x2200][0x1]);
        if (err != Error_t::OK)
        {
            m_log.error("Error reading Output Encoder Position");
            return {0.0f, err};
        }

        i32   positionRaw = (i32)(open_types::INTEGER32_t)(*m_od)[0x2200][0x1];
        float position    = positionRaw / 1000000.0f;

        return {position, err};
    }

    std::pair<float, MDCO::Error_t> MDCO::getOutputEncoderVelocity()
    {
        Error_t err = readSDO((*m_od)[0x2200][0x2]);
        if (err != Error_t::OK)
        {
            m_log.error("Error reading Output Encoder Velocity");
            return {0.0f, err};
        }

        i32   velocityRaw = (i32)(open_types::INTEGER32_t)(*m_od)[0x2200][0x2];
        float velocity    = velocityRaw / 1000000.0f;

        return {velocity, err};
    }

    std::pair<u8, MDCO::Error_t> MDCO::getTemperature()
    {
        Error_t err = readSDO((*m_od)[0x2300]);
        if (err != Error_t::OK)
        {
            m_log.error("Error reading Temperature");
            return {0, err};
        }

        u8 temperature = (u8)(open_types::UNSIGNED8_t)(*m_od)[0x2300];

        return {temperature, err};
    }

    MDCO::Error_t MDCO::readSDO(EDSEntry& edsEntry) const
    {
        std::vector<std::byte> result;
        if (!edsEntry.getValueMetaData().has_value())
        {
            return Error_t::UNKNOWN_OBJECT;
        }

        if (edsEntry.valueSize() <= 4 &&
            edsEntry.getEntryMetaData().edsValueMeta.value().dataType !=
                EDSEntry::DataType_E::VISIBLE_STRING)
        {
            // using expedited transfer
            std::vector<u8> transmitFrame = {
                INITIATE_SDO_UPLOAD_REQUEST,
                (u8)edsEntry.getEntryMetaData().address.first,
                (u8)(edsEntry.getEntryMetaData().address.first >> 8),
                (u8)(edsEntry.getEntryMetaData().address.second.value_or(0))};
            transmitFrame.resize(8, 0);

            auto [response, error] = transferCanOpenFrame(
                SDO_REQUEST_BASE + m_canId, transmitFrame, transmitFrame.size());

            if (error != candleTypes::Error_t::OK)
            {
                m_log.error("Failed upload SDO 0x%x", SDO_REQUEST_BASE + m_canId);
                return Error_t::TRANSFER_FAILED;
            }
            m_log.debug("Address: 0x%x", edsEntry.getEntryMetaData().address.first);

            // Verify expedited response (bit 1 == 1)
            if ((response[0] & 0x40) == 0)
            {
                m_log.error("Invalid expedited download response");

                return Error_t::TRANSFER_FAILED;
            }

            // Number of unused bytes (bits 2-3)
            u8     emptyBytes = (response[0] >> 2) & 0x03;
            size_t dataSize   = 4 - emptyBytes;

            result.reserve(dataSize);

            for (size_t i = 0; i < dataSize; ++i)
            {
                result.push_back(static_cast<std::byte>(response[4 + i]));
            }
        }
        else
        {
            // using segmented transfer

            std::vector<u8> transmitFrame = {
                INITIATE_SDO_UPLOAD_REQUEST,
                (u8)edsEntry.getEntryMetaData().address.first,
                (u8)(edsEntry.getEntryMetaData().address.first >> 8),
                (u8)(edsEntry.getEntryMetaData().address.second.value_or(0))};
            transmitFrame.resize(8, 0);

            auto [response, error] = transferCanOpenFrame(
                SDO_REQUEST_BASE + m_canId, transmitFrame, transmitFrame.size());

            if (error != candleTypes::Error_t::OK)
            {
                m_log.error("Failed initiate segmented upload SDO 0x%x",
                            SDO_REQUEST_BASE + m_canId);
                return Error_t::TRANSFER_FAILED;
            }

            // If server responds with expedited transfer, handle it here
            if ((response[0] & 0x02) != 0)
            {
                // Size indicated must be set for expedited upload
                if ((response[0] & 0x01) == 0)
                {
                    m_log.error("Expedited upload without size indication");
                    return Error_t::TRANSFER_FAILED;
                }

                u8     emptyBytes = (response[0] >> 2) & 0x03;
                size_t dataSize   = 4 - emptyBytes;

                result.reserve(dataSize);

                for (size_t i = 0; i < dataSize; ++i)
                {
                    result.push_back(static_cast<std::byte>(response[4 + i]));
                }

                // Store value and return immediately (no segmented loop)
                if (edsEntry.setSerializedValue(result) == EDSEntry::Error_t::OK)
                    return Error_t::OK;

                m_log.error("EDS parsing failed with code: %d",
                            edsEntry.setSerializedValue(result));
                return Error_t::REQUEST_INVALID;
            }

            std::vector<u8> completeData;
            bool            lastSegment = false;
            u8              toggle      = 0;

            while (!lastSegment)
            {
                std::vector<u8> segmentRequest(8, 0);
                segmentRequest[0] = 0x60 | (toggle << 4);

                auto [segmentResponse, segError] = transferCanOpenFrame(
                    SDO_REQUEST_BASE + m_canId, segmentRequest, segmentRequest.size());

                if (segError != candleTypes::Error_t::OK)
                {
                    m_log.error("Segment upload failed SDO 0x%x", SDO_REQUEST_BASE + m_canId);
                    return Error_t::TRANSFER_FAILED;
                }

                lastSegment   = (segmentResponse[0] & 0x01);
                u8 emptyBytes = (segmentResponse[0] >> 1) & 0x07;

                size_t dataSize = 7 - emptyBytes;

                completeData.insert(completeData.end(),
                                    segmentResponse.begin() + 1,
                                    segmentResponse.begin() + 1 + dataSize);

                toggle ^= 1;
            }

            // Move into result as std::byte
            result.reserve(completeData.size());
            for (u8 b : completeData)
            {
                result.push_back(static_cast<std::byte>(b));
            }
        }
        if (edsEntry.setSerializedValue(result) == EDSEntry::Error_t::OK)
            return Error_t::OK;
        else
        {
            m_log.error("EDS parsing failed with code: %d", edsEntry.setSerializedValue(result));
            return Error_t::REQUEST_INVALID;
        }
    }

    MDCO::Error_t MDCO::writeSDO(EDSEntry& edsEntry) const
    {
        if (!edsEntry.getValueMetaData().has_value())
        {
            return Error_t::UNKNOWN_OBJECT;
        }
        if (edsEntry.getValueMetaData().value().accessType == EDSEntry::AccessRights_E::READ_ONLY)
        {
            m_log.error("Coudl not write to %s as it is a read-only object!",
                        edsEntry.getEntryMetaData().parameterName.c_str());
        }

        const std::vector<std::byte>& data        = edsEntry.getSerializedValue();
        const size_t                  payloadSize = edsEntry.valueSize();
        const size_t                  size        = data.size();

        if (payloadSize <= 4)
        {
            // -------- Expedited download --------
            std::vector<u8> transmitFrame(8, 0);

            // Command specifier:
            // 0x20 = initiate download
            // 0x02 = expedited
            // 0x01 = size indicated
            // bits 2-3 = number of unused bytes
            // if (payloadSize == 0x1)
            //     transmitFrame[0] = 0x2F;
            // else if (payloadSize == 0x2)
            //     transmitFrame[0] = 0x2B;
            // else if (payloadSize == 0x3)
            //     transmitFrame[0] = 0x27;
            // else
            //     transmitFrame[0] = 0x23;
            transmitFrame[0] = INITIATE_SDO_DOWNLOAD_REQUEST;
            transmitFrame[1] = (u8)edsEntry.getEntryMetaData().address.first;
            transmitFrame[2] = (u8)(edsEntry.getEntryMetaData().address.first >> 8);
            transmitFrame[3] = (u8)(edsEntry.getEntryMetaData().address.second.value_or(0));

            for (size_t i = 0; i < payloadSize; ++i)
            {
                transmitFrame[4 + i] = static_cast<u8>(data[i]);
            }

            auto [response, error] = transferCanOpenFrame(
                SDO_REQUEST_BASE + m_canId, transmitFrame, transmitFrame.size());

            if (error != candleTypes::Error_t::OK)
            {
                m_log.error("Failed expedited download SDO 0x%x", SDO_REQUEST_BASE + m_canId);
                return Error_t::TRANSFER_FAILED;
            }
            m_log.debug("Address: 0x%x", edsEntry.getEntryMetaData().address.first);
            m_log.debug("Lenght: 0x%x", payloadSize);

            // Expect initiate download response (0x60)
            if ((response[0] & 0xE0) != 0x60)
            {
                m_log.error("Invalid expedited download response");
                return Error_t::TRANSFER_FAILED;
            }
        }
        else
        {
            // -------- Segmented download --------

            // ---- Initiate segmented download ----
            std::vector<u8> transmitFrame(8, 0);

            // 0x20 = initiate download (no expedited, no size indicated here)
            transmitFrame[0] = 0x20;
            transmitFrame[1] = (u8)edsEntry.getEntryMetaData().address.first;
            transmitFrame[2] = (u8)(edsEntry.getEntryMetaData().address.first >> 8);
            transmitFrame[3] = (u8)(edsEntry.getEntryMetaData().address.second.value_or(0));

            auto [response, error] = transferCanOpenFrame(
                SDO_REQUEST_BASE + m_canId, transmitFrame, transmitFrame.size());

            if (error != candleTypes::Error_t::OK)
            {
                m_log.error("Failed initiate segmented download SDO 0x%x",
                            SDO_REQUEST_BASE + m_canId);
                return Error_t::TRANSFER_FAILED;
            }

            if ((response[0] & 0xE0) != 0x60)
            {
                m_log.error("Invalid initiate segmented download response");
                return Error_t::TRANSFER_FAILED;
            }

            // ---- Send segments ----
            size_t offset      = 0;
            u8     toggle      = 0;
            bool   lastSegment = false;

            while (!lastSegment)
            {
                std::vector<u8> segmentFrame(8, 0);

                size_t remaining = size - offset;
                size_t chunkSize = (remaining > 7) ? 7 : remaining;

                lastSegment = (remaining <= 7);

                u8 emptyBytes = static_cast<u8>(7 - chunkSize);

                // 0x00 = download segment
                // bit 4 = toggle
                // bit 0 = last segment
                // bits 1-3 = number of unused bytes (only valid for last segment)
                segmentFrame[0] = (toggle << 4) | (lastSegment ? 0x01 : 0x00) |
                                  (lastSegment ? (emptyBytes << 1) : 0x00);

                for (size_t i = 0; i < chunkSize; ++i)
                {
                    segmentFrame[1 + i] = static_cast<u8>(data[offset + i]);
                }

                auto [segmentResponse, segError] = transferCanOpenFrame(
                    SDO_REQUEST_BASE + m_canId, segmentFrame, segmentFrame.size());

                if (segError != candleTypes::Error_t::OK)
                {
                    m_log.error("Segment download failed SDO 0x%x", SDO_REQUEST_BASE + m_canId);
                    return Error_t::TRANSFER_FAILED;
                }

                // Expect segment response (0x20 | toggle<<4)
                if ((segmentResponse[0] & 0xE0) != 0x20)
                {
                    m_log.error("Invalid segment download response");
                    return Error_t::TRANSFER_FAILED;
                }

                offset += chunkSize;
                toggle ^= 1;
            }
        }

        return Error_t::OK;
    }

    MDCO::Error_t MDCO::resetNMT() const
    {
        // NMT Reset Node command (0x81) to this node ID
        std::vector<u8> frame(8, 0);
        frame[0] = 0x81;     // Reset Node command
        frame[1] = m_canId;  // Target node ID

        auto [response, error] = transferCanOpenFrame(0x000, frame, 2);

        if (error != candleTypes::Error_t::OK)
        {
            m_log.error("Failed to send NMT Reset to node %d", m_canId);
            return Error_t::TRANSFER_FAILED;
        }

        return Error_t::OK;
    }

    std::vector<canId_t> MDCO::discoverOpenMDs(Candle*                              candle,
                                               std::shared_ptr<EDSObjectDictionary> od)
    {
        constexpr canId_t MIN_VALID_ID = 0x01;  // ids less than that are reserved for special
        constexpr canId_t MAX_VALID_ID = 0x7F;  // 0x600-0x580=0x7F

        std::vector<u8> frame = {
            0x40,  // Command: initiate upload
            0x00,  // Index LSB
            0x10,  // Index MSB
            0,     // Subindex
            0,     // Padding
            0,
            0,
            0,
        };

        Logger               log(Logger::ProgramLayer_E::TOP, "MD_DISCOVERY");
        std::vector<canId_t> ids;

        if (candle == nullptr)
        {
            log.error("Candle is empty!");
            return std::vector<canId_t>();
        }

        log.info("Looking for MDs");

        for (canId_t id = MIN_VALID_ID; id < MAX_VALID_ID; id++)
        {
            log.debug("Trying to bind MD with id %d", id);
            log.progress(float(id) / float(MAX_VALID_ID));
            // workaround for ping error spam
            Logger::Verbosity_E prevVerbosity =
                Logger::g_m_verbosity.value_or(Logger::Verbosity_E::VERBOSITY_1);
            Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;
            MDCO md(id, candle, od);
            auto [response, error] = md.transferCanOpenFrame(0x600 + id, frame, frame.size());

            if (response[4] == 0x92)
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
