#include "pds.hpp"

#include "pds_protocol.hpp"
#include <string>
#include <bit>

enum moduleVersion_E : uint8_t
{
    UNKNOWN = 0x00,
    V0_1,  // 0.1
    V0_2,  // 0.2
    V0_3,  // 0.3
           /* NEW MODULE VERSIONS HERE */
};

namespace mab
{

    Pds::Pds(uint16_t canId, Candle& candle)
        : PdsModule(socketIndex_E::UNASSIGNED, moduleType_E::CONTROL_BOARD, candle, canId),
          m_candle(candle),
          m_canId(canId)
    {
        PdsModule::error_E result;
        m_log.m_tag   = "PDS";
        m_log.m_layer = Logger::ProgramLayer_E::LAYER_2;
        // m_log.g_m_verbosity = Logger::Verbosity_E::VERBOSITY_3;
        result = readModules();
        if (result != PdsModule::error_E ::OK)
            m_log.error("Unable to read modules data from PDS [ %d ]", static_cast<int8_t>(result));
    }

    PdsModule::error_E Pds::readModules(void)
    {
        PdsMessage::error_E result = PdsMessage::error_E::OK;
        PropertyGetMessage  message(moduleType_E::CONTROL_BOARD, socketIndex_E::UNASSIGNED);

        u8     responseBuffer[64] = {0};
        size_t responseLength     = 0;
        u32    rawData            = 0;

        message.addProperty(properties_E::SOCKET_1_MODULE);
        message.addProperty(properties_E::SOCKET_2_MODULE);
        message.addProperty(properties_E::SOCKET_3_MODULE);
        message.addProperty(properties_E::SOCKET_4_MODULE);
        message.addProperty(properties_E::SOCKET_5_MODULE);
        message.addProperty(properties_E::SOCKET_6_MODULE);

        std::vector<u8> serializedMessage = message.serialize();

        if (!m_candle.sendGenericFDCanFrame(m_canId,
                                            serializedMessage.size(),
                                            reinterpret_cast<const char*>(serializedMessage.data()),
                                            reinterpret_cast<char*>(responseBuffer),
                                            &responseLength,
                                            500))
            return PdsModule::error_E ::COMMUNICATION_ERROR;

        result = message.parseResponse(responseBuffer, responseLength);
        if (result != PdsMessage::error_E::OK)
            return PdsModule::error_E ::COMMUNICATION_ERROR;

        result = message.getProperty(properties_E::SOCKET_1_MODULE, &rawData);
        if (result != PdsMessage::error_E::OK)
            return PdsModule::error_E ::COMMUNICATION_ERROR;
        m_moduleTypes[0] = decodeModuleType(rawData);

        result = message.getProperty(properties_E::SOCKET_2_MODULE, &rawData);
        if (result != PdsMessage::error_E::OK)
            return PdsModule::error_E ::COMMUNICATION_ERROR;
        m_moduleTypes[1] = decodeModuleType(rawData);

        result = message.getProperty(properties_E::SOCKET_3_MODULE, &rawData);
        if (result != PdsMessage::error_E::OK)
            return PdsModule::error_E ::COMMUNICATION_ERROR;
        m_moduleTypes[2] = decodeModuleType(rawData);

        result = message.getProperty(properties_E::SOCKET_4_MODULE, &rawData);
        if (result != PdsMessage::error_E::OK)
            return PdsModule::error_E ::COMMUNICATION_ERROR;
        m_moduleTypes[3] = decodeModuleType(rawData);

        result = message.getProperty(properties_E::SOCKET_5_MODULE, &rawData);
        if (result != PdsMessage::error_E::OK)
            return PdsModule::error_E ::COMMUNICATION_ERROR;
        m_moduleTypes[4] = decodeModuleType(rawData);

        result = message.getProperty(properties_E::SOCKET_6_MODULE, &rawData);
        if (result != PdsMessage::error_E::OK)
            return PdsModule::error_E ::COMMUNICATION_ERROR;
        m_moduleTypes[5] = decodeModuleType(rawData);

        for (uint8_t i = 0; i < MAX_MODULES; i++)
        {
            socketIndex_E socketIndex = static_cast<socketIndex_E>(i + 1);
            switch (m_moduleTypes[i])
            {
                case moduleType_E::BRAKE_RESISTOR:
                    m_brakeResistors.push_back(
                        std::make_unique<BrakeResistor>(socketIndex, m_candle, m_canId));
                    break;

                case moduleType_E::ISOLATED_CONVERTER_12V:
                    m_IsolatedConv12s.push_back(
                        std::make_unique<IsolatedConv12>(socketIndex, m_candle, m_canId));
                    break;

                case moduleType_E::ISOLATED_CONVERTER_5V:
                    m_IsolatedConv5s.push_back(
                        std::make_unique<IsolatedConv5>(socketIndex, m_candle, m_canId));
                    break;

                case moduleType_E::POWER_STAGE:
                    m_powerStages.push_back(
                        std::make_unique<PowerStage>(socketIndex, m_candle, m_canId));
                    break;

                case moduleType_E::UNDEFINED:
                default:
                    break;
            }
        }

        return PdsModule::error_E ::OK;
    }

