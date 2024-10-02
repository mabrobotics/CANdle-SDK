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

        enum class error_E
        {
            OK            = 0x00,
            UNKNOWN_ERROR = 0x01
        };

      protected:
        /* ModuleType / socket AKA who / where */
        PdsMessage(moduleType_E moduleType, socketIndex_E socket);
        Logger              m_log;
        const moduleType_E  m_moduleType;
        const socketIndex_E m_socketIndex;
    };

    class PropertyWriteMessage : public PdsMessage
    {
      public:
        PropertyWriteMessage() = delete;
        PropertyWriteMessage(moduleType_E moduleType, socketIndex_E socket);
        ~PropertyWriteMessage() = default;

        template <typename propertyT, typename valueT>
        void addProperty(propertyT propertyType, valueT value)
        {
            u8  castedPropertyType = static_cast<u8>(propertyType);
            u32 castedValue        = static_cast<uint32_t>(value);

            m_properties.push_back(std::make_pair(castedPropertyType, castedValue));
        }

        std::vector<u8> serialize();

      private:
        std::vector<std::pair<u8, u32>> m_properties;
    };

}  // namespace mab