#pragma once

#include "mab_types.hpp"

#ifdef WIN32
#include <windows.h>
#include <unistd.h>
#endif

namespace mab
{
    namespace candleTypes
    {
        enum Error_t
        {
            OK,
            DEVICE_NOT_CONNECTED,
            INITIALIZATION_ERROR,
            UNINITIALIZED,
            DATA_TOO_LONG,
            DATA_EMPTY,
            RESPONSE_TIMEOUT,
            CAN_DEVICE_NOT_RESPONDING,
            INVALID_ID,
            BAD_RESPONSE,
            UNKNOWN_ERROR
        };

        enum busTypes_t
        {
            USB,
            SPI
        };
    };                                      // namespace candleTypes
    constexpr u32 DEFAULT_CAN_TIMEOUT = 2;  // ms
}  // namespace mab