    Pds::modulesSet_t Pds::getModules(void)
    {
        return m_moduleTypes;
    }

    std::unique_ptr<BrakeResistor> Pds::attachBrakeResistor(const socketIndex_E socket)
    {
        if (!m_brakeResistors.empty())
        {
            for (auto& module : m_brakeResistors)
            {
                if (module->getSocketIndex() == socket)
                    return std::move(module);
            }
            m_log.error("No brake resistor module connected to socket [ %u ]!",
                        static_cast<uint8_t>(socket));

            return nullptr;
        }

        m_log.error("No Brake Resistor modules connected to PDS device!");
        return nullptr;
    }

    std::unique_ptr<PowerStage> Pds::attachPowerStage(const socketIndex_E socket)
    {
        if (!m_powerStages.empty())
        {
            for (auto& module : m_powerStages)
            {
                if (module->getSocketIndex() == socket)
                    return std::move(module);
            }

            m_log.error("No power stage module connected to socket [ %u ]!",
                        static_cast<uint8_t>(socket));

            return nullptr;
        }

        m_log.error("No power stage modules connected to PDS device!");
        return nullptr;
    }

    std::unique_ptr<IsolatedConv12> Pds::attachIsolatedConverter12(const socketIndex_E socket)
    {
        if (!m_IsolatedConv12s.empty())
        {
            for (auto& module : m_IsolatedConv12s)
            {
                if (module->getSocketIndex() == socket)
                    return std::move(module);
            }

            m_log.error("No Isolated Converter 12V module connected to socket [ %u ]!",
                        static_cast<uint8_t>(socket));

            return nullptr;
        }

        m_log.error("No Isolated Converter 12V modules connected to PDS device!");
        return nullptr;
    }

    std::unique_ptr<IsolatedConv5> Pds::attachIsolatedConverter5(const socketIndex_E socket)
    {
        if (!m_IsolatedConv5s.empty())
        {
            for (auto& module : m_IsolatedConv5s)
            {
                if (module->getSocketIndex() == socket)
                    return std::move(module);
            }

            m_log.error("No Isolated Converter 5V module connected to socket [ %u ]!",
                        static_cast<uint8_t>(socket));

            return nullptr;
        }

        m_log.error("No Isolated Converter 5V modules connected to PDS device!");
        return nullptr;
    }

    PdsModule::error_E Pds::getBusVoltage(u32& busVoltage)
    {
        return readModuleProperty(properties_E::BUS_VOLTAGE, busVoltage);
    }

    PdsModule::error_E Pds::getTemperature(f32& temperature)
    {
        return readModuleProperty(properties_E::TEMPERATURE, temperature);
    }

    moduleType_E Pds::decodeModuleType(uint8_t moduleTypeCode)
    {
        switch (moduleTypeCode)
        {
            case static_cast<u8>(moduleType_E::CONTROL_BOARD):
                return moduleType_E::CONTROL_BOARD;

            case static_cast<u8>(moduleType_E::BRAKE_RESISTOR):
                return moduleType_E::BRAKE_RESISTOR;

            case static_cast<u8>(moduleType_E::ISOLATED_CONVERTER_12V):
                return moduleType_E::ISOLATED_CONVERTER_12V;

            case static_cast<u8>(moduleType_E::ISOLATED_CONVERTER_5V):
                return moduleType_E::ISOLATED_CONVERTER_5V;

            case static_cast<u8>(moduleType_E::POWER_STAGE):
                return moduleType_E::POWER_STAGE;

            default:
                return moduleType_E::UNDEFINED;
        }
    }

    const char* Pds::moduleTypeToString(moduleType_E type)
    {
        switch (type)
        {
            case moduleType_E::UNDEFINED:
                return "UNDEFINED";
            case moduleType_E::CONTROL_BOARD:
                return "CONTROL_BOARD";
            case moduleType_E::BRAKE_RESISTOR:
                return "BRAKE_RESISTOR";
            case moduleType_E::ISOLATED_CONVERTER_12V:
                return "ISOLATED_CONVERTER_12V";
            case moduleType_E::ISOLATED_CONVERTER_5V:
                return "ISOLATED_CONVERTER_5V";
            case moduleType_E::POWER_STAGE:
                return "POWER_STAGE";
            /* NEW MODULE TYPES HANDLED HERE */
            default:
                return "UNKNOWN_MODULE_TYPE";
        }
    }

}  // namespace mab