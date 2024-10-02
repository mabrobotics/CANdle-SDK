#include "pds_protocol.hpp"

namespace mab
{

    PdsMessage::PdsMessage(moduleType_E moduleType, socketIndex_E socket)
        : m_moduleType(moduleType), m_socketIndex(socket)
    {
    }

    PropertyWriteMessage::PropertyWriteMessage(moduleType_E moduleType, socketIndex_E socket)
        : PdsMessage(moduleType, socket)
    {
    }

    std::vector<u8> PropertyWriteMessage::serialize()
    {
        std::vector<u8> serializedMessage;

        serializedMessage.push_back(static_cast<u8>(m_moduleType));
        serializedMessage.push_back(static_cast<u8>(m_socketIndex));

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

}  // namespace mab