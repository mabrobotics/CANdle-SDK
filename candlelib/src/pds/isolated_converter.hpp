#pragma once

#include "pds_module.hpp"

namespace mab
{
    /**
     * @brief 12V Isolated converter module class
     * @note The instances of the PDS modules are not intended to be created by user manually!!!
     *       The idea is that PDS base class detects connected modules automatically and
     *
     */
    class IsolatedConv12 : public PdsModule
    {
      public:
        IsolatedConv12() = delete;
        IsolatedConv12(socketIndex_E socket, Candle& candle, u16 canId);
        ~IsolatedConv12() = default;

        error_E enable();
        error_E disable();

        error_E isEnabled(bool& enabled);

        /*
          Properties indexes used internally for creating protocol messages
          for this particular module type. Note that the properties may differ
          from type to type so they all provide own enumerator definition even if they share
          exact same set of properties.
        */
        enum class properties_E : uint8_t
        {
            STATUS            = 0x00,  // [ uint32_t ] Contains status bits
            ENABLED           = 0x01,  // [ BOOL ] Indicates if the module is enabled or not
            TEMPERATURE       = 0x02,  // [ uint32_t ]
            BUS_VOLTAGE       = 0x05,  // [ uint32_t ] ( mV )
            LOAD_CURRENT      = 0x06,
            LOAD_POWER        = 0x07,
            TOTAL_ENERGY      = 0x08,
            OCD_LEVEL         = 0x09,  // [ mA ]
            OCD_DELAY         = 0x0A,  // [ us ]
            TEMPERATURE_LIMIT = 0x0B,  // [ *C/10 ]
        };
    };

    /**
     * @brief 5V Isolated converter module class
     * @note The instances of the PDS modules are not intended to be created by user manually!!!
     *       The idea is that PDS base class detects connected modules automatically and
     *
     */
    class IsolatedConv5 : public PdsModule
    {
      public:
        IsolatedConv5() = delete;
        IsolatedConv5(socketIndex_E socket, Candle& candle, u16 canId);
        ~IsolatedConv5() = default;
    };

}  // namespace mab