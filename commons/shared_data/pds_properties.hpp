#pragma once

#include <array>
#include <cstddef>
#include "mab_types.hpp"

namespace mab
{

    enum class accessRights_E
    {
        READ_ONLY  = 0x00,
        READ_WRITE = 0x01,
        WRITE_ONLY = 0x02,
    };

    /*
        List of each property ID that could be referenced in protocol.
        The list is global and not per-module to simplify unique indexing
        and avoid situation when the same property will have different IDs
        in different modules.
    */
    enum class propertyId_E : u8
    {

        STATUS_WORD              = 0x00,
        STATUS_CLEAR             = 0x01,
        ENABLE                   = 0x02,
        TEMPERATURE              = 0x03,
        TEMPERATURE_LIMIT        = 0x04,
        BUS_VOLTAGE              = 0x05,
        AUTOSTART                = 0x06,
        LOAD_CURRENT             = 0x10,
        LOAD_POWER               = 0x11,
        TOTAL_DELIVERED_ENERGY   = 0x12,
        TOTAL_RECUPERATED_ENERGY = 0x13,
        CAN_ID                   = 0x20,
        CAN_BAUDRATE             = 0x21,
        SOCKET_1_MODULE          = 0x22,
        SOCKET_2_MODULE          = 0x23,
        SOCKET_3_MODULE          = 0x24,
        SOCKET_4_MODULE          = 0x25,
        SOCKET_5_MODULE          = 0x26,
        SOCKET_6_MODULE          = 0x27,
        SHUTDOWN_TIME            = 0x28,
        BATTERY_VOLTAGE_L1       = 0x29,
        BATTERY_VOLTAGE_L2       = 0x2A,
        BR_SOCKET_INDEX          = 0x30,
        BR_TRIGGER_VOLTAGE       = 0x31,
        OCD_LEVEL                = 0x40,
        OCD_DELAY                = 0x41,

        /* ... */

        HW_VERSION = 0xFD,
        FW_VERSION = 0xFE,
        COMMAND    = 0xFF,  // Used for sending various commands to PDS Device

    };

    enum class commands_E : u8
    {

        NULL_CMD    = 0x00,
        SHUTDOWN    = 0x01,
        REBOOT      = 0x02,
        SAVE_CONFIG = 0x03,

    };

    enum class statusBits_E : u32
    {

        ENABLED          = (1 << 0),
        OVER_TEMPERATURE = (1 << 1),
        OVER_CURRENT     = (1 << 2),

        /*...*/

        STO_1              = (1 << 10),
        STO_2              = (1 << 11),
        FDCAN_TIMEOUT      = (1 << 12),
        SUBMODULE_1_ERROR  = (1 << 13),
        SUBMODULE_2_ERROR  = (1 << 14),
        SUBMODULE_3_ERROR  = (1 << 15),
        SUBMODULE_4_ERROR  = (1 << 16),
        SUBMODULE_5_ERROR  = (1 << 17),
        SUBMODULE_6_ERROR  = (1 << 18),
        CHARGER_DETECTED   = (1 << 19),
        SHUTDOWN_SCHEDULED = (1 << 20),

        /*...*/

    };

    constexpr std::array<std::pair<propertyId_E, size_t>, 29> propertiesSizeArray = {
        std::make_pair(propertyId_E::STATUS_WORD, sizeof(u32)),
        std::make_pair(propertyId_E::STATUS_CLEAR, sizeof(u32)),
        std::make_pair(propertyId_E::ENABLE, sizeof(bool)),
        std::make_pair(propertyId_E::TEMPERATURE, sizeof(f32)),
        std::make_pair(propertyId_E::TEMPERATURE_LIMIT, sizeof(f32)),
        std::make_pair(propertyId_E::BUS_VOLTAGE, sizeof(u32)),
        std::make_pair(propertyId_E::AUTOSTART, sizeof(bool)),
        std::make_pair(propertyId_E::LOAD_CURRENT, sizeof(s32)),
        std::make_pair(propertyId_E::LOAD_POWER, sizeof(u32)),
        std::make_pair(propertyId_E::TOTAL_DELIVERED_ENERGY, sizeof(u32)),
        std::make_pair(propertyId_E::TOTAL_RECUPERATED_ENERGY, sizeof(u32)),
        std::make_pair(propertyId_E::CAN_ID, sizeof(u16)),
        std::make_pair(propertyId_E::CAN_BAUDRATE, sizeof(u8)),
        std::make_pair(propertyId_E::SOCKET_1_MODULE, sizeof(u8)),
        std::make_pair(propertyId_E::SOCKET_2_MODULE, sizeof(u8)),
        std::make_pair(propertyId_E::SOCKET_3_MODULE, sizeof(u8)),
        std::make_pair(propertyId_E::SOCKET_4_MODULE, sizeof(u8)),
        std::make_pair(propertyId_E::SOCKET_5_MODULE, sizeof(u8)),
        std::make_pair(propertyId_E::SOCKET_6_MODULE, sizeof(u8)),
        std::make_pair(propertyId_E::SHUTDOWN_TIME, sizeof(u32)),
        std::make_pair(propertyId_E::BATTERY_VOLTAGE_L1, sizeof(u32)),
        std::make_pair(propertyId_E::BATTERY_VOLTAGE_L2, sizeof(u32)),
        std::make_pair(propertyId_E::BR_SOCKET_INDEX, sizeof(u8)),
        std::make_pair(propertyId_E::BR_TRIGGER_VOLTAGE, sizeof(u32)),
        std::make_pair(propertyId_E::OCD_LEVEL, sizeof(u32)),
        std::make_pair(propertyId_E::OCD_DELAY, sizeof(u32)),
        std::make_pair(propertyId_E::HW_VERSION, sizeof(u8)),
        std::make_pair(propertyId_E::FW_VERSION, sizeof(u32)),
        std::make_pair(propertyId_E::COMMAND, sizeof(commands_E)),
    };

    inline size_t getPropertySize(propertyId_E propertyId)
    {
        for (const auto& property : propertiesSizeArray)
        {
            if (property.first == propertyId)
            {
                return property.second;
            }
        }
        return 0;  // Default size if not found
    }

    struct controlBoardStatus_S
    {
        bool ENABLED;
        bool OVER_TEMPERATURE;
        bool OVER_CURRENT;

        /*...*/

        bool STO_1;
        bool STO_2;
        bool FDCAN_TIMEOUT;

        bool SUBMODULE_1_ERROR;
        bool SUBMODULE_2_ERROR;
        bool SUBMODULE_3_ERROR;
        bool SUBMODULE_4_ERROR;
        bool SUBMODULE_5_ERROR;
        bool SUBMODULE_6_ERROR;
        bool CHARGER_DETECTED;
        bool SHUTDOWN_SCHEDULED;

        /*...*/
    };

    struct powerStageStatus_S
    {
        bool ENABLED;
        bool OVER_TEMPERATURE;
        bool OVER_CURRENT;
    };

    struct brakeResistorStatus_S
    {
        bool ENABLED;
        bool OVER_TEMPERATURE;
    };

    struct isolatedConverterStatus_S
    {
        bool ENABLED;
        bool OVER_TEMPERATURE;
        bool OVER_CURRENT;
    };

}  // namespace mab