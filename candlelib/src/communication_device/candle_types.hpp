#pragma once

#include "mab_types.hpp"
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
    constexpr u32 DEFAULT_CAN_TIMEOUT = 1;  // ms
}  // namespace mab