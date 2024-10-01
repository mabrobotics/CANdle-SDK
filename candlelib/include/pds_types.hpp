#pragma once

#include <stdint.h>

namespace mab
{

    // PDS pluggable module type
    enum class moduleType_E : uint8_t
    {
        // Undefined means that module is not connected or PDS could not determine its type.
        UNDEFINED              = 0x00,
        BRAKE_RESISTOR         = 0x01,
        ISOLATED_CONVERTER_12V = 0x02,
        ISOLATED_CONVERTER_5V  = 0x03,
        POWER_STAGE            = 0x04,
        /* NEW MODULE TYPES HERE */
    };

    // Pluggable type socket index
    enum class socketIndex_E : uint8_t
    {
        SOCKET_1 = 0x00,
        SOCKET_2 = 0x01,
        SOCKET_3 = 0x02,
        SOCKET_4 = 0x03,
        SOCKET_5 = 0x04,
        SOCKET_6 = 0x05,
    };

}  // namespace mab