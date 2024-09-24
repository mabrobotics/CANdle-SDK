#include "pds_module.hpp"

namespace mab
{

    constexpr logger::LogLevel_E DEFAULT_PDS_MODULE_LOG_LEVEL = logger::LogLevel_E::DEBUG;

    PdsModule::PdsModule(socket_E socket) : m_socketIndex(socket)
    {
        m_log.level = DEFAULT_PDS_MODULE_LOG_LEVEL;
    }

    PdsModule::socket_E PdsModule::getSocket()
    {
        return m_socketIndex;
    }

    BrakeResistor::BrakeResistor(socket_E socket) : PdsModule(socket)
    {
        m_type    = type_E::BRAKE_RESISTOR;
        m_log.tag = "BR  :: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

    PowerStageV1::PowerStageV1(socket_E socket) : PdsModule(socket)
    {
        m_type    = type_E::POWER_STAGE_V1;
        m_log.tag = "PSV1:: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

    PowerStageV2::PowerStageV2(socket_E socket) : PdsModule(socket)
    {
        m_type    = type_E::POWER_STAGE_V2;
        m_log.tag = "PSV2:: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

    IsolatedConv12::IsolatedConv12(socket_E socket) : PdsModule(socket)
    {
        m_type    = type_E::ISOLATED_CONVERTER_12V;
        m_log.tag = "IC12:: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

    IsolatedConv5::IsolatedConv5(socket_E socket) : PdsModule(socket)
    {
        m_type    = type_E::ISOLATED_CONVERTER_5V;
        m_log.tag = "IC5 :: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

}  // namespace mab