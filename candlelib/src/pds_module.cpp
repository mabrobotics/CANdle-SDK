#include "pds_module.hpp"
#include "pds_protocol.hpp"

namespace mab
{

    constexpr Logger::ProgramLayer_E DEFAULT_PDS_MODULE_LOG_LAYER = Logger::ProgramLayer_E::LAYER_2;

    PdsModule::PdsModule(socketIndex_E           socket,
                         moduleType_E            type,
                         std::shared_ptr<Candle> sp_Candle,
                         u16                     canId)
        : m_socketIndex(socket), m_type(type), msp_Candle(sp_Candle), m_canId(canId)
    {
        m_log.m_layer = DEFAULT_PDS_MODULE_LOG_LAYER;
    }

    socketIndex_E PdsModule::getSocket()
    {
        return m_socketIndex;
    }

    BrakeResistor::BrakeResistor(socketIndex_E socket, std::shared_ptr<Candle> sp_Candle, u16 canId)
        : PdsModule(socket, moduleType_E::BRAKE_RESISTOR, sp_Candle, canId)
    {
        m_log.m_tag = "BR  :: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

    PowerStage::PowerStage(socketIndex_E socket, std::shared_ptr<Candle> sp_Candle, u16 canId)
        : PdsModule(socket, moduleType_E::POWER_STAGE, sp_Candle, canId)
    {
        m_log.m_tag = "PS  :: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

    PdsModule::error_E PowerStage::enable()
    {
        PdsMessage::error_E result = PdsMessage::error_E::OK;
        PropertySetMessage  enableMessage(m_type, m_socketIndex);

        u8 responseBuffer[64] = {0};

        enableMessage.addProperty(controlParameters_E::ENABLED, true);
        std::vector<u8> serializedMessage = enableMessage.serialize();

        msp_Candle->sendGenericFDCanFrame(m_canId,
                                          serializedMessage.size(),
                                          reinterpret_cast<const char*>(serializedMessage.data()),
                                          reinterpret_cast<char*>(responseBuffer));

        result = enableMessage.parseResponse(responseBuffer);

        if (result != PdsMessage::error_E::OK)
        {
            return error_E::PROTOCOL_ERROR;
        }

        return error_E::OK;
    }

    PdsModule::error_E PowerStage::disable()
    {
        PdsMessage::error_E result = PdsMessage::error_E::OK;
        PropertySetMessage  enableMessage(m_type, m_socketIndex);

        u8 responseBuffer[64] = {0};

        enableMessage.addProperty(controlParameters_E::ENABLED, false);
        std::vector<u8> serializedMessage = enableMessage.serialize();

        msp_Candle->sendGenericFDCanFrame(m_canId,
                                          serializedMessage.size(),
                                          reinterpret_cast<const char*>(serializedMessage.data()),
                                          reinterpret_cast<char*>(responseBuffer));

        result = enableMessage.parseResponse(responseBuffer);

        if (result != PdsMessage::error_E::OK)
        {
            return error_E::PROTOCOL_ERROR;
        }

        return error_E::OK;
    }

    IsolatedConv12::IsolatedConv12(socketIndex_E           socket,
                                   std::shared_ptr<Candle> sp_Candle,
                                   u16                     canId)
        : PdsModule(socket, moduleType_E::ISOLATED_CONVERTER_12V, sp_Candle, canId)
    {
        m_log.m_tag = "IC12:: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

    PdsModule::error_E IsolatedConv12::enable()
    {
        PdsMessage::error_E result = PdsMessage::error_E::OK;
        PropertySetMessage  enableMessage(m_type, m_socketIndex);

        u8 responseBuffer[64] = {0};

        enableMessage.addProperty(controlParameters_E::ENABLED, true);
        std::vector<u8> serializedMessage = enableMessage.serialize();

        msp_Candle->sendGenericFDCanFrame(m_canId,
                                          serializedMessage.size(),
                                          reinterpret_cast<const char*>(serializedMessage.data()),
                                          reinterpret_cast<char*>(responseBuffer));

        result = enableMessage.parseResponse(responseBuffer);

        if (result != PdsMessage::error_E::OK)
        {
            return error_E::PROTOCOL_ERROR;
        }

        return error_E::OK;
    }

    PdsModule::error_E IsolatedConv12::disable()
    {
        PdsMessage::error_E result = PdsMessage::error_E::OK;
        PropertySetMessage  enableMessage(m_type, m_socketIndex);

        u8 responseBuffer[64] = {0};

        enableMessage.addProperty(controlParameters_E::ENABLED, false);
        std::vector<u8> serializedMessage = enableMessage.serialize();

        msp_Candle->sendGenericFDCanFrame(m_canId,
                                          serializedMessage.size(),
                                          reinterpret_cast<const char*>(serializedMessage.data()),
                                          reinterpret_cast<char*>(responseBuffer));

        result = enableMessage.parseResponse(responseBuffer);

        if (result != PdsMessage::error_E::OK)
        {
            return error_E::PROTOCOL_ERROR;
        }

        return error_E::OK;
    }

    IsolatedConv5::IsolatedConv5(socketIndex_E socket, std::shared_ptr<Candle> sp_Candle, u16 canId)
        : PdsModule(socket, moduleType_E::ISOLATED_CONVERTER_5V, sp_Candle, canId)
    {
        m_log.m_tag = "IC5 :: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

}  // namespace mab