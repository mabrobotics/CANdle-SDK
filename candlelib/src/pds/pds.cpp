#include "pds.hpp"

namespace mab
{
    Pds::Pds(u16 canId, Candle* p_candle)
        : PdsModule(socketIndex_E::UNASSIGNED, moduleType_E::CONTROL_BOARD, p_candle, m_rootCanId),
          mp_candle(p_candle),
          m_rootCanId(canId)
    {
        m_log.m_tag   = "PDS";
        m_log.m_layer = Logger::ProgramLayer_E::LAYER_2;
    }

    void Pds::printModuleInfo(void)
    {
        // TODO
    }

    void Pds::init(void)
    {
        PdsModule::error_E result = readModules();
        if (result != PdsModule::error_E ::OK)
        {
            m_log.error("Reading PDS submodules failed! [ %s ]", PdsModule::error2String(result));
            // TODO: How to handle this error?
        }
    }
    void Pds::init(u16 canId)
    {
        m_canId = canId;
        init();
    }

    PdsModule::error_E Pds::createModule(moduleType_E type, socketIndex_E socket)
    {
        switch (type)
        {
            case moduleType_E::BRAKE_RESISTOR:
                m_brakeResistors.push_back(
                    std::make_shared<BrakeResistor>(socket, mp_candle, m_rootCanId));
                return PdsModule::error_E::OK;

            case moduleType_E::ISOLATED_CONVERTER:
                m_IsolatedConvs.push_back(
                    std::make_shared<IsolatedConv>(socket, mp_candle, m_rootCanId));
                return PdsModule::error_E::OK;

            case moduleType_E::POWER_STAGE:
                m_powerStages.push_back(
                    std::make_shared<PowerStage>(socket, mp_candle, m_rootCanId));
                return PdsModule::error_E::OK;

            case moduleType_E::UNDEFINED:
            default:
                return PdsModule::error_E::INTERNAL_ERROR;
        }
    }

    PdsModule::error_E Pds::readModules(void)
    {
        PdsMessage::error_E result = PdsMessage::error_E::OK;
        PropertyGetMessage  message(moduleType_E::CONTROL_BOARD, socketIndex_E::UNASSIGNED);

        std::pair<std::vector<u8>, mab::candleTypes::Error_t> transferResult;
        // u8     responseBuffer[64] = {0};
        // size_t responseLength     = 0;
        u32 rawData = 0;

        message.addProperty(propertyId_E::SOCKET_1_MODULE);
        message.addProperty(propertyId_E::SOCKET_2_MODULE);
        message.addProperty(propertyId_E::SOCKET_3_MODULE);
        message.addProperty(propertyId_E::SOCKET_4_MODULE);
        message.addProperty(propertyId_E::SOCKET_5_MODULE);
        message.addProperty(propertyId_E::SOCKET_6_MODULE);

        std::vector<u8> serializedMessage = message.serialize();

        transferResult = mp_candle->transferCANFrame(m_canId, serializedMessage, 66U);
        if (transferResult.second != mab::candleTypes::Error_t::OK)
        {
            m_log.error("Failed to transfer CAN frame");
            return PdsModule::error_E ::COMMUNICATION_ERROR;
        }

        result = message.parseResponse(transferResult.first.data(), transferResult.first.size());

        if (result != PdsMessage::error_E::OK)
            return PdsModule::error_E ::COMMUNICATION_ERROR;

        result = message.getProperty(propertyId_E::SOCKET_1_MODULE, &rawData);
        if (result != PdsMessage::error_E::OK)
            return PdsModule::error_E ::COMMUNICATION_ERROR;
        m_modulesSet.moduleTypeSocket1 = decodeModuleType(rawData);
        createModule(m_modulesSet.moduleTypeSocket1, socketIndex_E::SOCKET_1);

        result = message.getProperty(propertyId_E::SOCKET_2_MODULE, &rawData);
        if (result != PdsMessage::error_E::OK)
            return PdsModule::error_E ::COMMUNICATION_ERROR;
        m_modulesSet.moduleTypeSocket2 = decodeModuleType(rawData);
        createModule(m_modulesSet.moduleTypeSocket2, socketIndex_E::SOCKET_2);

        result = message.getProperty(propertyId_E::SOCKET_3_MODULE, &rawData);
        if (result != PdsMessage::error_E::OK)
            return PdsModule::error_E ::COMMUNICATION_ERROR;
        m_modulesSet.moduleTypeSocket3 = decodeModuleType(rawData);
        createModule(m_modulesSet.moduleTypeSocket3, socketIndex_E::SOCKET_3);

        result = message.getProperty(propertyId_E::SOCKET_4_MODULE, &rawData);
        if (result != PdsMessage::error_E::OK)
            return PdsModule::error_E ::COMMUNICATION_ERROR;
        m_modulesSet.moduleTypeSocket4 = decodeModuleType(rawData);
        createModule(m_modulesSet.moduleTypeSocket4, socketIndex_E::SOCKET_4);

        result = message.getProperty(propertyId_E::SOCKET_5_MODULE, &rawData);
        if (result != PdsMessage::error_E::OK)
            return PdsModule::error_E ::COMMUNICATION_ERROR;
        m_modulesSet.moduleTypeSocket5 = decodeModuleType(rawData);
        createModule(m_modulesSet.moduleTypeSocket5, socketIndex_E::SOCKET_5);

        result = message.getProperty(propertyId_E::SOCKET_6_MODULE, &rawData);
        if (result != PdsMessage::error_E::OK)
            return PdsModule::error_E ::COMMUNICATION_ERROR;
        m_modulesSet.moduleTypeSocket6 = decodeModuleType(rawData);
        createModule(m_modulesSet.moduleTypeSocket6, socketIndex_E::SOCKET_6);

        return PdsModule::error_E ::OK;
    }

