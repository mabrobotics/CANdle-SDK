#include "pds_module.hpp"
#include "pds_protocol.hpp"

namespace mab
{

    constexpr Logger::ProgramLayer_E DEFAULT_PDS_MODULE_LOG_LAYER = Logger::ProgramLayer_E::LAYER_2;

    PdsModule::PdsModule(socketIndex_E socket, moduleType_E type, Candle& candle, u16& canId)
        : m_socketIndex(socket), m_type(type), m_candle(candle), m_canId(canId)
    {
        m_log.m_layer = DEFAULT_PDS_MODULE_LOG_LAYER;
        m_log.m_tag   = "PDS MODULE";
        // m_log.m_optionalLevel = Logger::LogLevel_E::DEBUG;
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