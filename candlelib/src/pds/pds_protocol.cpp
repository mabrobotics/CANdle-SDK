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
            serializedMessage.push_back(static_cast<u8>(property.first));
            size_t propertySize = getPropertySize(property.first);
            for (size_t i = 0; i < propertySize; i++)
            {
                serializedMessage.push_back(static_cast<u8>(property.second >> (i * 8)));
            }
            // for (u8 byteI = 0; byteI < sizeof(property.second); byteI++)
            // {
            //     serializedMessage.push_back(*(((u8*)&property.second) + byteI));
            // }
        }

        if (serializedMessage.size() > MAX_SERIALIZED_SIZE)
            throw std::runtime_error("Serialized message exceeds FDCAN max buffer size");

        return serializedMessage;
    }

    PdsMessage::error_E PropertySetMessage::parseResponse(u8* p_response, size_t responseLength)
    {
        size_t expectedResponseLength  = 2 + m_properties.size();  // 2 response header bytes
        u8     nProperties             = 0;
        bool   propertiesModifySuccess = true;
        propertyError_E propertyResult = propertyError_E::OK;

        if (p_response == nullptr)
            return error_E::UNKNOWN_ERROR;

        if (responseLength <
            expectedResponseLength)  // Might be greater (FD CAN) or equal but not less
            return error_E::INVALID_RESPONSE_LENGTH;

        msgResponse_E responseCode = static_cast<msgResponse_E>(*p_response++);

        if (responseCode != msgResponse_E::OK)
            return error_E::RESPONSE_STATUS_ERROR;

        nProperties = *p_response++;

        if (nProperties != m_properties.size())
            return error_E::INVALID_PROPERTIES_NUMBER;

        for (u8 i = 0; i < nProperties; i++)
        {
            propertyResult = static_cast<propertyError_E>(*p_response++);
            if (propertyResult != propertyError_E::OK)
            {
                propertiesModifySuccess = false;
                m_log.warn("Setting property [ %u ] failed with error code [ %u ( %s ) ]",
                           m_properties[i].first,
                           propertyResult,
                           propertyError2String(propertyResult));
            }
        }

        if (!propertiesModifySuccess)
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
            serializedMessage.push_back(static_cast<u8>(property));
        }

        if (serializedMessage.size() > MAX_SERIALIZED_SIZE)
            throw std::runtime_error("Serialized message exceeds FDCAN max buffer size");

        return serializedMessage;
    }

    PdsMessage::error_E PropertyGetMessage::parseResponse(u8* p_response, size_t responseLength)
    {
        u8*           initialPointer             = p_response;
        msgResponse_E responseStatusCode         = msgResponse_E::OK;
        size_t        numberOfReceivedProperties = 0;

        if (p_response == nullptr)
            return error_E::UNKNOWN_ERROR;

        responseStatusCode = static_cast<msgResponse_E>(*p_response++);
        if (responseStatusCode != msgResponse_E::OK)
            return error_E::RESPONSE_STATUS_ERROR;

        // Remove all previously parsed properties. It allows to reuse previously defined message to
        // be used multiple times
        m_receivedProperties.clear();

        numberOfReceivedProperties = static_cast<size_t>(*p_response++);
        if (numberOfReceivedProperties != m_properties.size())
            return error_E::INVALID_PROPERTIES_NUMBER;

        for (uint8_t i = 0; i < numberOfReceivedProperties; i++)
        {
            propertyId_E    type           = m_properties[i];
            size_t          propertySize   = getPropertySize(type);
            u32             rawValue       = 0;
            propertyError_E propertyStatus = static_cast<propertyError_E>(*p_response++);
            size_t          bytesLeft      = responseLength - (p_response - initialPointer);
            if (bytesLeft < propertySize)
            {
                m_log.error("Not enough bytes left in response buffer to read property value");
                return error_E::INVALID_RESPONSE_LENGTH;
            }
            memcpy(&rawValue, p_response, propertySize);
            if (propertyStatus == propertyError_E::OK)
                m_receivedProperties.push_back(std::make_pair(type, rawValue));
            p_response += propertySize;
        }

        return error_E::OK;
    }

    // propertyError2String
    const char* PdsMessage::propertyError2String(propertyError_E error)
    {
        switch (error)
        {
            case propertyError_E::OK:
                return "OK";
            case propertyError_E::PROPERTY_NOT_AVAILABLE:
                return "PROPERTY_NOT_AVAILABLE";
            case propertyError_E::INVALID_ACCESS:
                return "INVALID_ACCESS";
            case propertyError_E::INVALID_DATA:
                return "INVALID_DATA";
            default:
                return "UNKNOWN_ERROR";
        }
    }

}  // namespace mab