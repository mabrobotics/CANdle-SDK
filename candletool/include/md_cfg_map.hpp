#pragma once

#include "logger.hpp"
#include "mab_types.hpp"
#include "md_types.hpp"
#include "MD_strings.hpp"
#include "mini/ini.h"
#include "utilities.hpp"

#include <cctype>
#include <optional>
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
        static constexpr std::string_view MIN_SUFFIX = "_min";
        static constexpr std::string_view MAX_SUFFIX = "_max";
        struct ParserFunctions_S
        {
            using toReadable_t = const std::function<std::string(std::string_view)>;
            using fromReadable_t =
                const std::function<std::optional<std::string>(std::string_view)>;
            using verify_t = const std::function<std::optional<std::string>(std::string_view)>;

            toReadable_t   m_toReadable;
            fromReadable_t m_fromReadable;
            verify_t       m_verify;
            ParserFunctions_S()
                : m_toReadable([](std::string_view value) { return std::string(value); }),
                  m_fromReadable([](std::string_view value) { return std::string(value); }),
                  m_verify([](std::string_view value) -> std::optional<std::string> { return {}; })
            {
            }

            ParserFunctions_S(toReadable_t toReadable, fromReadable_t fromReadable, verify_t verify)
                : m_toReadable(toReadable), m_fromReadable(fromReadable), m_verify(verify)
            {
            }

            ParserFunctions_S(verify_t verify)
                : m_toReadable([](std::string_view value) { return std::string(value); }),
                  m_fromReadable([](std::string_view value) { return std::string(value); }),
                  m_verify(verify)
            {
            }
        };

        const std::string_view m_tomlSection;
        const std::string_view m_tomlKey;

        const ParserFunctions_S m_parserFunctions;

        std::string m_value;

        MDCfgElement(std::string_view&& section,
                     std::string_view&& key,
                     ParserFunctions_S  parserFunctions = ParserFunctions_S())
            : m_tomlSection(section),
              m_tomlKey(key),
              m_parserFunctions(parserFunctions),
              m_value("")
        {
        }

        std::string getReadable() const
        {
            if (!m_value.empty() && m_parserFunctions.m_verify(m_value).has_value())
            {
                Logger logger(Logger::ProgramLayer_E::TOP, "Config Parser");
                logger.error("%s", m_parserFunctions.m_verify(m_value).value().c_str());
            }
            return m_parserFunctions.m_toReadable(m_value);
        }
        [[nodiscard("Info on parsing state")]] bool setFromReadable(
            const std::string_view value) noexcept
        {
            if (m_parserFunctions.m_fromReadable(value).has_value())
            {
                if (!value.empty() && m_parserFunctions.m_verify(value).has_value())
                {
                    Logger logger(Logger::ProgramLayer_E::TOP, "Config Parser");
                    logger.error("%s", m_parserFunctions.m_verify(value).value().c_str());
                }
                m_value = m_parserFunctions.m_fromReadable(value).value();
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
        std::optional<mINI::INIStructure> m_mdConfigSchema;

      public:
        MDConfigMap(std::optional<mINI::INIStructure> mdConfigSchema = std::nullopt)
            : m_mdConfigSchema(mdConfigSchema)
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

        // ADD NEW CONFIGURATION PARAMETERS HERE
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
            {0x01E,
             MDCfgElement("motor",
                          "calibration mode",
                          MDCfgElement::ParserFunctions_S(mainEncoderCalibrationModeToReadable,
                                                          mainEncoderCalibrationModeFromReadable,
                                                          verifyPlaceholder))},
            {0x808, MDCfgElement("motor", "shutdown temp")},
            {0x600, MDCfgElement("motor", "reverse direction")},

            // Encoder parameters
            {0x020,
             MDCfgElement("output encoder",
                          "output encoder",
                          MDCfgElement::ParserFunctions_S(
                              encoderToReadable, encoderFromReadable, verifyPlaceholder))},
            {0x025,
             MDCfgElement("output encoder",
                          "output encoder mode",
                          MDCfgElement::ParserFunctions_S(
                              encoderModeToReadable, encoderModeFromReadable, verifyPlaceholder))},
            {0x026,
             MDCfgElement("output encoder",
                          "output encoder calibration mode",
                          MDCfgElement::ParserFunctions_S(encoderCalibrationModeToReadable,
                                                          encoderCalibrationModeFromReadable,
                                                          verifyPlaceholder))},

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
            {0x160,
             MDCfgElement("GPIO",
                          "mode",
                          MDCfgElement::ParserFunctions_S(
                              GPIOModeToReadable, GPIOModeFromReadable, verifyPlaceholder))}};

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

        // TODO remove placeholder
        const MDCfgElement::ParserFunctions_S::verify_t verifyPlaceholder =
            [](std::string_view value) -> std::optional<std::string> { return {}; };

        // special cases for parsing
        const std::function<std::string(std::string_view)> encoderToReadable =
            [](const std::string_view value) -> std::string
        {
            std::string output = trim(value);
            return MDAuxEncoderValue_S::toReadable(std::stoll(value.data())).value_or("NOT FOUND");
        };

        const std::function<std::optional<std::string>(const std::string_view)>
            encoderFromReadable = [](const std::string_view value) -> std::optional<std::string>
        {
            std::string output = trim(value);
            return std::to_string(MDAuxEncoderValue_S::toNumeric(output).value_or(0));
        };

        const std::function<std::string(std::string_view)> mainEncoderCalibrationModeToReadable =
            [](const std::string_view value) -> std::string
        {
            std::string output = trim(value);
            return MDMainEncoderCalibrationModeValue_S::toReadable(std::stoll(output))
                .value_or("NOT FOUND");
        };
        const std::function<std::optional<std::string>(const std::string_view)>
            mainEncoderCalibrationModeFromReadable =
                [](const std::string_view value) -> std::optional<std::string>
        {
            std::string output = trim(value);
            return std::to_string(
                MDMainEncoderCalibrationModeValue_S::toNumeric(value.data()).value_or(0));
        };

        const std::function<std::string(std::string_view)> auxEncoderCalibrationModeToReadable =
            [](const std::string_view value) -> std::string
        {
            std::string output = trim(value);
            return MDAuxEncoderCalibrationModeValue_S::toReadable(std::stoll(output))
                .value_or("NOT FOUND");
        };
        const std::function<std::optional<std::string>(const std::string_view)>
            auxEncoderCalibrationModeFromReadable =
                [](const std::string_view value) -> std::optional<std::string>
        {
            std::string output = trim(value);
            return std::to_string(
                MDAuxEncoderCalibrationModeValue_S::toNumeric(value.data()).value_or(0));
        };

        const std::function<std::string(std::string_view)> encoderModeToReadable =
            [](const std::string_view value) -> std::string
        {
            std::string output = trim(value);
            return MDAuxEncoderModeValue_S::toReadable(std::stoll(output)).value_or("NOT FOUND");
        };
        const std::function<std::optional<std::string>(const std::string_view)>
            encoderModeFromReadable = [](const std::string_view value) -> std::optional<std::string>
        {
            std::string output = trim(value);
            return std::to_string(MDAuxEncoderModeValue_S::toNumeric(value.data()).value_or(0));
        };

        const std::function<std::string(std::string_view)> encoderCalibrationModeToReadable =
            [](const std::string_view value) -> std::string
        {
            std::string output = trim(value);
            return MDAuxEncoderCalibrationModeValue_S::toReadable(std::stoll(output))
                .value_or("NOT FOUND");
        };
        const std::function<std::optional<std::string>(const std::string_view)>
            encoderCalibrationModeFromReadable =
                [](const std::string_view value) -> std::optional<std::string>
        {
            std::string output = trim(value);
            return std::to_string(
                MDAuxEncoderCalibrationModeValue_S::toNumeric(value.data()).value_or(0));
        };

        const std::function<std::string(std::string_view)> GPIOModeToReadable =
            [](const std::string_view value) -> std::string
        {
            std::string output = trim(value);
            return MDUserGpioConfigurationValue_S::toReadable(std::stoll(value.data()))
                .value_or("NOT FOUND");
        };
        const std::function<std::optional<std::string>(const std::string_view)>
            GPIOModeFromReadable = [](const std::string_view value) -> std::optional<std::string>
        {
            std::string output = trim(value);
            return std::to_string(MDUserGpioConfigurationValue_S::toNumeric(output).value_or(0));
        };

      private:
        MDRegisters_S registers;  // only for verification purposes
    };

}  // namespace mab
