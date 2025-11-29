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
    struct manufacturerData_S
    {
        u32          CRC32;
        u16          version;
        deviceType_E deviceType;
        u8           deviceRevision;
        u8           batchCode[24];

        inline bool isEmpty() const
        {
            return version == 0xffff;
        }
    };
#pragma pack(pop)

}  // namespace mab
