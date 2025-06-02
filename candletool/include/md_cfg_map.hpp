#pragma once

#include "mab_types.hpp"
#include "md_types.hpp"
#include "candletool.hpp"

#include <stdexcept>
#include <map>
#include <string>
#include <memory>
#include <string_view>

namespace mab
{
    struct MDCfgElement
    {
      public:
        const std::string_view m_tomlSection;
        const std::string_view m_tomlKey;

        const std::function<std::string(std::string_view)>                m_toReadable;
        const std::function<std::optional<std::string>(std::string_view)> m_fromReadable;

        std::string m_value;

        MDCfgElement(
            std::string_view&&                                 section,
            std::string_view&&                                 key,
            std::function<std::string(const std::string_view)> toReadable =
                [](std::string_view value) { return std::string(value); },
            std::function<std::optional<std::string>(const std::string_view)> fromReadable =
                [](std::string_view value) { return std::string(value); })
            : m_tomlSection(section),
              m_tomlKey(key),
              m_toReadable(toReadable),
              m_fromReadable(fromReadable),
              m_value("")
        {
        }

        std::string getReadable() const
        {
            return m_toReadable(m_value);
        }
        [[nodiscard("Info on parsing state")]] bool setFromReadable(
            const std::string_view value) noexcept
        {
            if (m_fromReadable(value).has_value())
            {
                m_value = m_fromReadable(value).value();
                return true;
            }
            else
            {
                return false;
            }
        }
    };

    class MDConfigMap
    {
      public:
        MDConfigMap()
        {
            // Verify that all the keys are actual registers

            for (const auto& cfgPair : m_map)
            {
                bool      foundInRegisters = false;
                const u16 lookingFor       = cfgPair.first;
                auto      findInRegisters =
                    [&foundInRegisters, lookingFor]<typename T>(MDRegisterEntry_S<T>& reg)
                {
                    if (reg.m_regAddress == lookingFor)
                        foundInRegisters = true;
                };
                registers.forEachRegister(findInRegisters);
                if (!foundInRegisters)
                {
                    throw std::runtime_error(
                        "MDConfigMap: Key '" + std::to_string(lookingFor) +
                        "' not found in MD registers. Please check the configuration map.");
                }
            }
        }
        ~MDConfigMap() = default;
        std::map<u16, MDCfgElement> m_map{
            // Motor parameters
            {0x010, MDCfgElement("motor", "name")},
            {0x011, MDCfgElement("motor", "pole pairs")},
            {0x012, MDCfgElement("motor", "torque constant")},
            {0x016, MDCfgElement("motor", "max current")},
            {0x017, MDCfgElement("motor", "gear ratio")},
            {0x018, MDCfgElement("motor", "torque bandwidth")},
            {0x01D, MDCfgElement("motor", "KV")},
            {0x013, MDCfgElement("motor", "torque constant a")},
            {0x014, MDCfgElement("motor", "torque constant b")},
            {0x015, MDCfgElement("motor", "torque constant c")},
            {0x019, MDCfgElement("motor", "dynamic friction")},
            {0x01A, MDCfgElement("motor", "static friction")},
            {0x01E, MDCfgElement("motor", "calibration mode")},
            {0x808, MDCfgElement("motor", "shutdown temp")},
            {0x600, MDCfgElement("motor", "reverse direction")},

            // Encoder parameters
            {0x020,
             MDCfgElement(
                 "output encoder", "output encoder", encoderToReadable, encoderFromReadable)},
            {0x022, MDCfgElement("output encoder", "output encoder default baud")},
            {0x025,
             MDCfgElement("output encoder",
                          "output encoder mode",
                          encoderModeToReadable,
                          encoderModeFromReadable)},
            {0x026, MDCfgElement("output encoder", "output encoder calibration mode")},

            // PID parameters
            {0x030, MDCfgElement("position PID", "kp")},
            {0x031, MDCfgElement("position PID", "ki")},
            {0x032, MDCfgElement("position PID", "kd")},
            {0x034, MDCfgElement("position PID", "windup")},
            {0x040, MDCfgElement("velocity PID", "kp")},
            {0x041, MDCfgElement("velocity PID", "ki")},
            {0x042, MDCfgElement("velocity PID", "kd")},
            {0x044, MDCfgElement("velocity PID", "windup")},
            {0x050, MDCfgElement("impedance PD", "kp")},
            {0x051, MDCfgElement("impedance PD", "kd")},

            // Limits
            {0x112, MDCfgElement("limits", "max torque")},
            {0x110, MDCfgElement("limits", "max position")},
            {0x111, MDCfgElement("limits", "min position")},
            {0x113, MDCfgElement("limits", "max velocity")},
            {0x114, MDCfgElement("limits", "max acceleration")},
            {0x115, MDCfgElement("limits", "max deceleration")},

            // Motion profile
            {0x120, MDCfgElement("profile", "velocity")},
            {0x121, MDCfgElement("profile", "acceleration")},
            {0x122, MDCfgElement("profile", "deceleration")},
            {0x123, MDCfgElement("profile", "quick stop deceleration")},

            // Hardware configuration
            {0x700, MDCfgElement("hardware", "shunt resistance")},
            {0x160, MDCfgElement("GPIO", "mode", GPIOModeToReadable, GPIOModeFromReadable)}};

