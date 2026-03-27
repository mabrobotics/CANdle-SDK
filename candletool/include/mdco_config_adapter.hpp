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
#include <vector>

namespace mab
{
    struct MDCOConfigAdapter
    {
        std::vector<std::reference_wrapper<EDSEntry>> configToOd(
            MDConfigMap& config, std::shared_ptr<EDSObjectDictionary> od);
        void configFromOd(std::shared_ptr<EDSObjectDictionary> od, MDConfigMap& config);

        static constexpr auto toMili = [](std::string_view x) -> std::string
        {
            i64 xInt;
            std::from_chars(x.begin(), x.end(), xInt);
            return std::to_string(static_cast<i64>(xInt * 1000));
        };
        static constexpr auto fromMili = [](std::string_view x) -> std::string
        {
            float xFloat;
            std::from_chars(x.begin(), x.end(), xFloat);
            return std::to_string(xFloat / 1000.f);
        };

        static constexpr auto toEncTick = [](std::string_view x) -> std::string
        {
            i64 xInt;
            std::from_chars(x.begin(), x.end(), xInt);
            return std::to_string(static_cast<i64>(xInt * 16384 / (2 * M_PI)));
        };
        static constexpr auto fromEncTick = [](std::string_view x) -> std::string
        {
            f32 xFloat;
            std::from_chars(x.begin(), x.end(), xFloat);
            return std::to_string(xFloat * 2 * M_PI / 16384.f);
        };

        static constexpr auto toRPM = [](std::string_view x) -> std::string
        {
            i64 xInt;
            std::from_chars(x.begin(), x.end(), xInt);
            return std::to_string(static_cast<i64>(xInt * 60.f / (2 * M_PI)));
        };
        static constexpr auto fromRPM = [](std::string_view x) -> std::string
        {
            f32 xFloat;
            std::from_chars(x.begin(), x.end(), xFloat);
            return std::to_string(xFloat * 2 * M_PI / 60.f);
        };

        MDCOConfigAdapter()
            : cfgToOdUnitConversions({{0x016, toMili},
                                      {0x112, toMili},
                                      {0x110, toEncTick},
                                      {0x111, toEncTick},
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
                                      {0x111, fromEncTick},
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
                {0x01E, "Calibration Mode", {}},
                {0x808, "Motor Shutdown Temperature", {}},
                // {0x600, "Reverse Direction", {}}, removed for safety

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

                {0x113, 0x6080, {}},  // Max velocity
                {0x114, 0x60C5, {}},  // Max acceleration
                {0x115, 0x60C6, {}},  // Max deceleration

                {0x120, 0x6081, {}},  // Profile velocity
                {0x121, 0x6083, {}},  // Profile acceleration
                {0x122, 0x6084, {}},  // Profile deceleration
                {0x123, 0x6085, {}}   // Quick stop deceleration
            });

        const std::unordered_map<u16, std::function<std::string(std::string_view)>>
            cfgToOdUnitConversions;
        const std::unordered_map<u16, std::function<std::string(std::string_view)>>
            odToCfgUnitConversions;
    };

}  // namespace mab