    PdsModule::error_E Pds::getFwMetadata(pdsFwMetadata_S& metadata) const
    {
        msgResponse_E responseStatusCode = msgResponse_E::UNKNOWN_ERROR;
        std::pair<std::vector<u8>, mab::candleTypes::Error_t> transferResult;

        std::vector<u8> getFwMetadataMessage = {
            static_cast<u8>(PdsMessage::commandCode_E::GET_FW_METADATA)};

        transferResult = mp_candle->transferCANFrame(m_canId, getFwMetadataMessage, 66U);

        if (transferResult.second != mab::candleTypes::Error_t::OK)
        {
            m_log.error("Failed to transfer CAN frame");
            return PdsModule::error_E ::COMMUNICATION_ERROR;
        }

        responseStatusCode = (msgResponse_E)*transferResult.first.data();
        if (responseStatusCode != msgResponse_E::OK)
        {
            m_log.error("Failed to get firmware metadata! [ %u ]",
                        static_cast<uint8_t>(responseStatusCode));
            return PdsModule::error_E ::PROTOCOL_ERROR;
        }

        size_t responseSize = (u8) * (transferResult.first.data() + 1);

        memcpy(&metadata, transferResult.first.data() + 2, sizeof(pdsFwMetadata_S));

        return PdsModule::error_E ::OK;
    }

    Pds::modulesSet_S Pds::getModules(void)
    {
        return m_modulesSet;
    }

    bool Pds::verifyModuleSocket(moduleType_E type, socketIndex_E socket)
    {
        // Check if module of type <type> is available on socket <socket>
        switch (socket)
        {
            case socketIndex_E::SOCKET_1:
                return m_modulesSet.moduleTypeSocket1 == type;

            case socketIndex_E::SOCKET_2:
                return m_modulesSet.moduleTypeSocket2 == type;

            case socketIndex_E::SOCKET_3:
                return m_modulesSet.moduleTypeSocket3 == type;

            case socketIndex_E::SOCKET_4:
                return m_modulesSet.moduleTypeSocket4 == type;

            case socketIndex_E::SOCKET_5:
                return m_modulesSet.moduleTypeSocket5 == type;

            case socketIndex_E::SOCKET_6:
                return m_modulesSet.moduleTypeSocket6 == type;

            default:
                return false;
        }
    }

