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

    PdsModule::error_E BrakeResistor::enable()
    {
        return writeModuleProperty(properties_E::ENABLED, true);
    }

    PdsModule::error_E BrakeResistor::disable()
    {
        return writeModuleProperty(properties_E::ENABLED, false);
    }

    PowerStage::PowerStage(socketIndex_E socket, std::shared_ptr<Candle> sp_Candle, u16 canId)
        : PdsModule(socket, moduleType_E::POWER_STAGE, sp_Candle, canId)
    {
        m_log.m_tag = "PS  :: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

    PowerStage::~PowerStage()
    {
        disable();
    }

    PdsModule::error_E PowerStage::enable()
    {
        return writeModuleProperty(properties_E::ENABLED, true);
    }

    PdsModule::error_E PowerStage::disable()
    {
        return writeModuleProperty(properties_E::ENABLED, false);
    }

    PdsModule::error_E PowerStage::getStatus(status_S& status)
    {
        return readModuleProperty(properties_E::STATUS, status);
    }

    PdsModule::error_E PowerStage::getEnabled(bool& enabled)
    {
        return readModuleProperty(properties_E::ENABLED, enabled);
    }

    PdsModule::error_E BrakeResistor::getEnabled(bool& enabled)
    {
        return readModuleProperty(properties_E::ENABLED, enabled);
    }

    PdsModule::error_E PowerStage::bindBrakeResistor(socketIndex_E brakeResistorSocketIndex)
    {
        return writeModuleProperty(properties_E::BR_SOCKET_INDEX, brakeResistorSocketIndex);
    }

    PdsModule::error_E PowerStage::setBrakeResistorTriggerVoltage(uint32_t brTriggerVoltage)
    {
        return writeModuleProperty(properties_E::BR_TRIGGER_VOLTAGE, brTriggerVoltage);
    }

    PdsModule::error_E PowerStage::getOutputVoltage(u32& outputVoltage)
    {
        return readModuleProperty(properties_E::BUS_VOLTAGE, outputVoltage);
    }

    PdsModule::error_E PowerStage::getLoadCurrent(s32& loadCurrent)
    {
        return readModuleProperty(properties_E::LOAD_CURRENT, loadCurrent);
    }

    PdsModule::error_E PowerStage::getPower(s32& power)
    {
        return readModuleProperty(properties_E::LOAD_POWER, power);
    }

    PdsModule::error_E PowerStage::getEnergy(s32& energy)
    {
        return readModuleProperty(properties_E::TOTAL_ENERGY, energy);
    }

    PdsModule::error_E PowerStage::getTemperature(f32& temperature)
    {
        return readModuleProperty(properties_E::TEMPERATURE, temperature);
    }

    PdsModule::error_E PowerStage::setOcdLevel(u32 ocdLevel)
    {
        return writeModuleProperty(properties_E::OCD_LEVEL, ocdLevel);
    }

    PdsModule::error_E PowerStage::getOcdLevel(u32& ocdLevel)
    {
        return readModuleProperty(properties_E::OCD_LEVEL, ocdLevel);
    }

    PdsModule::error_E PowerStage::setOcdDelay(u32 ocdDelay)
    {
        return writeModuleProperty(properties_E::OCD_DELAY, ocdDelay);
    }

    PdsModule::error_E PowerStage::getOcdDelay(u32& ocdDelay)
    {
        return readModuleProperty(properties_E::OCD_DELAY, ocdDelay);
    }

    PdsModule::error_E PowerStage::setTemperatureLimit(f32 temperatureLimit)
    {
        return writeModuleProperty(properties_E::TEMPERATURE_LIMIT, temperatureLimit);
    }

    PdsModule::error_E PowerStage::getTemperatureLimit(f32& temperatureLimit)
    {
        return readModuleProperty(properties_E::TEMPERATURE_LIMIT, temperatureLimit);
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
        return writeModuleProperty(properties_E::ENABLED, true);
    }

    PdsModule::error_E IsolatedConv12::disable()
    {
        return writeModuleProperty(properties_E::ENABLED, false);
    }

    IsolatedConv5::IsolatedConv5(socketIndex_E socket, std::shared_ptr<Candle> sp_Candle, u16 canId)
        : PdsModule(socket, moduleType_E::ISOLATED_CONVERTER_5V, sp_Candle, canId)
    {
        m_log.m_tag = "IC5 :: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

}  // namespace mab