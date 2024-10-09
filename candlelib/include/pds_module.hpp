#pragma once

#include <stdint.h>
#include "logger.hpp"

#include "pds_types.hpp"
#include "candle.hpp"

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
            PROTOCOL_ERROR = -2,
            UNKNOWN_ERROR  = -1,
            OK             = 0,
        };

        PdsModule() = delete;

        socketIndex_E getSocket();

        // static std::string moduleType2String(moduleType_E type);

      protected:
        /**
         * @brief Construct a new Pds Module object
         *
         * @param socket Physical socket index
         * @param type module type
         * @param sp_Candle shared pointer to Candle device the parent PDS device is connected with
         * @param canId CAN id of the parent PDS device
         *
         * @note Constructor is protected because even if this class has no pure virtual methods, it
            still should not be instantiated.
         */
        PdsModule(socketIndex_E           socket,
                  moduleType_E            type,
                  std::shared_ptr<Candle> sp_Candle,
                  u16                     canId);

        Logger m_log;

        // Represents physical socket index number that the particular module is connected to.
        const socketIndex_E m_socketIndex;

        /* Type of the module */
        const moduleType_E m_type;

        /* Pointer to the candle object. Assumed to be passed in a constructor from parent PDS
         * object. It will be used for independent communication from each module perspective */
        const std::shared_ptr<Candle> msp_Candle = nullptr;

        /* CAN ID of the parent PDS device. Assumed to be passed in a constructor from parent PDS
         * object */
        const u16 m_canId;
    };

    /**
     * @brief Brake resistor module class
     * @note The instances of the PDS modules are not intended to be created by user manually!!!
     *       The idea is that PDS base class detects connected modules automatically and
     */
    class BrakeResistor : public PdsModule
    {
      public:
        BrakeResistor() = delete;
        BrakeResistor(socketIndex_E socket, std::shared_ptr<Candle> sp_Candle, u16 canId);
        ~BrakeResistor() = default;

        // TODO: concept function. Might be removed in the future ( To be discussed )
        error_E setOperatingVoltageThreshold(uint16_t voltage);
    };

    /**
     * @brief Power stage module class
     * @note The instances of the PDS modules are not intended to be created by user manually!!!
     *       The idea is that PDS base class detects connected modules automatically and
     *
     */
    class PowerStage : public PdsModule
    {
      public:
        PowerStage() = delete;
        PowerStage(socketIndex_E socket, std::shared_ptr<Candle> sp_Candle, u16 canId);
        ~PowerStage() = default;

        error_E enable();
        error_E disable();

        error_E getEnabled(bool& enabled);

        error_E bindBrakeResistor(socketIndex_E brakeResistorSocketIndex);

        // error_E setBrakeResistorTriggerVoltage(uint32_t brTriggerVoltage);

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
        // private:
    };

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
        IsolatedConv12(socketIndex_E socket, std::shared_ptr<Candle> sp_Candle, u16 canId);
        ~IsolatedConv12() = default;

        error_E enable();
        error_E disable();

        error_E isEnabled(bool& enabled);

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
        IsolatedConv5(socketIndex_E socket, std::shared_ptr<Candle> sp_Candle, u16 canId);
        ~IsolatedConv5() = default;
    };

}  // namespace mab