    std::shared_ptr<BrakeResistor> Pds::attachBrakeResistor(const socketIndex_E socket)
    {
        if (!m_brakeResistors.empty())
        {
            for (auto module = m_brakeResistors.begin(); module != m_brakeResistors.end(); module++)
            {
                if (*module == nullptr)
                {
                    m_log.error("Brake resistor has some dangling pointers and will fail!");
                    return nullptr;
                }
                if ((*module)->getSocketIndex() == socket)
                {
                    return *module;
                }
            }
            m_log.error("No brake resistor module connected to socket [ %u ]!",
                        static_cast<uint8_t>(socket));

            return nullptr;
        }

        m_log.error("No Brake Resistor modules connected to PDS device!");
        return nullptr;
    }

    std::shared_ptr<PowerStage> Pds::attachPowerStage(const socketIndex_E socket)
    {
        if (!m_powerStages.empty())
        {
            for (auto module = m_powerStages.begin(); module != m_powerStages.end(); module++)
            {
                if (*module == nullptr)
                {
                    m_log.error("Power stage has some dangling pointers and will fail!");
                    return nullptr;
                }
                if ((*module)->getSocketIndex() == socket)
                {
                    return *module;
                }
            }

            m_log.error("No power stage module connected to socket [ %u ]!",
                        static_cast<uint8_t>(socket));

            return nullptr;
        }

        m_log.error("No power stage modules connected to PDS device!");
        return nullptr;
    }

    std::shared_ptr<IsolatedConv> Pds::attachIsolatedConverter(const socketIndex_E socket)
    {
        if (!m_IsolatedConvs.empty())
        {
            for (auto module = m_IsolatedConvs.begin(); module != m_IsolatedConvs.end(); module++)
            {
                if (*module == nullptr)
                {
                    m_log.error("Isolated converter has some dangling pointers and will fail!");
                    return nullptr;
                }
                if ((*module)->getSocketIndex() == socket)
                {
                    return *module;
                }
            }

            m_log.error("No Isolated Converter module connected to socket [ %u ]!",
                        static_cast<uint8_t>(socket));

            return nullptr;
        }

        m_log.error("No Isolated Converter modules connected to PDS device!");
        return nullptr;
    }

    PdsModule::error_E Pds::getStatus(controlBoardStatus_S& status)
    {
        u32                statusWord = 0;
        PdsModule::error_E result     = readModuleProperty(propertyId_E::STATUS_WORD, statusWord);

        if (result != PdsModule::error_E::OK)
            return result;

        status.ENABLED          = statusWord & (u32)statusBits_E::ENABLED;
        status.OVER_TEMPERATURE = statusWord & (u32)statusBits_E::OVER_TEMPERATURE;
        // status.OVER_CURRENT       = statusWord & (u32)statusBits_E::OVER_CURRENT;
        status.STO_1              = statusWord & (u32)statusBits_E::STO_1;
        status.STO_2              = statusWord & (u32)statusBits_E::STO_2;
        status.FDCAN_TIMEOUT      = statusWord & (u32)statusBits_E::FDCAN_TIMEOUT;
        status.SUBMODULE_1_ERROR  = statusWord & (u32)statusBits_E::SUBMODULE_1_ERROR;
        status.SUBMODULE_2_ERROR  = statusWord & (u32)statusBits_E::SUBMODULE_2_ERROR;
        status.SUBMODULE_3_ERROR  = statusWord & (u32)statusBits_E::SUBMODULE_3_ERROR;
        status.SUBMODULE_4_ERROR  = statusWord & (u32)statusBits_E::SUBMODULE_4_ERROR;
        status.SUBMODULE_5_ERROR  = statusWord & (u32)statusBits_E::SUBMODULE_5_ERROR;
        status.SUBMODULE_6_ERROR  = statusWord & (u32)statusBits_E::SUBMODULE_6_ERROR;
        status.CHARGER_DETECTED   = statusWord & (u32)statusBits_E::CHARGER_DETECTED;
        status.SHUTDOWN_SCHEDULED = statusWord & (u32)statusBits_E::SHUTDOWN_SCHEDULED;

        return result;
    }

