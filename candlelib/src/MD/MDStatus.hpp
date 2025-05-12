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
            mutable bool      m_set = false;

            StatusItem_S(std::string _name) : name(_name)
            {
            }

            inline void set(bool _set) const
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

        const std::unordered_map<const QuickStatusBits, StatusItem_S> quickStatus =
            std::unordered_map<const QuickStatusBits, StatusItem_S>{
                {QuickStatusBits::MainEncoderStatus, StatusItem_S("Main Encoder Status")},
                {QuickStatusBits::OutputEncoderStatus, StatusItem_S("Output Encoder Status")},
                {QuickStatusBits::CalibrationEncoderStatus,
                 StatusItem_S("Calibration Encoder Status")},
                {QuickStatusBits::MosfetBridgeStatus, StatusItem_S("Mosfet Bridge Status")},
                {QuickStatusBits::HardwareStatus, StatusItem_S("Hardware Status")},
                {QuickStatusBits::CommunicationStatus, StatusItem_S("Communication Status")},
                {QuickStatusBits::MotionStatus, StatusItem_S("Motion Status")},
                {QuickStatusBits::TargetPositionReached,
                 StatusItem_S("Target Position Reached")}  //
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

        const std::unordered_map<const EncoderStatusBits, StatusItem_S> encoderStatus = {
            {EncoderStatusBits::ErrorCommunication, StatusItem_S("Error Communication")},
            {EncoderStatusBits::ErrorWrongDirection, StatusItem_S("Error Wrong Direction")},
            {EncoderStatusBits::ErrorEmptyLUT, StatusItem_S("Error Empty LUT")},
            {EncoderStatusBits::ErrorFaultyLUT, StatusItem_S("Error Faulty LUT")},
            {EncoderStatusBits::ErrorCalibration, StatusItem_S("Error Calibration")},
            {EncoderStatusBits::ErrorPositionInvalid, StatusItem_S("Error Position Invalid")},
            {EncoderStatusBits::ErrorInitialization, StatusItem_S("Error Initialization")},
            {EncoderStatusBits::WarningLowAccuracy, StatusItem_S("Warning Low Accuracy")}};

        enum class CalibrationStatusBits : bitPos
        {
            ErrorOffsetCalibration = 0,
            ErrorResistance        = 1,
            ErrorInductance        = 2,
            ErrorPolePairDetection = 3,
            ErrorSetup             = 4
        };

        const std::unordered_map<const CalibrationStatusBits, StatusItem_S> calibrationStatus = {
            {CalibrationStatusBits::ErrorOffsetCalibration,
             StatusItem_S("Error Offset Calibration")},
            {CalibrationStatusBits::ErrorResistance, StatusItem_S("Error Resistance")},
            {CalibrationStatusBits::ErrorInductance, StatusItem_S("Error Inductance")},
            {CalibrationStatusBits::ErrorPolePairDetection,
             StatusItem_S("Error Pole Pair Detection")},
            {CalibrationStatusBits::ErrorSetup, StatusItem_S("Error Setup")}};

        enum class BridgeStatusBits : bitPos
        {
            ErrorCommunication = 0,
            ErrorOvercurrent   = 1,
            ErrorGeneralFault  = 2
        };

        const std::unordered_map<const BridgeStatusBits, StatusItem_S> bridgeStatus = {
            {BridgeStatusBits::ErrorCommunication, StatusItem_S("Error Communication")},
            {BridgeStatusBits::ErrorOvercurrent, StatusItem_S("Error Overcurrent")},
            {BridgeStatusBits::ErrorGeneralFault, StatusItem_S("Error General Fault")}};

        enum class HardwareStatusBits : bitPos
        {
            ErrorOverCurrent       = 0,
            ErrorOverVoltage       = 1,
            ErrorUnderVoltage      = 2,
            ErrorMotorTemperature  = 3,
            ErrorMosfetTemperature = 4,
            ErrorADCCurrentOffset  = 5
        };

        const std::unordered_map<const HardwareStatusBits, StatusItem_S> hardwareStatus = {
            {HardwareStatusBits::ErrorOverCurrent, StatusItem_S("Error Over Current")},
            {HardwareStatusBits::ErrorOverVoltage, StatusItem_S("Error Over Voltage")},
            {HardwareStatusBits::ErrorUnderVoltage, StatusItem_S("Error Under Voltage")},
            {HardwareStatusBits::ErrorMotorTemperature, StatusItem_S("Error Motor Temperature")},
            {HardwareStatusBits::ErrorMosfetTemperature, StatusItem_S("Error Mosfet Temperature")},
            {HardwareStatusBits::ErrorADCCurrentOffset, StatusItem_S("Error ADC Current Offset")}};

        enum class CommunicationStatusBits : bitPos
        {
            WarningCANWatchdog = 30
        };

        const std::unordered_map<const CommunicationStatusBits, StatusItem_S> communicationStatus =
            {{CommunicationStatusBits::WarningCANWatchdog, StatusItem_S("Warning CAN Watchdog")}};

        enum class MotionStatusBits : bitPos
        {
            ErrorPositionLimit  = 0,
            ErrorVelocityLimit  = 1,
            WarningAcceleration = 24,
            WarningTorque       = 25,
            WarningVelocity     = 26,
            WarningPosition     = 27
        };

        const std::unordered_map<const MotionStatusBits, StatusItem_S> motionStatus = {
            {MotionStatusBits::ErrorPositionLimit, StatusItem_S("Error Position Limit")},
            {MotionStatusBits::ErrorVelocityLimit, StatusItem_S("Error Velocity Limit")},
            {MotionStatusBits::WarningAcceleration, StatusItem_S("Warning Acceleration")},
            {MotionStatusBits::WarningTorque, StatusItem_S("Warning Torque")},
            {MotionStatusBits::WarningVelocity, StatusItem_S("Warning Velocity")},
            {MotionStatusBits::WarningPosition, StatusItem_S("Warning Position")}};

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
        static void toMap(u32 bytes, const std::unordered_map<const T, StatusItem_S>& map)
        {
            for (auto& bit : map)
            {
                bit.second.set(static_cast<bool>(bytes & (1 << static_cast<u8>(bit.first))));
            }
        }
    };
}  // namespace mab
