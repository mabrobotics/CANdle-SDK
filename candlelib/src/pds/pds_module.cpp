#include "pds_module.hpp"
#include "pds_protocol.hpp"

namespace mab
{

    constexpr Logger::ProgramLayer_E DEFAULT_PDS_MODULE_LOG_LAYER = Logger::ProgramLayer_E::LAYER_2;

    PdsModule::PdsModule(socketIndex_E socket, moduleType_E type, Candle& candle, u16& canId)
        : m_socketIndex(socket), m_type(type), m_candle(candle), m_canId(canId)
    {
        m_log.m_layer = DEFAULT_PDS_MODULE_LOG_LAYER;
    }

    socketIndex_E PdsModule::getSocketIndex()
    {
        return m_socketIndex;
    }

    PdsModule::error_E PdsModule::getBoardVersion(moduleVersion_E& version)
    {
        return readModuleProperty(propertyId_E::HW_VERSION, version);
    }

    std::string PdsModule::moduleType2String(moduleType_E type)
    {
        switch (type)
        {
            case moduleType_E::UNDEFINED:
                return "UNDEFINED";
            case moduleType_E::CONTROL_BOARD:
                return "CONTROL_BOARD";
            case moduleType_E::BRAKE_RESISTOR:
                return "BRAKE_RESISTOR";
            case moduleType_E::ISOLATED_CONVERTER:
                return "ISOLATED_CONVERTER";
            case moduleType_E::POWER_STAGE:
                return "POWER_STAGE";

                /* NEW MODULE TYPES HERE */

            default:
                return "UNKNOWN";
        }
    }

    std::string PdsModule::error2String(error_E error)
    {
        switch (error)
        {
            case error_E::OK:
                return "OK";
            case error_E::INTERNAL_ERROR:
                return "INTERNAL_ERROR";
            case error_E::PROTOCOL_ERROR:
                return "PROTOCOL_ERROR";
            case error_E::COMMUNICATION_ERROR:
                return "COMMUNICATION_ERROR";
            default:
                return "UNKNOWN_ERROR";
        }
    }
}  // namespace mab