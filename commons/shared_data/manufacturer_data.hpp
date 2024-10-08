#pragma once

#include "mab_types.hpp"

namespace mab
{

    enum class deviceType : u8{
        UNKNOWN = 0x00,
        MD80    = 0x01, 
        MD20    = 0x02,
        MD80HV  = 0x03,
        PDS     = 0x04,

        UNKNOWN    = 0xA0,
        CANDLE     = 0xA1,
        CANDLE_HAT = 0xA2
    };

#pragma pack(push, 1)

    struct hardwareType_S
    {
        deviceType deviceType;  // 0xAX - hosts, 0x0X - peripherials
        u8 deviceRevision;
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