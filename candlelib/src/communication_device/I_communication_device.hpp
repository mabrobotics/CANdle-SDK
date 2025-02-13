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
            UNINITIALIZED,
            DATA_TOO_LONG,
            DATA_EMPTY,
            RESPONSE_TIMEOUT,
            UNKNOWN_ERROR
        };
        /// @brief Method for exchanging data on the CAN network
        /// @param dataToSend vector with the data to send, must not exceed maximum CAN frame size
        /// @param responseSize compatibility parameter TODO: get rid of that when bus, MD and PDS
        /// reworked
        /// @return pair of received data and errors from the transmission, when error != OK data is
        /// undefined
        virtual const std::pair<std::vector<u8>, Error_t> transferCANFrame(
            const std::vector<u8> dataToSend, const size_t responseSize = 0) = 0;

        /// @brief Method to initialize communication device object's pipes and states
        /// @return status of the initialization
        virtual Error_t init() = 0;
    };
}  // namespace mab