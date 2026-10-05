#pragma once

#include "logger.hpp"
#include "MDObjects.hpp"
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
            i64 xInt = 0;
            std::from_chars(x.begin(), x.end(), xInt);
            return std::to_string(static_cast<i64>(xInt * 1000));
        };
        static constexpr auto fromMili = [](std::string_view x) -> std::string
        {
            float xFloat = 0.f;
            std::from_chars(x.begin(), x.end(), xFloat);
            return std::to_string(xFloat / 1000.f);
        };

        static constexpr auto toEncTick = [](std::string_view x) -> std::string
        {
            i64 xInt = 0;
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
            f32 xFloat = 0.f;
            std::from_chars(x.begin(), x.end(), xFloat);
            return std::to_string(static_cast<i64>(xFloat * 60.f / (2 * M_PI)));
        };
        static constexpr auto fromRPM = [](std::string_view x) -> std::string
        {
            i64 xInt = 0;
            std::from_chars(x.begin(), x.end(), xInt);
            return std::to_string(xInt * 2 * M_PI / 60.f);
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

        // Objects are addressed through md_objects, which resolves them by index and confirms
        // the identity by name, so a renamed or renumbered .eds revision still maps correctly.
        // Registers with no CANopen counterpart are absent here on purpose:
        //   0x012 torque constant, 0x01D KV - the drive derives these from rated torque/current
        //   0x700 shunt resistance          - deprecated, kept in the .cfg for older files
        static constexpr auto manufacturerRegMaping =
            std::to_array<std::pair<u16, md_objects::ObjectRef>>({
                {0x010, md_objects::MOTOR_NAME},
                {0x011, md_objects::POLE_PAIRS},
                {0x018, md_objects::TORQUE_BANDWIDTH_CFG},
                {0x01E, md_objects::MOTOR_CALIBRATION_MODE},
                {0x808, md_objects::MOTOR_SHUTDOWN_TEMP},
                // {0x600, "Reverse Direction"}, removed for safety

                {0x02A, md_objects::MAIN_ENCODER_TYPE},
                {0x02B, md_objects::MAIN_ENCODER_DIR},

                {0x020, md_objects::AUX_ENCODER_TYPE},
                {0x025, md_objects::AUX_ENCODER_MODE},
                {0x026, md_objects::AUX_ENCODER_CALIBRATION_MODE},

                {0x200, md_objects::TORQUE_SENSOR_TYPE},

                {0x030, md_objects::POSITION_PID_KP},
                {0x031, md_objects::POSITION_PID_KI},
                {0x032, md_objects::POSITION_PID_KD},
                {0x034, md_objects::POSITION_PID_WINDUP},

                {0x040, md_objects::VELOCITY_PID_KP},
                {0x041, md_objects::VELOCITY_PID_KI},
                {0x042, md_objects::VELOCITY_PID_KD},
                {0x044, md_objects::VELOCITY_PID_WINDUP},

                {0x050, md_objects::IMPEDANCE_PD_KP},
                {0x051, md_objects::IMPEDANCE_PD_KD},

                {0x160, md_objects::GPIO_MODE},
            });

        static constexpr auto standardRegMaping =
            std::to_array<std::tuple<u16, u16, std::optional<u8>>>({
                {0x112, 0x6072, {}},  // Motor max torque 
                {0x016, 0x6073, {}},  // Motor max current
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
