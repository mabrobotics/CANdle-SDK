#pragma once

#include <stdint.h>
#include "logger.hpp"

#include "pds_types.hpp"
#include "pds_protocol.hpp"
#include "candle.hpp"
#include <cstring>
#include <memory>

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
            INTERNAL_ERROR      = 1,
            PROTOCOL_ERROR      = 2,
            COMMUNICATION_ERROR = 3,

        };

        PdsModule() = delete;

        socketIndex_E getSocketIndex();

        error_E getBoardVersion(moduleVersion_E& version);

        virtual void printModuleInfo(void) = 0;

        constexpr static const char* mType2Str(moduleType_E type)
        {
            switch (type)
            {
                case moduleType_E::UNDEFINED:
                    return "UNDEFINED";
                case moduleType_E::CONTROL_BOARD:
                    return "CONTROL_BOARD";
                case moduleType_E::BRAKE_RESISTOR:
                    return "BRAKE_RESISTOR";
                case moduleType_E::ISOLATED_CONVERTER:
                    return "ISOLATED_CONVERTER";
                case moduleType_E::POWER_STAGE:
                    return "POWER_STAGE";

                    /* NEW MODULE TYPES HERE */

                default:
                    return "UNKNOWN";
            }
        }

        constexpr static const char* error2String(error_E error)
        {
            switch (error)
            {
                case error_E::OK:
                    return "OK";
                case error_E::INTERNAL_ERROR:
                    return "INTERNAL_ERROR";
                case error_E::PROTOCOL_ERROR:
                    return "PROTOCOL_ERROR";
                case error_E::COMMUNICATION_ERROR:
                    return "COMMUNICATION_ERROR";
                default:
                    return "UNKNOWN_ERROR";
            }
        }

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
        PdsModule(socketIndex_E socket, moduleType_E type, Candle& candle, u16& canId);

        Logger m_log;

        // Represents physical socket index number that the particular module is connected to.
        const socketIndex_E m_socketIndex;

        /* Type of the module */
        const moduleType_E m_type;

        /* Pointer to the candle object. Assumed to be passed in a constructor from parent PDS
         * object. It will be used for independent communication from each module perspective */
        Candle& m_candle;

        /* CAN ID of the parent PDS device. Assumed to be passed in a constructor from parent PDS
         * object */
        u16& m_canId;

        // TODO: Now propertyID is single for all modules so it dont has to be templated
        template <typename dataValueT>
        [[nodiscard]] PdsModule::error_E readModuleProperty(propertyId_E property,
                                                            dataValueT&  dataValue)
        {
            PdsMessage::error_E result = PdsMessage::error_E::OK;
            PropertyGetMessage  message(m_type, m_socketIndex);

            u8     responseBuffer[64] = {0};
            size_t responseLength     = 0;
            u32    rawData            = 0;

            message.addProperty(property);

            std::vector<u8> serializedMessage = message.serialize();
            if (!m_candle.sendGenericFDCanFrame(
                    m_canId,
                    serializedMessage.size(),
                    reinterpret_cast<const char*>(serializedMessage.data()),
                    reinterpret_cast<char*>(responseBuffer),
                    &responseLength))
                return error_E::COMMUNICATION_ERROR;

            result = message.parseResponse(responseBuffer, responseLength);
            if (result != PdsMessage::error_E::OK)
                return error_E::PROTOCOL_ERROR;

            result = message.getProperty(property, &rawData);
            if (result != PdsMessage::error_E::OK)
                return error_E::PROTOCOL_ERROR;

            std::memcpy(&dataValue, &rawData, sizeof(dataValue));

            return error_E::OK;
        }

        template <typename dataValueT>
        [[nodiscard]] PdsModule::error_E writeModuleProperty(propertyId_E property,
                                                             dataValueT   dataValue)
        {
            PdsMessage::error_E result = PdsMessage::error_E::OK;
            PropertySetMessage  message(m_type, m_socketIndex);
            u8                  responseBuffer[64] = {0};
            size_t              responseLength     = 0;

            message.addProperty(property, dataValue);
            std::vector<u8> serializedMessage = message.serialize();

            if (!(m_candle.sendGenericFDCanFrame(
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

}  // namespace mab