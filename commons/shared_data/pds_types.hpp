#pragma once

#include "mab_types.hpp"

namespace mab
{

    // TODO: full names of board types
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
        List of whole properties IDs that could be referenced in protocol.
        The list is global and not per-module to simplify unique indexing
        and avoid situation whet the same property will have different IDs
        in different modules.
    */
    enum class propertyId_E
    {

        STATUS_WORD        = 0x00,
        STATUS_CLEAR       = 0x01,
        ENABLE             = 0x02,
        TEMPERATURE        = 0x03,
        TEMPERATURE_LIMIT  = 0x04,
        BUS_VOLTAGE        = 0x05,
        LOAD_CURRENT       = 0x10,
        LOAD_POWER         = 0x11,
        TOTAL_ENERGY       = 0x12,
        CAN_ID             = 0x20,
        CAN_BAUDRATE       = 0x21,
        SOCKET_1_MODULE    = 0x22,
        SOCKET_2_MODULE    = 0x23,
        SOCKET_3_MODULE    = 0x24,
        SOCKET_4_MODULE    = 0x25,
        SOCKET_5_MODULE    = 0x26,
        SOCKET_6_MODULE    = 0x27,
        BR_SOCKET_INDEX    = 0x30,
        BR_TRIGGER_VOLTAGE = 0x31,
        OCD_LEVEL          = 0x40,
        OCD_DELAY          = 0x41,

    };

}  // namespace mab