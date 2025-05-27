#include "pds_module.hpp"
#include "pds_protocol.hpp"

namespace mab
{

    PdsModule::PdsModule(socketIndex_E socket, moduleType_E type, Candle* p_candle, u16& canId)
        : m_socketIndex(socket), m_type(type), mp_candle(p_candle), m_canId(canId)
    {
    }

    socketIndex_E PdsModule::getSocketIndex() const
    {
        return m_socketIndex;
    }

    PdsModule::error_E PdsModule::getBoardVersion(moduleVersion_E& version) const
    {
        return readModuleProperty(propertyId_E::HW_VERSION, version);
    }

}  // namespace mab