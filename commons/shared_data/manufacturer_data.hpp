#pragma once

#include "mab_types.hpp"
#ifdef __cplusplus
#include <cstring>
#else
#include <string.h>
#endif

namespace mab
{
    // When adding deviceType_E dont forget to update parsign functions below!!!
    enum class deviceType_E : u8
    {
        UNKNOWN_DEVICE = 0x00,
        MD80           = 0x01,
        MD20           = 0x02,
        MD80HV         = 0x03,
        PDS            = 0x04,
        MD80DE         = 0x05,
        MD80EC         = 0x06,

        MD0004 = 0xB0,

        UNKNOWN_DEVICE_2 = 0xA0,
        CANDLE           = 0xA1,
        CANDLE_HAT       = 0xA2
    };
    inline mab::deviceType_E cStringToDeviceType(char* cstr)
    {
        if (strcmp(cstr, "md80") == 0)
            return mab::deviceType_E::MD80;
        if (strcmp(cstr, "md20") == 0)
            return mab::deviceType_E::MD20;
        if (strcmp(cstr, "md80hv") == 0)
            return mab::deviceType_E::MD80HV;
        if (strcmp(cstr, "md80de") == 0)
            return mab::deviceType_E::MD80DE;
        if (strcmp(cstr, "md80ec") == 0)
            return mab::deviceType_E::MD80EC;
        if (strcmp(cstr, "md0004") == 0)
            return mab::deviceType_E::MD0004;
        if (strcmp(cstr, "candle") == 0)
            return mab::deviceType_E::CANDLE;
        if (strcmp(cstr, "candlehat") == 0)
            return mab::deviceType_E::CANDLE_HAT;
        if (strcmp(cstr, "pds") == 0)
            return mab::deviceType_E::PDS;
        return mab::deviceType_E::UNKNOWN_DEVICE;
    }
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
            case mab::deviceType_E::MD80EC:
                return (char*)"md80ec";
            case mab::deviceType_E::MD0004:
                return (char*)"md0004";
            case mab::deviceType_E::CANDLE:
                return (char*)"candle";
            case mab::deviceType_E::CANDLE_HAT:
                return (char*)"candlehat";
            case mab::deviceType_E::PDS:
                return (char*)"pds";
            default:
                return (char*)"UNKNOWN";
        }
    }

#pragma pack(push, 1)
    // When editing this struct, remember to also update md programming procedure in station repo
    struct manufacturerData_S
    {
        u32          CRC32;
        u16          version;         // manufacturerData format version
        deviceType_E deviceType;      // device type
        u8           deviceRevision;  // 10 -> HW1.0, 32 -> HW3.2
        u8           prodDate[6];     // ddmmyy format
        u8           batchCode[24];   // production code

        inline bool isEmpty() const
        {
            return version == 0xffff;
        }
    };
#pragma pack(pop)

}  // namespace mab
