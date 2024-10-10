#pragma once

#include <stdint.h>
#include "pds_types.hpp"
#include "logger.hpp"
#include "mab_types.hpp"

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
            GET_MODULES         = 0x05,
            GET_MODULE_PROPERTY = 0x20,
            SET_MODULE_PROPERTY = 0x21
        };

        enum class responseCode_E : u8
        {
            OK    = 0x00,
            ERROR = 0x01,
        };

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

        // TODO: change the value to not templated uint32_t and treat it as raw data buffer to be
        // able to handle float and double values
        template <typename propertyT, typename valueT>
        void addProperty(propertyT propertyType, valueT value)
        {
            u8  castedPropertyType = static_cast<u8>(propertyType);
            u32 castedValue        = static_cast<uint32_t>(value);

            m_properties.push_back(std::make_pair(castedPropertyType, castedValue));
        }

        std::vector<u8> serialize();
        error_E         parseResponse(u8* p_response, size_t responseLength);

      private:
        std::vector<std::pair<u8, u32>> m_properties;
    };

    class PropertyGetMessage : public PdsMessage
    {
      public:
        PropertyGetMessage() = delete;
        PropertyGetMessage(moduleType_E moduleType, socketIndex_E socket);
        ~PropertyGetMessage() = default;

        template <typename propertyT>
        void addProperty(propertyT propertyType)
        {
            u8 castedPropertyType = static_cast<u8>(propertyType);
            m_properties.push_back(castedPropertyType);
        }

        template <typename propertyT>
        error_E getProperty(propertyT propertyType, u32* p_propertyValue)
        {
            u8 castedPropertyType = static_cast<u8>(propertyType);
            for (auto& property : m_receivedProperties)
            {
                if (property.first == castedPropertyType)
                    *p_propertyValue = property.second;
                return error_E::OK;
            }

            return error_E::RESPONSE_STATUS_ERROR;
        }

        std::vector<u8> serialize();
        error_E         parseResponse(u8* p_response, size_t responseLength);

      private:
        // Vector that holds a set of properties that we want to read
        std::vector<u8> m_properties;

        /*
          Vector that holds a set of properties that has been received in response
          in the same order that we construct the message
          */
        std::vector<std::pair<u8, u32>> m_receivedProperties;
    };

}  // namespace mab