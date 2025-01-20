#pragma once

#include "mab_types.hpp"

namespace mab
{
    enum class msgResponse_E : u8
    {
        OK                          = 0x00,
        UNKNOWN_ERROR               = 0x01,
        INVALID_MSG_BODY            = 0x02,
        INVALID_MODULE_TYPE         = 0x03,
        NO_MODULE_TYPE_AT_SOCKET    = 0x04,
        WRONG_MODULE_TYPE_AT_SOCKET = 0x05,
        MODULE_PROPERTY_ERROR       = 0x06,
    };

    /**
     * @brief Property access operation results
     *
     */
    enum class propertyError_E
    {
        OK                     = 0x00,
        PROPERTY_NOT_AVAILABLE = 0x01,
        INVALID_ACCESS         = 0x02,
        INVALID_DATA           = 0x03
    };

    enum class moduleType_E : u8
    {
        UNDEFINED = 0x00,
        CONTROL_BOARD,
        BRAKE_RESISTOR,
        ISOLATED_CONVERTER,
        POWER_STAGE,

        /* NEW MODULE TYPES HERE */
        OUT_OF_RANGE,
    };

    enum class socketIndex_E : u8
    {

        UNASSIGNED = 0x00,

        SOCKET_1 = 0x01,
        SOCKET_2 = 0x02,
        SOCKET_3 = 0x03,
        SOCKET_4 = 0x04,
        SOCKET_5 = 0x05,
        SOCKET_6 = 0x06,

        SOCKET_INDEX_MAX = 0x06,
    };

    enum class accessRights_E
    {
        READ_ONLY  = 0x00,
        READ_WRITE = 0x01,
        WRITE_ONLY = 0x02,
    };

    /*
        List of property IDs that could be referenced in protocol.
        The list is global and not per-module to simplify unique indexing
        and avoid situation whet the same property will have different IDs
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
        BR_SOCKET_INDEX          = 0x30,
        BR_TRIGGER_VOLTAGE       = 0x31,
        OCD_LEVEL                = 0x40,
        OCD_DELAY                = 0x41,

    };

    enum class statusBits_E : u32
    {
        ENABLED          = (1 << 0),
        OVER_TEMPERATURE = (1 << 1),
        OVER_CURRENT     = (1 << 2),

        /*...*/

        STO_1             = (1 << 10),
        STO_2             = (1 << 11),
        FDCAN_TIMEOUT     = (1 << 12),
        SUBMODULE_1_ERROR = (1 << 13),
        SUBMODULE_2_ERROR = (1 << 14),
        SUBMODULE_3_ERROR = (1 << 15),
        SUBMODULE_4_ERROR = (1 << 16),
        SUBMODULE_5_ERROR = (1 << 17),
        SUBMODULE_6_ERROR = (1 << 18),
        CHARGER_DETECTED  = (1 << 19),

        /*...*/

    };

    struct status_S
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
        /*...*/
    };

    // TODO: This enum should be replaced with a single one for all MAB Codebase ( The one that is
    // used in CANdle )
    enum class canBaudrate_E : u8
    {
        BAUD_1M = 0x00,
        BAUD_2M = 0x01,
        BAUD_5M = 0x02,
        BAUD_8M = 0x03
    };

    struct pdsConfig_S
    {
        u16           canId;
        canBaudrate_E canBaudrate;
    };

}  // namespace mab