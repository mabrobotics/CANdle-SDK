#include "pds_module.hpp"
#include "pds_protocol.hpp"

namespace mab
{

    constexpr Logger::ProgramLayer_E DEFAULT_PDS_MODULE_LOG_LAYER = Logger::ProgramLayer_E::LAYER_2;

    PdsModule::PdsModule(socketIndex_E socket, moduleType_E type)
        : m_socketIndex(socket), m_type(type)
    {
        m_log.m_layer = DEFAULT_PDS_MODULE_LOG_LAYER;
    }

    socketIndex_E PdsModule::getSocket()
    {
        return m_socketIndex;
    }

    BrakeResistor::BrakeResistor(socketIndex_E socket)
        : PdsModule(socket, moduleType_E::BRAKE_RESISTOR)
    {
        m_log.m_tag = "BR  :: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

    PowerStage::PowerStage(socketIndex_E socket) : PdsModule(socket, moduleType_E::POWER_STAGE)
    {
        m_log.m_tag = "PS  :: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

    PdsModule::error_E PowerStage::enable()
    {
        SetPropertyMessage enableMessage(m_type, m_socketIndex);

        return error_E::OK;
    }

    IsolatedConv12::IsolatedConv12(socketIndex_E socket)
        : PdsModule(socket, moduleType_E::ISOLATED_CONVERTER_12V)
    {
        m_log.m_tag = "IC12:: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

    IsolatedConv5::IsolatedConv5(socketIndex_E socket)
        : PdsModule(socket, moduleType_E::ISOLATED_CONVERTER_5V)
    {
        m_log.m_tag = "IC5 :: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

}  // namespace mab