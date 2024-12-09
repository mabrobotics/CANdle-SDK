#pragma once

#include "mab_types.hpp"

/* When editing this file please iterate
version in manufacturerData_S,
and update md_station OTP-FLASH.py file
with new device map */

namespace mab
{

    enum class deviceType_E : u8
    {
        UNKNOWN_DEVICE = 0x00,
        MD80           = 0x01,
        MD20           = 0x02,
        MD80HV         = 0x03,
        PDS            = 0x04,

        UNKNOWN_DEVICE_2 = 0xA0,
        CANDLE           = 0xA1,
        CANDLE_HAT       = 0xA2
    };

#pragma pack(push, 1)

    struct hardwareType_S
    {
        deviceType_E deviceType;  // 0xAX - hosts, 0x0X - peripherals
        u8           deviceRevision;
    };

    struct manufacturerData_S
    {
        u16            version = 1;
        u8             batchCode[24];
        hardwareType_S hardwareType;
        u8             placeholder[130];
        u32            CRC32;
    };

    union manufacturerData_U
    {
        manufacturerData_S manufacturerData{};
        u8                 bytes[sizeof(manufacturerData_S)];
    };

#pragma pack(pop)

}  // namespace mab