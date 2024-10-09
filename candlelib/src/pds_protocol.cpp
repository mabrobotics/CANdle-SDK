#include "pds_protocol.hpp"

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
        return error_E::UNKNOWN_ERROR;
    }

}  // namespace mab