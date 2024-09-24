#pragma once

#include <stdint.h>
#include "logger.hpp"

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

        enum class type_E : uint8_t
        {
            // Undefined means that module is not connected or PDS could not determine its type.
            UNDEFINED = 0x00,
            BRAKE_RESISTOR,
            ISOLATED_CONVERTER_12V,
            ISOLATED_CONVERTER_5V,
            POWER_STAGE_V1,
            POWER_STAGE_V2,
            /* NEW MODULE TYPES HERE */
        };

        enum class socket_E : uint8_t
        {
            SOCKET_1 = 0x00,
            SOCKET_2 = 0x01,
            SOCKET_3 = 0x02,
            SOCKET_4 = 0x03,
            SOCKET_5 = 0x04,
            SOCKET_6 = 0x05,
        };

        PdsModule() = delete;

        socket_E getSocket();

        // static std::string moduleType2String(moduleType_E type);

      protected:
        /*
           Constructor is protected because even if this class has no pure virtual methods, it
           still should not be instantiated.
        */
        PdsModule(socket_E socket);
        logger m_log;

        type_E m_type;

        // Represents physical socket index number that the particular module is connected to.
        socket_E m_socketIndex;
    };

    class BrakeResistor : public PdsModule
    {
      public:
        BrakeResistor() = delete;
        BrakeResistor(socket_E socket);
        ~BrakeResistor() = default;

        // TODO: concept function. Might be removed in the future ( To be discussed )
        error_E setOperatingVoltageThreshold(uint16_t voltage);
    };

    class PowerStageV1 : public PdsModule
    {
      public:
        PowerStageV1() = delete;
        PowerStageV1(socket_E socket);
        ~PowerStageV1() = default;

        error_E enable();
        error_E disable();
        error_E isEnabled(bool& enabled);
    };

    class PowerStageV2 : public PdsModule
    {
      public:
        PowerStageV2() = delete;
        PowerStageV2(socket_E socket);
        ~PowerStageV2() = default;
    };

    class IsolatedConv12 : public PdsModule
    {
      public:
        IsolatedConv12() = delete;
        IsolatedConv12(socket_E socket);
        ~IsolatedConv12() = default;
    };

    class IsolatedConv5 : public PdsModule
    {
      public:
        IsolatedConv5() = delete;
        IsolatedConv5(socket_E socket);
        ~IsolatedConv5() = default;
    };

}  // namespace mab