        // Function to get the value of a specific register by address
        std::string getValueByAddress(u16 address) const
        {
            auto it = m_map.find(address);
            if (it != m_map.end())
            {
                return it->second.m_value;
            }
            throw std::runtime_error("MDConfigMap: Address " + std::to_string(address) +
                                     " not found in configuration map.");
        }

        // Function to set the value of a specific register by address
        void setValueByAddress(u16 address, const std::string& value)
        {
            auto it = m_map.find(address);
            if (it != m_map.end())
            {
                it->second.m_value = value;
            }
            else
            {
                throw std::runtime_error("MDConfigMap: Address " + std::to_string(address) +
                                         " not found in configuration map.");
            }
        }

        // special cases for parsing
        static const std::function<std::string(std::string_view)> encoderToReadable;
        static const std::function<std::optional<std::string>(const std::string_view)>
            encoderFromReadable;

        static const std::function<std::string(std::string_view)> encoderModeToReadable;
        static const std::function<std::optional<std::string>(const std::string_view)>
            encoderModeFromReadable;

        static const std::function<std::string(std::string_view)> GPIOModeToReadable;
        static const std::function<std::optional<std::string>(const std::string_view)>
            GPIOModeFromReadable;

      private:
        MDRegisters_S registers;  // only for verification purposes
    };

    // Special case for encoder type
    inline const std::function<std::optional<std::string>(const std::string_view)>
        MDConfigMap::encoderFromReadable =
            [](const std::string_view value) -> std::optional<std::string>
    {
        if (value == "NONE" || value == "0")
            return "0";
        if (value == "ME_AS_CENTER" || value == "1")
            return "1";
        if (value == "ME_AS_OFFAXIS" || value == "2")
            return "2";
        if (value == "MB053SFA17BENT00" || value == "3")
            return "3";
        if (value == "CM_OFFAXIS" || value == "4")
            return "4";
        if (value == "M24B_CENTER" || value == "5")
            return "5";
        if (value == "M24B_OFFAXIS" || value == "6")
            return "6";

        return std::nullopt;  // Return nullopt if the value is not recognized
    };

    inline const std::function<std::string(std::string_view)> MDConfigMap::encoderToReadable =
        [](const std::string_view value) -> std::string
    {
        if (value == "0")
            return "NONE";
        if (value == "1")
            return "ME_AS_CENTER";
        if (value == "2")
            return "ME_AS_OFFAXIS";
        if (value == "3")
            return "MB053SFA17BENT00";
        if (value == "4")
            return "CM_OFFAXIS";
        if (value == "5")
            return "M24B_CENTER";
        if (value == "6")
            return "M24B_OFFAXIS";

        return "UNKNOWN: " + std::string(value);
    };

    // Special case for encoder mode
    inline const std::function<std::string(std::string_view)> MDConfigMap::encoderModeToReadable =
        [](const std::string_view value) -> std::string
    {
        if (value == "0")
            return "NONE";
        if (value == "1")
            return "STARTUP";
        if (value == "2")
            return "MOTION";
        if (value == "3")
            return "REPORT";
        if (value == "4")
            return "MAIN";
        if (value == "5")
            return "CALIBRATED_REPORT";

        return "UNKNOWN: " + std::string(value);
    };

    inline const std::function<std::optional<std::string>(const std::string_view)>
        MDConfigMap::encoderModeFromReadable =
            [](const std::string_view value) -> std::optional<std::string>
    {
        if (value == "NONE" || value == "0")
            return "0";
        if (value == "STARTUP" || value == "1")
            return "1";
        if (value == "MOTION" || value == "2")
            return "2";
        if (value == "REPORT" || value == "3")
            return "3";
        if (value == "MAIN" || value == "4")
            return "4";
        if (value == "CALIBRATED_REPORT" || value == "5")
            return "5";

        return std::nullopt;  // Return nullopt if the value is not recognized
    };

    // Special case for GPIO mode
    inline const std::function<std::string(std::string_view)> MDConfigMap::GPIOModeToReadable =
        [](const std::string_view value) -> std::string
    {
        if (value == "0")
            return "OFF";
        if (value == "1")
            return "AUTO-BRAKE";
        if (value == "2")
            return "INPUT";

        return "UNKNOWN: " + std::string(value);
    };
    inline const std::function<std::optional<std::string>(const std::string_view)>
        MDConfigMap::GPIOModeFromReadable =
            [](const std::string_view value) -> std::optional<std::string>
    {
        if (value == "OFF")
            return "0";
        if (value == "AUTO-BRAKE")
            return "1";
        if (value == "INPUT")
            return "2";

        return std::nullopt;  // Return nullopt if the value is not recognized
    };
}  // namespace mab
