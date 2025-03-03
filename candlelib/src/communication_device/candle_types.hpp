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
            UNKNOWN_ERROR
        };

        enum busTypes_t
        {
            USB,
            SPI
        };
    };                                      // namespace candleTypes
    constexpr u32 DEFAULT_CAN_TIMEOUT = 1;  // ms

    // dataOut, errorsDuringTransmissions (canID, dataIn, expectedDataOut, timeoutMs)
    using canTransmitFrame_t = std::pair<std::vector<u8>, candleTypes::Error_t>(
        const canId_t, const std::vector<u8>, const size_t, const u32);
}  // namespace mab