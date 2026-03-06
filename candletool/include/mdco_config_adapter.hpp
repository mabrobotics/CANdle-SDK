#pragma once

#include "logger.hpp"
#include "md_cfg_map.hpp"
#include "edsEntry.hpp"
#include "MDCO.hpp"

#include <array>
#include <charconv>
#include <cmath>
#include <functional>
#include <string>
#include <string_view>

namespace mab
{
    struct MDCOConfigAdapter
    {
        enum class Error_t
        {
            UNKNOWN,
            OK,
            PARSING_FAILED,
            TRANSMISSION_FAILED
        };
        inline static Error_t sendConfigToMDCO(const MDConfigMap&   config,
                                               EDSObjectDictionary& od,
                                               MDCO&                mdco)
        {
            Logger log(Logger::ProgramLayer_E::TOP, "MDCO CFG");
            // Manufacturer part parsing
            for (const auto& [regAddr, objName, subIdx] : manufacturerRegMaping)
            {
                const std::string cfgValue = config.getValueByAddress(regAddr);
                if (cfgValue.empty())
                {
                    log.warn("Manufacturer register %s not found in config", objName.data());
                    continue;
                }
                auto objOpt = od.getEntryByName(objName);
                if (!objOpt.has_value())
                {
                    log.warn("Manufacturer register %s not found in OD - skipping", objName.data());
                    continue;
                }
                auto& obj = objOpt.value().get();
                if (obj.setFromString(cfgValue) != EDSEntry::Error_t::OK)
                {
                    log.error("Failed to set value for manufacturer register %s", objName.data());
                    return od;
                }
            }

            // Parse standard CANopen registers with unit conversion
            for (const auto& [cfgAddr, odAddr, subIdx] : standardRegMaping)
            {
                try
                {
                    std::string cfgValue = config.getValueByAddress(cfgAddr);
                    if (!cfgValue.empty())
                    {
                        // Convert from config value to OD value if conversion exists
                        f32 cfgNumVal = std::stof(cfgValue);
                        i64 odValue   = cfgNumVal;

                        // Apply unit conversion based on register address
                        if (auto it = cfgToOdUnitConversions.find(cfgAddr);
                            it != cfgToOdUnitConversions.end())
                        {
                            odValue = it->second(cfgNumVal);
                        }

                        EDSEntry& entry = od[odAddr];
                        if (subIdx.has_value())
                        {
                            entry[subIdx.value()] = odValue;
                        }
                        else
                        {
                            entry = odValue;
                        }
                    }
                }
                catch (const std::exception&)
                {
                    // Skip if register not found in config
                    continue;
                }
            }

            return od;
        }
        inline static Error_t receiveConfigFromMDCO(const EDSObjectDictionary& od,
                                                    MDConfigMap&               config,
                                                    MDCO&                      mdco)
        {
            return config;
        }

        static constexpr auto toMili = [](std::string_view x) -> std::string
        {
            f32 xFloat;
            std::from_chars(x.begin(), x.end(), xFloat);
            return std::to_string(xFloat * 1000.f);
        };
        static constexpr auto fromMili = [](std::string_view x) -> std::string
        {
            i64 xInt;
            std::from_chars(x.begin(), x.end(), xInt);
            return std::to_string(static_cast<i64>(xInt / 1000.f));
        };

        static constexpr auto toEncTick = [](std::string_view x) -> std::string
        {
            f32 xFloat;
            std::from_chars(x.begin(), x.end(), xFloat);
            return std::to_string(xFloat * 16384.f / (2 * M_PI));
        };
        static constexpr auto fromEncTick = [](std::string_view x) -> std::string
        {
            i64 xInt;
            std::from_chars(x.begin(), x.end(), xInt);
            return std::to_string(static_cast<i64>(xInt * 2 * M_PI / 16384.f));
        };

        static constexpr auto toRPM = [](std::string_view x) -> std::string
        {
            f32 xFloat;
            std::from_chars(x.begin(), x.end(), xFloat);
            return std::to_string(xFloat * 60.f / (2 * M_PI));
        };
        static constexpr auto fromRPM = [](std::string_view x) -> std::string
        {
            i64 xInt;
            std::from_chars(x.begin(), x.end(), xInt);
            return std::to_string(static_cast<i64>(xInt * 2 * M_PI / 60.f));
        };

        MDCOConfigAdapter()
            : cfgToOdUnitConversions({{0x016, toMili},
                                      {0x112, toMili},
                                      {0x110, toEncTick},
                                      {0x113, toRPM},
                                      {0x114, toRPM},
                                      {0x115, toRPM},
                                      {0x120, toRPM},
                                      {0x121, toRPM},
                                      {0x122, toRPM},
                                      {0x123, toRPM}}),
              odToCfgUnitConversions({{0x016, fromMili},
                                      {0x112, fromMili},
                                      {0x110, fromEncTick},
                                      {0x113, fromRPM},
                                      {0x114, fromRPM},
                                      {0x115, fromRPM},
                                      {0x120, fromRPM},
                                      {0x121, fromRPM},
                                      {0x122, fromRPM},
                                      {0x123, fromRPM}})
        {
        }

        static constexpr auto manufacturerRegMaping =
            std::to_array<std::tuple<u16, std::string_view, std::optional<u8>>>({
                {0x010, "Motor Name", {}},
                {0x012, "Torque constant", {}},

                {0x017, "Gear Ratio", {}},
                {0x018, "Torque Bandwidth", {}},
                {0x01D, "Torque constant", {}},
                {0x01E, "Calibration Mode", {}},
                {0x808, "Motor Shutdown Temperature", {}},
                {0x600, "Reverse Direction", {}},

                {0x020, "Output Encoder", {0x1}},
                {0x025, "Output Encoder", {0x3}},
                {0x026, "Output Encoder", {0x2}},

                {0x030, "Position PID Controller", {0x1}},
                {0x031, "Position PID Controller", {0x2}},
                {0x032, "Position PID Controller", {0x3}},
                {0x034, "Position PID Controller", {0x4}},

                {0x040, "Velocity PID Controller", {0x1}},
                {0x041, "Velocity PID Controller", {0x2}},
                {0x042, "Velocity PID Controller", {0x3}},
                {0x044, "Velocity PID Controller", {0x4}},

                {0x050, "Impedance PD Controller", {0x1}},
                {0x051, "Impedance PD Controller", {0x2}},

                {0x160, "User GPIO Configuration", {}},
            });

        static constexpr auto standardRegMaping =
            std::to_array<std::tuple<u16, u16, std::optional<u8>>>({
                {0x016, 0x6075, {}},  // Motor rated current
                {0x112, 0x6076, {}},  // Motor rated torque

                {0x110, 0x607D, 2},  // Max position
                {0x111, 0x607D, 1},  // Min position

                {0x113, 0x607F, {}},  // Max velocity
                {0x114, 0x60C5, {}},  // Max acceleration
                {0x115, 0x60C6, {}},  // Max deceleration

                {0x120, 0x6081, {}},  // Profile velocity
                {0x121, 0x6083, {}},  // Profile acceleration
                {0x122, 0x6084, {}},  // Profile deceleration
                {0x123, 0x6085, {}}   // Quick stop deceleration
            });

        const std::map<u16, std::function<std::string(std::string_view)>> cfgToOdUnitConversions;
        const std::map<u16, std::function<std::string(std::string_view)>> odToCfgUnitConversions;
    };

}  // namespace mab
