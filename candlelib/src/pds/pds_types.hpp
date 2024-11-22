#pragma once

#include <stdint.h>

namespace mab
{

    // PDS pluggable module type
    enum class moduleType_E : uint8_t
    {
        // Undefined means that module is not connected or PDS could not determine its type.
        UNDEFINED              = 0x00,
        CONTROL_BOARD          = 0x01,
        BRAKE_RESISTOR         = 0x02,
        ISOLATED_CONVERTER_12V = 0x03,
        ISOLATED_CONVERTER_5V  = 0x04,
        POWER_STAGE            = 0x05,
        /* NEW MODULE TYPES HERE */
    };

    // Pluggable type socket index
    enum class socketIndex_E : uint8_t
    {
        UNASSIGNED = 0x00,

        SOCKET_1 = 0x01,
        SOCKET_2 = 0x02,
        SOCKET_3 = 0x03,
        SOCKET_4 = 0x04,
        SOCKET_5 = 0x05,
        SOCKET_6 = 0x06,
    };

}  // namespace mab