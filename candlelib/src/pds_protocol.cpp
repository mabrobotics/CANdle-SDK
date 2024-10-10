#include "pds_protocol.hpp"

#include <string.h>

namespace mab
{

    PdsMessage::PdsMessage(moduleType_E moduleType, socketIndex_E socket)
        : m_moduleType(moduleType), m_socketIndex(socket)
    {
    }

    PropertySetMessage::PropertySetMessage(moduleType_E moduleType, socketIndex_E socket)
        : PdsMessage(moduleType, socket)
    {
    }

    std::vector<u8> PropertySetMessage::serialize()
    {
        std::vector<u8> serializedMessage;

        if (m_properties.empty())
            throw std::runtime_error("The message to be serialized has no properties added");

        serializedMessage.push_back(
            static_cast<u8>(PdsMessage::commandCode_E::SET_MODULE_PROPERTY));

        serializedMessage.push_back(static_cast<u8>(m_moduleType));
        serializedMessage.push_back(static_cast<u8>(m_socketIndex));
        serializedMessage.push_back(static_cast<u8>(m_properties.size()));

        for (auto property : m_properties)
        {
            // TODO: Tweak property value size in bytes to its actual size instead of fixed 4 bytes
            serializedMessage.push_back(property.first);
            for (u8 byteI = 0; byteI < sizeof(property.second); byteI++)
            {
                serializedMessage.push_back(*(((u8*)&property.second) + byteI));
            }
        }

        if (serializedMessage.size() > MAX_SERIALIZED_SIZE)
            throw std::runtime_error("Serialized message exceeds FDCAN max buffer size");

        return serializedMessage;
    }

    PdsMessage::error_E PropertySetMessage::parseResponse(u8* p_response, size_t responseLength)
    {
        if (p_response == nullptr)
            return error_E::UNKNOWN_ERROR;

        if ((responseLength == 0) || responseLength > 1)
            return error_E::INVALID_RESPONSE_LENGTH;

        responseCode_E responseCode = static_cast<responseCode_E>(*p_response);

        if (responseCode != responseCode_E::OK)
            return error_E::RESPONSE_STATUS_ERROR;

        return error_E::OK;
    }

    PropertyGetMessage::PropertyGetMessage(moduleType_E moduleType, socketIndex_E socket)
        : PdsMessage(moduleType, socket)
    {
    }

    std::vector<u8> PropertyGetMessage::serialize()
    {
        std::vector<u8> serializedMessage;

        if (m_properties.empty())
            throw std::runtime_error("The message to be serialized has no properties added");

        serializedMessage.push_back(
            static_cast<u8>(PdsMessage::commandCode_E::GET_MODULE_PROPERTY));

        serializedMessage.push_back(static_cast<u8>(m_moduleType));
        serializedMessage.push_back(static_cast<u8>(m_socketIndex));
        serializedMessage.push_back(static_cast<u8>(m_properties.size()));

        for (auto property : m_properties)
        {
            serializedMessage.push_back(property);
        }

        if (serializedMessage.size() > MAX_SERIALIZED_SIZE)
            throw std::runtime_error("Serialized message exceeds FDCAN max buffer size");

        return serializedMessage;
    }

    PdsMessage::error_E PropertyGetMessage::parseResponse(u8* p_response, size_t responseLength)
    {
        responseCode_E responseStatusCode         = responseCode_E::OK;
        size_t         numberOfReceivedProperties = 0;

        if (p_response == nullptr)
            return error_E::UNKNOWN_ERROR;

        responseStatusCode = static_cast<responseCode_E>(*p_response++);
        if (responseStatusCode != responseCode_E::OK)
            return error_E::RESPONSE_STATUS_ERROR;
        /*
            Calculate correct response length from previously created properties set
            Expecting:
            * two heading bytes ( Response status and number of properties in response )
            * 5 bytes for each property that was added to command
        */
        size_t expectedResponseLength = 2 + (m_properties.size() * 5);

        if (responseLength != expectedResponseLength)
            return error_E::INVALID_RESPONSE_LENGTH;

        numberOfReceivedProperties = static_cast<size_t>(*p_response++);
        if (numberOfReceivedProperties != m_properties.size())
            return error_E::INVALID_PROPERTIES_NUMBER;

        for (uint8_t i = 0; i < numberOfReceivedProperties; i++)
        {
            u8  type     = *p_response++;
            u32 rawValue = 0;
            memcpy(&rawValue, p_response, sizeof(u32));
            m_receivedProperties.push_back(std::make_pair(type, rawValue));
            p_response += 4;
        }

        return error_E::OK;
    }

}  // namespace mab