#pragma once

#include <stdint.h>
#include "logger.hpp"

#include "pds_types.hpp"
#include "pds_protocol.hpp"
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
            OK                  = 0,
            UNKNOWN_ERROR       = 1,
            PROTOCOL_ERROR      = 2,
            COMMUNICATION_ERROR = 3,
        };

        PdsModule() = delete;

        socketIndex_E getSocketIndex();

        /**
         * @brief This enum represents common status bits position in status word for all modules
         * @note Assuming that only first 8 ( LSBits ) are reserved for common status. Other bits
         * are for modules specific status information
         * TODO: Move to the CANdleSDK shared resources!
         */
        enum class status_E : uint32_t
        {
            ENABLED                = (1 << 0x00),
            OVER_TEMPERATURE_EVENT = (1 << 0x01),
            OVER_CURRENT_EVENT     = (1 << 0x02),
        };

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

        template <typename propertyT, typename dataValueT>
        [[nodiscard]] PdsModule::error_E readModuleProperty(propertyT   property,
                                                            dataValueT& dataValue)
        {
            PdsMessage::error_E result = PdsMessage::error_E::OK;
            PropertyGetMessage  message(m_type, m_socketIndex);

            u8     responseBuffer[64] = {0};
            size_t responseLength     = 0;
            u32    rawData            = 0;

            message.addProperty(property);

            std::vector<u8> serializedMessage = message.serialize();
            msp_Candle->sendGenericFDCanFrame(
                m_canId,
                serializedMessage.size(),
                reinterpret_cast<const char*>(serializedMessage.data()),
                reinterpret_cast<char*>(responseBuffer),
                &responseLength);

            result = message.parseResponse(responseBuffer, responseLength);
            if (result != PdsMessage::error_E::OK)
                return error_E::PROTOCOL_ERROR;

            result = message.getProperty(property, &rawData);
            if (result != PdsMessage::error_E::OK)
                return error_E::PROTOCOL_ERROR;

            dataValue = *reinterpret_cast<dataValueT*>(&rawData);

            return error_E::OK;
        }

        template <typename propertyT, typename dataValueT>
        [[nodiscard]] PdsModule::error_E writeModuleProperty(propertyT  property,
                                                             dataValueT dataValue)
        {
            PdsMessage::error_E result = PdsMessage::error_E::OK;
            PropertySetMessage  message(m_type, m_socketIndex);
            u8                  responseBuffer[64] = {0};
            size_t              responseLength     = 0;

            message.addProperty(property, dataValue);
            std::vector<u8> serializedMessage = message.serialize();

            if (!(msp_Candle->sendGenericFDCanFrame(
                    m_canId,
                    serializedMessage.size(),
                    reinterpret_cast<const char*>(serializedMessage.data()),
                    reinterpret_cast<char*>(responseBuffer),
                    &responseLength)))
            {
                return error_E::COMMUNICATION_ERROR;
            }

            result = message.parseResponse(responseBuffer, responseLength);
            if (result != PdsMessage::error_E::OK)
                return error_E::PROTOCOL_ERROR;

            return error_E::OK;
        }
    };

    /**
     * @brief Brake resistor module class
     * @note The instances of the PDS modules are not intended to be created by user manually!!!
     *       The idea is that PDS base class detects connected modules automatically and
     */
    class BrakeResistor : public PdsModule
    {
      public:
        /*
          Properties indexes used internally for creating protocol messages
          for this particular module type. Note that the properties may differ
          from type to type so they all provide own enumerator definition even if they share
          exact same set of properties.
        */
        enum class properties_E : uint8_t
        {

            ENABLED      = 0x00,  // [ BOOL ] Indicates if the module is enabled or not
            TEMPERATURE  = 0x01,  // [ uint32_t ]
            LOAD_CURRENT = 0x05,
            LOAD_POWER   = 0x06,
            TOTAL_ENERGY = 0x07,

        };

        BrakeResistor() = delete;
        BrakeResistor(socketIndex_E socket, std::shared_ptr<Candle> sp_Candle, u16 canId);
        ~BrakeResistor() = default;

        error_E enable();
        error_E disable();

        error_E getEnabled(bool& enabled);
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
        /*
    Properties indexes used internally for creating protocol messages
    for this particular module type. Note that the properties may differ
    from type to type so they all provide own enumerator definition even if they share
    exact same set of properties.
  */
        enum class properties_E : uint8_t
        {
            // TODO: Move
            STATUS          = 0x00,  // [ uint32_t ] Contains status bits
            STATUS_CLEAR    = 0x01,  // [ uint32_t ] Write only property used to clear status bits
            ENABLED         = 0x02,  // [ BOOL ] Indicates if the module is enabled or not
            TEMPERATURE     = 0x03,  // [ uint32_t ]
            BUS_VOLTAGE     = 0x04,  // [ uint32_t ] ( mV )
            BR_SOCKET_INDEX = 0x05,  // [ uint8_t ] Brake Resistor socket index
            BR_TRIGGER_VOLTAGE = 0x06,  // [ uint32_t ] Brake Resistor trigger voltage [ mV ]
            LOAD_CURRENT       = 0x07,
            LOAD_POWER         = 0x08,
            TOTAL_ENERGY       = 0x09,
            OCD_LEVEL          = 0x0A,  // [ mA ]
            OCD_DELAY          = 0x0B,  // [ us ]
            TEMPERATURE_LIMIT  = 0x0C,  // [ *C/10 ]
        };

        struct status_S
        {
            bool ENABLED;
            bool OCD_EVENT;  // Over-current detection event
            bool OVT_EVENT;  // Over-temperature event
        };

        PowerStage() = delete;
        PowerStage(socketIndex_E socket, std::shared_ptr<Candle> sp_Candle, u16 canId);
        ~PowerStage();

        error_E enable();
        error_E disable();

        error_E getStatus(status_S& status);
        error_E clearStatus(status_S status);

        error_E getEnabled(bool& enabled);

        /**
         * @brief Set the socket index which the Brake resistor we want to bind is connected to.
         *
         * @param brakeResistorSocketIndex
         * @return error_E
         */
        error_E bindBrakeResistor(socketIndex_E brakeResistorSocketIndex);

        /**
         * @brief Set the Brake Resistor Trigger Voltage. When the bus voltage will exceed this
         * value, the binded brake resistor will trigger. If there is no Brake resistor binded this
         * method has no effect
         *
         * @param brTriggerVoltage Bus voltage in [ mV ]
         * @return error_E
         */
        error_E setBrakeResistorTriggerVoltage(uint32_t brTriggerVoltage);

        /**
         * @brief Get the Output Voltage of Power Stage module
         *
         * @param outputVoltage
         * @return error_E
         */
        error_E getOutputVoltage(u32& outputVoltage);

        /**
         * @brief Get the Load Current of the Power Stage module
         *
         * @param loadCurrent
         * @return error_E
         */
        error_E getLoadCurrent(s32& loadCurrent);

        /**
         * @brief Get the momentary Power that goes through the Power Stage module
         * @note  Note that this parameter is calculated by the PDS device internally
         * so it may have been calculated from different current and voltage data then that
         * read by host SBC
         * @param power
         *
         * @return error_E
         */
        error_E getPower(s32& power);

        /**
         * @brief Get the total Energy that was delivered by the Power Stage module
         *
         * @param energy
         * @return error_E
         */
        error_E getEnergy(s32& energy);

        /**
         * @brief Get the Temperature of the module
         *
         * @param temperature
         * @return error_E
         */
        error_E getTemperature(f32& temperature);

        /**
         * @brief Set the Over-Current Detection Level of the Power stage module ( in mA )
         *
         * @param ocdLevel
         * @return error_E
         */
        error_E setOcdLevel(u32 ocdLevel);

        /**
         * @brief Get the Over-Current Detection Level of the Power stage module ( in mA )
         *
         * @param ocdLevel
         * @return error_E
         */
        error_E getOcdLevel(u32& ocdLevel);

        /**
         * @brief Set the Over-Current Detection Delay of the Power stage module ( in uS ).
         * If the measured current exceeds the limit ( set with setOcdLevel method )
         *
         * @param ocdDelay
         * @return error_E
         */
        error_E setOcdDelay(u32 ocdDelay);

        /**
         * @brief Get the Over-Current Detection Delay of the Power stage module ( in uS ).
         * If the measured current exceeds the limit ( set with setOcdLevel method )
         *
         * @param ocdDelay
         * @return error_E
         */
        error_E getOcdDelay(u32& ocdDelay);

        error_E setTemperatureLimit(f32 temperatureLimit);
        error_E getTemperatureLimit(f32& temperatureLimit);
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
        IsolatedConv5(socketIndex_E socket, std::shared_ptr<Candle> sp_Candle, u16 canId);
        ~IsolatedConv5() = default;
    };

}  // namespace mab