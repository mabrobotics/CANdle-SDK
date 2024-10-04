#pragma once

#include "mab_types.hpp"

namespace mab
{

#pragma pack(push, 1)

    struct hardwareType_S
    {
        u8 deviceType;  // 0xAX - hosts, 0x0X - peripherials
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