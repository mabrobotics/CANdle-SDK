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
            TRANSMITTER_ERROR,
            RECEIVER_ERROR,
            UNKNOWN_ERROR,
            DATA_TOO_LONG,
            DATA_EMPTY
        };

        virtual ~I_CommunicationInterface(){};

        /// @brief Method to claim communication interface and enable communication
        /// @return Error on failure, OK on success
        virtual Error_t connect() = 0;

        /// @brief Method to release communication interface and disable communication
        /// @return Error on failure, OK on success
        virtual Error_t disconnect() = 0;

        /// @brief Method to exchange data through interface
        /// @param data Data to send to the device
        /// @param timeoutMs Wait time for the response
        /// @return Error on failure, OK on success
        virtual Error_t transfer(std::vector<u8> data, const u32 timeoutMs) = 0;

        /// @brief Method to exchange data through interface
        /// @param data Data to send to the device
        /// @param timeoutMs Wait time for the response
        /// @param expectedReceivedDataSize Size of the response
        /// @return Data from the device and possible errors. If error_t != OK than data's content
        /// is UB.
        virtual std::pair<std::vector<u8>, Error_t> transfer(
            std::vector<u8> data, const u32 timeoutMs, const size_t expectedReceivedDataSize) = 0;
    };
}  // namespace mab