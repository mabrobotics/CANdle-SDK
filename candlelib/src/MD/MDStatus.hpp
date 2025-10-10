#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include <mab_types.hpp>

namespace mab
{
    struct MDStatus
    {
        struct StatusItem_S
        {
            const std::string name;
            const bool        isError = false;
            mutable bool      m_set   = false;

            StatusItem_S(std::string _name, bool _isError) : name(_name), isError(_isError)
            {
            }

            operator bool() const
            {
                return isSet();
            }

            inline void set(bool _set)
            {
                m_set = _set;
            }

            inline bool isSet() const
            {
                return m_set;
            }
        };

        using bitPos = u8;

        enum class QuickStatusBits : bitPos
        {
            MainEncoderStatus        = 0,
            OutputEncoderStatus      = 1,
            CalibrationEncoderStatus = 2,
            MosfetBridgeStatus       = 3,
            HardwareStatus           = 4,
            CommunicationStatus      = 5,
            MotionStatus             = 6,
            TargetPositionReached    = 15
        };

        std::unordered_map<QuickStatusBits, StatusItem_S> quickStatus = {
            {QuickStatusBits::MainEncoderStatus, StatusItem_S("Main Encoder Status", false)},
            {QuickStatusBits::OutputEncoderStatus, StatusItem_S("Output Encoder Status", false)},
            {QuickStatusBits::CalibrationEncoderStatus,
             StatusItem_S("Calibration Encoder Status", false)},
            {QuickStatusBits::MosfetBridgeStatus, StatusItem_S("Mosfet Bridge Status", false)},
            {QuickStatusBits::HardwareStatus, StatusItem_S("Hardware Status", false)},
            {QuickStatusBits::CommunicationStatus, StatusItem_S("Communication Status", false)},
            {QuickStatusBits::MotionStatus, StatusItem_S("Motion Status", false)},
            {QuickStatusBits::TargetPositionReached,
             StatusItem_S("Target Position Reached", false)}  //
        };

        enum class EncoderStatusBits : bitPos
        {
            ErrorCommunication   = 0,
            ErrorWrongDirection  = 1,
            ErrorEmptyLUT        = 2,
            ErrorFaultyLUT       = 3,
            ErrorCalibration     = 4,
            ErrorPositionInvalid = 5,
            ErrorInitialization  = 6,
            WarningLowAccuracy   = 30
        };

        std::unordered_map<EncoderStatusBits, StatusItem_S> encoderStatus = {
            {EncoderStatusBits::ErrorCommunication, StatusItem_S("Error Communication", true)},
            {EncoderStatusBits::ErrorWrongDirection, StatusItem_S("Error Wrong Direction", true)},
            {EncoderStatusBits::ErrorEmptyLUT, StatusItem_S("Error Empty LUT", true)},
            {EncoderStatusBits::ErrorFaultyLUT, StatusItem_S("Error Faulty LUT", true)},
            {EncoderStatusBits::ErrorCalibration, StatusItem_S("Error Calibration", true)},
            {EncoderStatusBits::ErrorPositionInvalid, StatusItem_S("Error Position Invalid", true)},
            {EncoderStatusBits::ErrorInitialization, StatusItem_S("Error Initialization", true)},
            {EncoderStatusBits::WarningLowAccuracy, StatusItem_S("Warning Low Accuracy", false)}};

        enum class CalibrationStatusBits : bitPos
        {
            ErrorOffsetCalibration = 0,
            ErrorResistance        = 1,
            ErrorInductance        = 2,
            ErrorPolePairDetection = 3,
            ErrorSetup             = 4
        };

        std::unordered_map<CalibrationStatusBits, StatusItem_S> calibrationStatus = {
            {CalibrationStatusBits::ErrorOffsetCalibration,
             StatusItem_S("Error Offset Calibration", true)},
            {CalibrationStatusBits::ErrorResistance, StatusItem_S("Error Resistance", true)},
            {CalibrationStatusBits::ErrorInductance, StatusItem_S("Error Inductance", true)},
            {CalibrationStatusBits::ErrorPolePairDetection,
             StatusItem_S("Error Pole Pair Detection", true)},
            {CalibrationStatusBits::ErrorSetup, StatusItem_S("Error Setup", true)}};

