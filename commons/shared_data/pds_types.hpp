#pragma once

#include "mab_types.hpp"
#include "pds_properties.hpp"
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
        INVALID_DATA           = 0x03,
    };

    enum class moduleType_E : u8
    {
        UNDEFINED = 0x00,
        CONTROL_BOARD,
        BRAKE_RESISTOR,
        ISOLATED_CONVERTER,
        POWER_STAGE,

        /* NEW MODULE TYPES HERE */
    };

    enum class moduleVersion_E : uint8_t
    {
        UNKNOWN = 0x00,
        V0_1,  // 0.1
        V0_2,  // 0.2
        V0_3,  // 0.3
        /* NEW MODULE VERSIONS HERE */
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

    };

    struct __attribute__((packed)) pdsFwMetadata_S
    {
        u8         metadataStructVersion;  // Version of this structure. Keep this always first!
        version_ut version;                // Version of the firmware
        char       gitHash[8];             // 8 characters of git hash
    };

}  // namespace mab
