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
        const std::unordered_map<bitPos, StatusItem_S> quickStatus =
            std::unordered_map<bitPos, StatusItem_S>{
                std::make_pair(0, StatusItem_S("Main Encoder Error")),
                std::make_pair(1, StatusItem_S("Output Encoder Error")),
                std::make_pair(2, StatusItem_S("Calibration Encoder Error")),
                std::make_pair(3, StatusItem_S("Mosfet Bridge Error")),
                std::make_pair(4, StatusItem_S("Hardware Errors")),
                std::make_pair(5, StatusItem_S("Communication Errors")),
                std::make_pair(6, StatusItem_S("Motion Errors")),
                std::make_pair(15, StatusItem_S("Target Position Reached"))  //
            };

        const std::unordered_map<bitPos, StatusItem_S> encoderError =
            std::unordered_map<bitPos, StatusItem_S>{
                std::make_pair(0, StatusItem_S("None")),
                std::make_pair(0, StatusItem_S("Error Communication")),
                std::make_pair(1, StatusItem_S("Error Wrong Direction")),
                std::make_pair(2, StatusItem_S("Error Empty LUT")),
                std::make_pair(3, StatusItem_S("Error Faulty LUT")),
                std::make_pair(4, StatusItem_S("Error Calibration")),
                std::make_pair(5, StatusItem_S("Error Position Invalid")),
                std::make_pair(6, StatusItem_S("Error Initialization")),
                std::make_pair(30, StatusItem_S("Warning Low Accuracy"))};

        const std::unordered_map<bitPos, StatusItem_S> calibrationError =
            std::unordered_map<bitPos, StatusItem_S>{
                std::make_pair(0, StatusItem_S("None")),
                std::make_pair(1, StatusItem_S("Error Offset Calibration")),
                std::make_pair(2, StatusItem_S("Error Resistance")),
                std::make_pair(3, StatusItem_S("Error Inductance")),
                std::make_pair(4, StatusItem_S("Error Pole Pair Detection")),
                std::make_pair(5, StatusItem_S("Error Setup"))};

        const std::unordered_map<bitPos, StatusItem_S> bridgeError =
            std::unordered_map<bitPos, StatusItem_S>{
                std::make_pair(0, StatusItem_S("None")),
                std::make_pair(1, StatusItem_S("Error Communication")),
                std::make_pair(2, StatusItem_S("Error Overcurrent")),
                std::make_pair(3, StatusItem_S("Error General Fault"))  //
            };

        const std::unordered_map<bitPos, StatusItem_S> hardwareError =
            std::unordered_map<bitPos, StatusItem_S>{
                std::make_pair(0, StatusItem_S("None")),
                std::make_pair(1, StatusItem_S("Error Over Current")),
                std::make_pair(2, StatusItem_S("Error Over Voltage")),
                std::make_pair(3, StatusItem_S("Error Under Voltage")),
                std::make_pair(4, StatusItem_S("Error Motor Temperature")),
                std::make_pair(5, StatusItem_S("Error Mosfet Temperature")),
                std::make_pair(6, StatusItem_S("Error ADC Current Offset"))};

        const std::unordered_map<bitPos, StatusItem_S> communicationError =
            std::unordered_map<bitPos, StatusItem_S>{
                std::make_pair(0, StatusItem_S("None")),
                std::make_pair(30, StatusItem_S("Warning CAN Watchdog"))};

        const std::unordered_map<bitPos, StatusItem_S> motionErrors =
            std::unordered_map<bitPos, StatusItem_S>{
                std::make_pair(0, StatusItem_S("None")),
                std::make_pair(1, StatusItem_S("Error Position Limit")),
                std::make_pair(2, StatusItem_S("Error Velocity Limit")),
                std::make_pair(24, StatusItem_S("Warning Acceleration")),
                std::make_pair(25, StatusItem_S("Warning Torque")),
                std::make_pair(26, StatusItem_S("Warning Velocity")),
                std::make_pair(27, StatusItem_S("Warning Position"))  //
            };

        static std::vector<std::string> getErrorsString(
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

        static void toMap(u32 bytes, const std::unordered_map<bitPos, StatusItem_S>& map)
        {
            for (auto& bit : map)
            {
                bit.second.set(static_cast<bool>(bytes & (1 << bit.first)));
            }
        }
    };
}  // namespace mab