        enum class BridgeStatusBits : bitPos
        {
            ErrorCommunication = 0,
            ErrorOvercurrent   = 1,
            ErrorGeneralFault  = 2
        };

        std::unordered_map<BridgeStatusBits, StatusItem_S> bridgeStatus = {
            {BridgeStatusBits::ErrorCommunication, StatusItem_S("Error Communication", true)},
            {BridgeStatusBits::ErrorOvercurrent, StatusItem_S("Error Overcurrent", true)},
            {BridgeStatusBits::ErrorGeneralFault, StatusItem_S("Error General Fault", true)}};

        enum class HardwareStatusBits : bitPos
        {
            ErrorOverCurrent       = 0,
            ErrorOverVoltage       = 1,
            ErrorUnderVoltage      = 2,
            ErrorMotorTemperature  = 3,
            ErrorMosfetTemperature = 4,
            ErrorADCCurrentOffset  = 5
        };

        std::unordered_map<HardwareStatusBits, StatusItem_S> hardwareStatus = {
            {HardwareStatusBits::ErrorOverCurrent, StatusItem_S("Error Over Current", true)},
            {HardwareStatusBits::ErrorOverVoltage, StatusItem_S("Error Over Voltage", true)},
            {HardwareStatusBits::ErrorUnderVoltage, StatusItem_S("Error Under Voltage", true)},
            {HardwareStatusBits::ErrorMotorTemperature,
             StatusItem_S("Error Motor Temperature", true)},
            {HardwareStatusBits::ErrorMosfetTemperature,
             StatusItem_S("Error Mosfet Temperature", true)},
            {HardwareStatusBits::ErrorADCCurrentOffset,
             StatusItem_S("Error ADC Current Offset", true)}};

        enum class CommunicationStatusBits : bitPos
        {
            WarningCANWatchdog = 30
        };

        std::unordered_map<CommunicationStatusBits, StatusItem_S> communicationStatus = {
            {CommunicationStatusBits::WarningCANWatchdog,
             StatusItem_S("Warning CAN Watchdog", false)}};

        enum class MotionStatusBits : bitPos
        {
            ErrorPositionLimit  = 0,
            ErrorVelocityLimit  = 1,
            WarningAcceleration = 24,
            WarningTorque       = 25,
            WarningVelocity     = 26,
            WarningPosition     = 27
        };

        std::unordered_map<MotionStatusBits, StatusItem_S> motionStatus = {
            {MotionStatusBits::ErrorPositionLimit, StatusItem_S("Error Position Limit", true)},
            {MotionStatusBits::ErrorVelocityLimit, StatusItem_S("Error Velocity Limit", true)},
            {MotionStatusBits::WarningAcceleration, StatusItem_S("Warning Acceleration", false)},
            {MotionStatusBits::WarningTorque, StatusItem_S("Warning Torque", false)},
            {MotionStatusBits::WarningVelocity, StatusItem_S("Warning Velocity", false)},
            {MotionStatusBits::WarningPosition, StatusItem_S("Warning Position", false)}};

        static std::vector<std::string> getStatusString(
            std::unordered_map<bitPos, StatusItem_S> errors)
        {
            std::vector<std::string> activeErrors;
            for (const auto& err : errors)
            {
                if (err.second.m_set)
                    activeErrors.push_back(err.second.name);
            }
            return activeErrors;
        }

        template <class T>
        static void decode(u32 bytes, std::unordered_map<T, StatusItem_S>& map)
        {
            for (auto& bit : map)
            {
                bit.second.set(static_cast<bool>(bytes & (1 << static_cast<u8>(bit.first))));
            }
        }
    };
}  // namespace mab
