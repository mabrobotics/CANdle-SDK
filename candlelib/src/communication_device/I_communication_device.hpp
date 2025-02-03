#pragma once

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

        virtual std::pair<std::vector<u8>, Error_t> transferData(std::vector<u8> dataToSend) = 0;
    };
}  // namespace mab