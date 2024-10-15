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

    socketIndex_E PdsModule::getSocketIndex()
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

        u8     responseBuffer[64] = {0};
        size_t responseLength     = 0;

        enableMessage.addProperty(properties_E::ENABLED, true);
        std::vector<u8> serializedMessage = enableMessage.serialize();

        msp_Candle->sendGenericFDCanFrame(m_canId,
                                          serializedMessage.size(),
                                          reinterpret_cast<const char*>(serializedMessage.data()),
                                          reinterpret_cast<char*>(responseBuffer),
                                          &responseLength);

        result = enableMessage.parseResponse(responseBuffer, responseLength);

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

        u8     responseBuffer[64] = {0};
        size_t responseLength     = 0;

        enableMessage.addProperty(properties_E::ENABLED, false);
        std::vector<u8> serializedMessage = enableMessage.serialize();

        msp_Candle->sendGenericFDCanFrame(m_canId,
                                          serializedMessage.size(),
                                          reinterpret_cast<const char*>(serializedMessage.data()),
                                          reinterpret_cast<char*>(responseBuffer),
                                          &responseLength);

        result = enableMessage.parseResponse(responseBuffer, responseLength);

        if (result != PdsMessage::error_E::OK)
        {
            return error_E::PROTOCOL_ERROR;
        }

        return error_E::OK;
    }

    PdsModule::error_E PowerStage::getEnabled(bool& enabled)
    {
        PdsMessage::error_E result = PdsMessage::error_E::OK;

        uint32_t receivedPropertyBuffer = 0;

        PropertyGetMessage getEnabledMessage(m_type, m_socketIndex);

        u8     responseBuffer[64] = {0};
        size_t responseLength     = 0;

        getEnabledMessage.addProperty(PowerStage::properties_E::ENABLED);

        std::vector<u8> serializedMessage = getEnabledMessage.serialize();
        msp_Candle->sendGenericFDCanFrame(m_canId,
                                          serializedMessage.size(),
                                          reinterpret_cast<const char*>(serializedMessage.data()),
                                          reinterpret_cast<char*>(responseBuffer),
                                          &responseLength);

        result = getEnabledMessage.parseResponse(responseBuffer, responseLength);
        if (result != PdsMessage::error_E::OK)
            return error_E::PROTOCOL_ERROR;

        result = getEnabledMessage.getProperty(PowerStage::properties_E::ENABLED,
                                               &receivedPropertyBuffer);
        if (result != PdsMessage::error_E::OK)
            return error_E::PROTOCOL_ERROR;

        enabled = static_cast<bool>(receivedPropertyBuffer);

        return error_E::OK;
    }

    PdsModule::error_E PowerStage::bindBrakeResistor(socketIndex_E brakeResistorSocketIndex)
    {
        PdsMessage::error_E result = PdsMessage::error_E::OK;
        PropertySetMessage  message(m_type, m_socketIndex);
        u8                  responseBuffer[64] = {0};
        size_t              responseLength     = 0;

        m_log.debug("Binding power stage [ %u ] with brake resistor [ %u ]");

        message.addProperty(properties_E::BR_SOCKET_INDEX, brakeResistorSocketIndex);
        std::vector<u8> serializedMessage = message.serialize();

        if (!(msp_Candle->sendGenericFDCanFrame(
                m_canId,
                serializedMessage.size(),
                reinterpret_cast<const char*>(serializedMessage.data()),
                reinterpret_cast<char*>(responseBuffer),
                &responseLength)))
        {
            m_log.error("Communication error");
            return error_E::COMMUNICATION_ERROR;
        }

        result = message.parseResponse(responseBuffer, responseLength);
        if (result != PdsMessage::error_E::OK)
        {
            m_log.error("PDS Message error [ %u ]", static_cast<u8>(result));
            return error_E::PROTOCOL_ERROR;
        }

        return error_E::OK;
    }

    PdsModule::error_E PowerStage::setBrakeResistorTriggerVoltage(uint32_t brTriggerVoltage)
    {
        PdsMessage::error_E result = PdsMessage::error_E::OK;
        PropertySetMessage  message(m_type, m_socketIndex);
        u8                  responseBuffer[64] = {0};
        size_t              responseLength     = 0;

        m_log.debug("Binding power stage [ %u ] with brake resistor [ %u ]");

        message.addProperty(properties_E::BR_TRIGGER_VOLTAGE, brTriggerVoltage);
        std::vector<u8> serializedMessage = message.serialize();

        if (!(msp_Candle->sendGenericFDCanFrame(
                m_canId,
                serializedMessage.size(),
                reinterpret_cast<const char*>(serializedMessage.data()),
                reinterpret_cast<char*>(responseBuffer),
                &responseLength)))
        {
            return error_E::COMMUNICATION_ERROR;
        }

        result = message.parseResponse(responseBuffer, responseLength);
        if (result != PdsMessage::error_E::OK)
            return error_E::PROTOCOL_ERROR;

        return error_E::OK;
    }

    PdsModule::error_E PowerStage::getOutputVoltage(u32& outputVoltage)
    {
        PdsMessage::error_E result = PdsMessage::error_E::OK;
        PropertyGetMessage  message(m_type, m_socketIndex);

        u8     responseBuffer[64] = {0};
        size_t responseLength     = 0;

        message.addProperty(PowerStage::properties_E::BUS_VOLTAGE);

        std::vector<u8> serializedMessage = message.serialize();
        msp_Candle->sendGenericFDCanFrame(m_canId,
                                          serializedMessage.size(),
                                          reinterpret_cast<const char*>(serializedMessage.data()),
                                          reinterpret_cast<char*>(responseBuffer),
                                          &responseLength);

        result = message.parseResponse(responseBuffer, responseLength);
        if (result != PdsMessage::error_E::OK)
            return error_E::PROTOCOL_ERROR;

        result = message.getProperty(PowerStage::properties_E::BUS_VOLTAGE, &outputVoltage);
        if (result != PdsMessage::error_E::OK)
            return error_E::PROTOCOL_ERROR;

        return error_E::OK;
    }

    PdsModule::error_E PowerStage::getLoadCurrent(s32& loadCurrent)
    {
        PdsMessage::error_E result = PdsMessage::error_E::OK;
        PropertyGetMessage  message(m_type, m_socketIndex);

        u8     responseBuffer[64] = {0};
        size_t responseLength     = 0;
        u32    rawCurrentData     = 0;

        message.addProperty(PowerStage::properties_E::LOAD_CURRENT);

        std::vector<u8> serializedMessage = message.serialize();
        msp_Candle->sendGenericFDCanFrame(m_canId,
                                          serializedMessage.size(),
                                          reinterpret_cast<const char*>(serializedMessage.data()),
                                          reinterpret_cast<char*>(responseBuffer),
                                          &responseLength);

        result = message.parseResponse(responseBuffer, responseLength);
        if (result != PdsMessage::error_E::OK)
            return error_E::PROTOCOL_ERROR;

        result = message.getProperty(PowerStage::properties_E::LOAD_CURRENT, &rawCurrentData);
        if (result != PdsMessage::error_E::OK)
            return error_E::PROTOCOL_ERROR;

        loadCurrent = *reinterpret_cast<s32*>(&rawCurrentData);

        return error_E::OK;
    }

    PdsModule::error_E PowerStage::getPower(s32& power)
    {
        PdsMessage::error_E result = PdsMessage::error_E::OK;
        PropertyGetMessage  message(m_type, m_socketIndex);

        u8     responseBuffer[64] = {0};
        size_t responseLength     = 0;
        u32    rawData            = 0;

        message.addProperty(PowerStage::properties_E::LOAD_POWER);

        std::vector<u8> serializedMessage = message.serialize();
        msp_Candle->sendGenericFDCanFrame(m_canId,
                                          serializedMessage.size(),
                                          reinterpret_cast<const char*>(serializedMessage.data()),
                                          reinterpret_cast<char*>(responseBuffer),
                                          &responseLength);

        result = message.parseResponse(responseBuffer, responseLength);
        if (result != PdsMessage::error_E::OK)
            return error_E::PROTOCOL_ERROR;

        result = message.getProperty(PowerStage::properties_E::LOAD_POWER, &rawData);
        if (result != PdsMessage::error_E::OK)
            return error_E::PROTOCOL_ERROR;

        power = *reinterpret_cast<s32*>(&rawData);

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

        u8     responseBuffer[64] = {0};
        size_t responseLength     = 0;

        enableMessage.addProperty(controlParameters_E::ENABLED, true);
        std::vector<u8> serializedMessage = enableMessage.serialize();

        msp_Candle->sendGenericFDCanFrame(m_canId,
                                          serializedMessage.size(),
                                          reinterpret_cast<const char*>(serializedMessage.data()),
                                          reinterpret_cast<char*>(responseBuffer),
                                          &responseLength);

        result = enableMessage.parseResponse(responseBuffer, responseLength);

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

        u8     responseBuffer[64] = {0};
        size_t responseLength     = 0;

        enableMessage.addProperty(controlParameters_E::ENABLED, false);
        std::vector<u8> serializedMessage = enableMessage.serialize();

        msp_Candle->sendGenericFDCanFrame(m_canId,
                                          serializedMessage.size(),
                                          reinterpret_cast<const char*>(serializedMessage.data()),
                                          reinterpret_cast<char*>(responseBuffer),
                                          &responseLength);

        result = enableMessage.parseResponse(responseBuffer, responseLength);

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