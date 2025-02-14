#pragma once

#include <optional>
#include <functional>
#include <utility>
#include <vector>

#include <mab_types.hpp>

namespace mab
{
    class I_CommunicationInterface
    {
      public:
        enum Error_t
        {
            OK,
            NOT_CONNECTED,
            INITIALIZATION_ERROR,
            COULD_NOT_ACQUIRE_INTEFACE,
            RESPONSE_TIMEOUT,
            UNKNOWN_ERROR,
            DATA_TOO_LONG,
            DATA_EMPTY
        };

        virtual Error_t connect()    = 0;
        virtual Error_t disconnect() = 0;

        virtual Error_t transfer(std::vector<u8> data, const u32 timeoutMs) = 0;
        virtual std::pair<std::vector<u8>, Error_t> transfer(
            std::vector<u8> data, const u32 timeoutMs, const size_t expectedReceivedDataSize) = 0;
    };
}  // namespace mab