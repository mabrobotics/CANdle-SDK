#include "pds_protocol.hpp"

namespace mab
{

    PdsMessage::PdsMessage(moduleType_E moduleType, socketIndex_E socket)
        : m_moduleType(moduleType), m_socketIndex(socket)
    {
    }

    SetPropertyMessage::SetPropertyMessage(moduleType_E moduleType, socketIndex_E socket)
        : PdsMessage(moduleType, socket)
    {
    }

}  // namespace mab