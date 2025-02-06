#pragma once

#include <vector>
#include <array>
#include <bit>
#include <type_traits>
#include <utility>
#include <vector>

#include <mab_types.hpp>

namespace mab
{

    class I_CommunicationDevice
    {
      public:
        enum Error_t
        {
            OK,
            DEVICE_NOT_CONNECTED,
            INITIALIZATION_ERROR,
            DATA_TOO_LONG,
            DATA_EMPTY,
            RESPONSE_TIMEOUT,
            UNKNOWN_ERROR
        };

        virtual const std::pair<std::vector<u8>, Error_t> transferCANFrame(
            const std::vector<u8> dataToSend, const size_t responseSize = 0) = 0;
    };
}  // namespace mab