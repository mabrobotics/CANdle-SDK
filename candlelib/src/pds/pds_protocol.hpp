#pragma once

#include <stdint.h>
#include "pds_types.hpp"
#include "logger.hpp"
#include "mab_types.hpp"
#include <string.h>
#include <vector>

namespace mab
{

    class PdsMessage
    {
      public:
        // Maximum number of bytes in message buffer - limited by the FDCAN Interface
        static constexpr size_t MAX_SERIALIZED_SIZE = 64u;

        PdsMessage()  = delete;
        ~PdsMessage() = default;

        enum class error_E : u8
        {
            OK                        = 0x00,
            UNKNOWN_ERROR             = 0x01,
            RESPONSE_STATUS_ERROR     = 0x02,
            INVALID_RESPONSE_LENGTH   = 0x03,
            INVALID_PROPERTIES_NUMBER = 0x04,
        };

        enum class commandCode_E : u8
        {
            // GET_MODULES         = 0x05,  // deprecated
            GET_MODULE_PROPERTY = 0x20,
            SET_MODULE_PROPERTY = 0x21,
            GET_FW_METADATA     = 0x22,
        };

        static const char* propertyError2String(propertyError_E error);

      protected:
        /* ModuleType / socket AKA who / where */
        PdsMessage(moduleType_E moduleType, socketIndex_E socket);
        Logger              m_log;
        const moduleType_E  m_moduleType;
        const socketIndex_E m_socketIndex;
    };

    class PropertySetMessage : public PdsMessage
    {
      public:
        PropertySetMessage() = delete;
        PropertySetMessage(moduleType_E moduleType, socketIndex_E socket);
        ~PropertySetMessage() = default;

        template <typename valueT>
        void addProperty(propertyId_E propertyType, valueT value)
        {
            // u8  castedPropertyType = static_cast<u8>(propertyType);
            u32 castedValue = 0;
            memcpy(&castedValue, &value, sizeof(valueT));

            m_properties.push_back(std::make_pair(propertyType, castedValue));
        }

        std::vector<u8> serialize();
        error_E         parseResponse(u8* p_response, size_t responseLength);

      private:
        std::vector<std::pair<propertyId_E, u32>> m_properties;
    };

    class PropertyGetMessage : public PdsMessage
    {
      public:
        PropertyGetMessage() = delete;
        PropertyGetMessage(moduleType_E moduleType, socketIndex_E socket);
        ~PropertyGetMessage() = default;

        void addProperty(propertyId_E propertyId)
        {
            // u8 castedPropertyType = static_cast<u8>(propertyId);
            m_properties.push_back(propertyId);
        }

        error_E getProperty(propertyId_E propertyId, u32* p_propertyValue)
        {
            // u8 castedPropertyType = static_cast<u8>(propertyId);
            for (auto& property : m_receivedProperties)
            {
                if (property.first == propertyId)
                {
                    *p_propertyValue = property.second;
                    return error_E::OK;
                }
            }

            return error_E::UNKNOWN_ERROR;
        }

        std::vector<u8> serialize();
        error_E         parseResponse(u8* p_response, size_t responseLength);

      private:
        // Vector that holds a set of properties that we want to read
        std::vector<propertyId_E> m_properties;

        /*
          Vector that holds a set of properties that has been received in response
          in the same order that we construct the message
          */
        std::vector<std::pair<propertyId_E, u32>> m_receivedProperties;
    };

}  // namespace mab