    PdsModule::error_E Pds::clearStatus(controlBoardStatus_S status)
    {
        u32 statusClearWord = 0;

        statusClearWord |= status.ENABLED ? (u32)statusBits_E::ENABLED : 0;
        statusClearWord |= status.OVER_TEMPERATURE ? (u32)statusBits_E::OVER_TEMPERATURE : 0;
        // statusClearWord |= status.OVER_CURRENT ? (u32)statusBits_E::OVER_CURRENT : 0;
        statusClearWord |= status.STO_1 ? (u32)statusBits_E::STO_1 : 0;
        statusClearWord |= status.STO_2 ? (u32)statusBits_E::STO_2 : 0;
        statusClearWord |= status.FDCAN_TIMEOUT ? (u32)statusBits_E::FDCAN_TIMEOUT : 0;
        statusClearWord |= status.SUBMODULE_1_ERROR ? (u32)statusBits_E::SUBMODULE_1_ERROR : 0;
        statusClearWord |= status.SUBMODULE_2_ERROR ? (u32)statusBits_E::SUBMODULE_2_ERROR : 0;
        statusClearWord |= status.SUBMODULE_3_ERROR ? (u32)statusBits_E::SUBMODULE_3_ERROR : 0;
        statusClearWord |= status.SUBMODULE_4_ERROR ? (u32)statusBits_E::SUBMODULE_4_ERROR : 0;
        statusClearWord |= status.SUBMODULE_5_ERROR ? (u32)statusBits_E::SUBMODULE_5_ERROR : 0;
        statusClearWord |= status.SUBMODULE_6_ERROR ? (u32)statusBits_E::SUBMODULE_6_ERROR : 0;
        statusClearWord |= status.CHARGER_DETECTED ? (u32)statusBits_E::CHARGER_DETECTED : 0;
        statusClearWord |= status.SHUTDOWN_SCHEDULED ? (u32)statusBits_E::SHUTDOWN_SCHEDULED : 0;

        return writeModuleProperty(propertyId_E::STATUS_CLEAR, statusClearWord);
    }

    PdsModule::error_E Pds::clearErrors(void)
    {
        u32 statusClearWord = 0;
        statusClearWord |= (u32)statusBits_E::OVER_TEMPERATURE;
        statusClearWord |= (u32)statusBits_E::FDCAN_TIMEOUT;
        return writeModuleProperty(propertyId_E::STATUS_CLEAR, statusClearWord);
    }

    PdsModule::error_E Pds::getBusVoltage(u32& busVoltage)
    {
        return readModuleProperty(propertyId_E::BUS_VOLTAGE, busVoltage);
    }

    PdsModule::error_E Pds::getTemperature(f32& temperature)
    {
        return readModuleProperty(propertyId_E::TEMPERATURE, temperature);
    }

    moduleType_E Pds::decodeModuleType(uint8_t moduleTypeCode)
    {
        switch (moduleTypeCode)
        {
            case static_cast<u8>(moduleType_E::CONTROL_BOARD):
                return moduleType_E::CONTROL_BOARD;

            case static_cast<u8>(moduleType_E::BRAKE_RESISTOR):
                return moduleType_E::BRAKE_RESISTOR;

            case static_cast<u8>(moduleType_E::ISOLATED_CONVERTER):
                return moduleType_E::ISOLATED_CONVERTER;

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
                return "NO MODULE CONNECTED";
            case moduleType_E::CONTROL_BOARD:
                return "CONTROL_BOARD";
            case moduleType_E::BRAKE_RESISTOR:
                return "BRAKE_RESISTOR";
            case moduleType_E::ISOLATED_CONVERTER:
                return "ISOLATED_CONVERTER";
            case moduleType_E::POWER_STAGE:
                return "POWER_STAGE";
            /* NEW MODULE TYPES HANDLED HERE */
            default:
                return "UNKNOWN_MODULE_TYPE";
        }
    }

    u16 Pds::getCanId()
    {
        return m_rootCanId;
    }

    PdsModule::error_E Pds::setCanId(u16 canId)
    {
        m_log.debug("Setting new CAN ID [ %u ]", canId);
        PdsModule::error_E result = PdsModule::error_E::OK;
        result                    = writeModuleProperty(propertyId_E::CAN_ID, canId);
        if (PdsModule::error_E::OK == result)
            m_rootCanId = canId;

        return result;
    }

    CANdleBaudrate_E Pds::getCanBaudrate(void)
    {
        return mp_candle->m_canBaudrate;
    }

