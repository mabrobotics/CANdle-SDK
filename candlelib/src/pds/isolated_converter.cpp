#include "isolated_converter.hpp"

namespace mab
{

    IsolatedConv12::IsolatedConv12(socketIndex_E socket, Candle& candle, u16 canId)
        : PdsModule(socket, moduleType_E::ISOLATED_CONVERTER_12V, candle, canId)
    {
        m_log.m_tag = "IC12:: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

    PdsModule::error_E IsolatedConv12::enable()
    {
        return writeModuleProperty(properties_E::ENABLED, true);
    }

    PdsModule::error_E IsolatedConv12::disable()
    {
        return writeModuleProperty(properties_E::ENABLED, false);
    }

    IsolatedConv5::IsolatedConv5(socketIndex_E socket, Candle& candle, u16 canId)
        : PdsModule(socket, moduleType_E::ISOLATED_CONVERTER_5V, candle, canId)
    {
        m_log.m_tag = "IC5 :: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }
}  // namespace mab