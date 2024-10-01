#pragma once

#include <stdint.h>
#include "logger.hpp"

#include "pds_types.hpp"

namespace mab
{

    /**
     * @brief Power distribution system pluggable module abstract class
     *
     */
    class PdsModule
    {
      public:
        enum class error_E : int8_t
        {
            UNKNOWN_ERROR = -1,
            OK            = 0,
        };

        PdsModule() = delete;

        socketIndex_E getSocket();

        // static std::string moduleType2String(moduleType_E type);

      protected:
        /*
           Constructor is protected because even if this class has no pure virtual methods, it
           still should not be instantiated.
        */
        PdsModule(socketIndex_E socket, moduleType_E type);
        Logger m_log;

        const socketIndex_E m_socketIndex;
        const moduleType_E  m_type;

        // Represents physical socket index number that the particular module is connected to.
    };

    class BrakeResistor : public PdsModule
    {
      public:
        BrakeResistor() = delete;
        BrakeResistor(socketIndex_E socket);
        ~BrakeResistor() = default;

        // TODO: concept function. Might be removed in the future ( To be discussed )
        error_E setOperatingVoltageThreshold(uint16_t voltage);
    };

    class PowerStage : public PdsModule
    {
      public:
        PowerStage() = delete;
        PowerStage(socketIndex_E socket);
        ~PowerStage() = default;

        error_E enable();
        error_E disable();
        error_E isEnabled(bool& enabled);

      private:
        /*
          Control parameters indexes used internally for creating protocol messages
          for this particular module type. Note that the control parameters may differ
          from type to type so they all provide own enumerator definition even if they share
          exact same set of control parameters.
        */
        enum class controlParameters_E : uint8_t
        {

            ENABLED      = 0x00,  // Indicates if the module is enabled or not
            BUS_VOLTAGE  = 0x01,
            LOAD_CURRENT = 0x02,
            TEMPERATURE  = 0x03,

        };
    };

    class IsolatedConv12 : public PdsModule
    {
      public:
        IsolatedConv12() = delete;
        IsolatedConv12(socketIndex_E socket);
        ~IsolatedConv12() = default;
    };

    class IsolatedConv5 : public PdsModule
    {
      public:
        IsolatedConv5() = delete;
        IsolatedConv5(socketIndex_E socket);
        ~IsolatedConv5() = default;
    };

}  // namespace mab