    PdsModule::error_E Pds::setCanBaudrate(CANdleBaudrate_E canBaudrate)
    {
        return writeModuleProperty(propertyId_E::CAN_BAUDRATE, canBaudrate);
    }

    PdsModule::error_E Pds::getTemperatureLimit(f32& temperatureLimit)
    {
        return readModuleProperty(propertyId_E::TEMPERATURE_LIMIT, temperatureLimit);
    }

    PdsModule::error_E Pds::setTemperatureLimit(f32 temperatureLimit)
    {
        return writeModuleProperty(propertyId_E::TEMPERATURE_LIMIT, temperatureLimit);
    }

    PdsModule::error_E Pds::getShutdownTime(u32& shutdownTime)
    {
        return readModuleProperty(propertyId_E::SHUTDOWN_TIME, shutdownTime);
    }

    PdsModule::error_E Pds::setShutdownTime(u32 shutdownTime)
    {
        return writeModuleProperty(propertyId_E::SHUTDOWN_TIME, shutdownTime);
    }

    PdsModule::error_E Pds::getBatteryVoltageLevels(u32& batteryLvl1, u32& batteryLvl2)
    {
        PdsModule::error_E result = PdsModule::error_E::OK;

        result = readModuleProperty(propertyId_E::BATTERY_VOLTAGE_L1, batteryLvl1);
        if (result != PdsModule::error_E::OK)
        {
            m_log.error("Reading battery voltage level 1 failed! [ %u ]", result);
            return error_E::PROTOCOL_ERROR;
        }

        result = readModuleProperty(propertyId_E::BATTERY_VOLTAGE_L2, batteryLvl2);
        if (result != PdsModule::error_E::OK)
        {
            m_log.error("Reading battery voltage level 2 failed! [ %u ]", result);
            return error_E::PROTOCOL_ERROR;
        }

        return error_E::OK;
    }

    PdsModule::error_E Pds::setBatteryVoltageLevels(u32 batteryLvl1, u32 batteryLvl2)
    {
        PdsModule::error_E result = PdsModule::error_E::OK;

        result = writeModuleProperty(propertyId_E::BATTERY_VOLTAGE_L2, batteryLvl2);
        if (result != PdsModule::error_E::OK)
        {
            m_log.error("Writing battery voltage level 2 failed! [ %u ]", result);
            return error_E::PROTOCOL_ERROR;
        }

        result = writeModuleProperty(propertyId_E::BATTERY_VOLTAGE_L1, batteryLvl1);
        if (result != PdsModule::error_E::OK)
        {
            m_log.error("Writing battery voltage level 1 failed! [ %u ]", result);
            return error_E::PROTOCOL_ERROR;
        }

        return error_E::OK;
    }

    PdsModule::error_E Pds::bindBrakeResistor(socketIndex_E brakeResistorSocketIndex)
    {
        return writeModuleProperty(propertyId_E::BR_SOCKET_INDEX, brakeResistorSocketIndex);
    }

    PdsModule::error_E Pds::getBindBrakeResistor(socketIndex_E& brakeResistorSocketIndex)
    {
        return readModuleProperty(propertyId_E::BR_SOCKET_INDEX, brakeResistorSocketIndex);
    }

    PdsModule::error_E Pds::setBrakeResistorTriggerVoltage(uint32_t brTriggerVoltage)
    {
        return writeModuleProperty(propertyId_E::BR_TRIGGER_VOLTAGE, brTriggerVoltage);
    }

    PdsModule::error_E Pds::getBrakeResistorTriggerVoltage(u32& brTriggerVoltage)
    {
        return readModuleProperty(propertyId_E::BR_TRIGGER_VOLTAGE, brTriggerVoltage);
    }

    PdsModule::error_E Pds::shutdown(void)
    {
        return writeModuleProperty(propertyId_E::COMMAND, commands_E::SHUTDOWN);
    }

    PdsModule::error_E Pds::reboot(void)
    {
        return writeModuleProperty(propertyId_E::COMMAND, commands_E::REBOOT);
    }

    PdsModule::error_E Pds::saveConfig(void)
    {
        return writeModuleProperty(propertyId_E::COMMAND, commands_E::SAVE_CONFIG);
    }

}  // namespace mab