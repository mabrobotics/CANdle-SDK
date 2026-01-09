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
        MD80DE         = 0x05,

        UNKNOWN_DEVICE_2 = 0xA0,
        CANDLE           = 0xA1,
        CANDLE_HAT       = 0xA2
    };
    inline char* deviceTypeToCstring(deviceType_E type)
    {
        switch (type)
        {
            case mab::deviceType_E::MD80:
                return (char*)"md80";
            case mab::deviceType_E::MD20:
                return (char*)"md20";
            case mab::deviceType_E::MD80HV:
                return (char*)"md80hv";
            case mab::deviceType_E::MD80DE:
                return (char*)"md80de";
            case mab::deviceType_E::CANDLE:
                return (char*)"candle";
            case mab::deviceType_E::CANDLE_HAT:
                return (char*)"candlehat";
            case mab::deviceType_E::PDS:
                return (char*)"candlehat";
            default:
                return (char*)"UNKNOWN";
        }
    }

#pragma pack(push, 1)
    struct manufacturerData_S
    {
        u32          CRC32;    // TODO: placeholder
        u16          version;  // manufacturerData format version
        deviceType_E deviceType;
        u8           deviceRevision;  // 10 -> HW1.0, 32 -> HW3.2
        u8           prodDate[6];     // ddmmyy format
        u8           batchCode[24];   // TODO: ASM production code placeholder

        inline bool isEmpty() const
        {
            return version == 0xffff;
        }
    };
#pragma pack(pop)

}  // namespace mab
