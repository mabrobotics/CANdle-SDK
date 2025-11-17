#pragma once

#include "mab_types.hpp"
#include <vector>

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

        struct CANFrameData_t
        {
            const canId_t                                m_canId          = 0;
            std::vector<u8>                              m_data           = {};
            u8                                           m_responseLength = 0;
            std::chrono::high_resolution_clock::duration m_timeout = std::chrono::microseconds(0);

            CANFrameData_t(const canId_t canId) : m_canId(canId)
            {
            }
        };
    };  // namespace candleTypes
    constexpr u32 DEFAULT_CAN_TIMEOUT = 2;  // ms
}  // namespace mab
