#include "power_stage.hpp"

namespace mab
{

    PowerStage::PowerStage(socketIndex_E socket, Candle& candle, u16 canId)
        : PdsModule(socket, moduleType_E::POWER_STAGE, candle, canId)
    {
        m_log.m_tag = "PS  :: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

    PowerStage::~PowerStage()
    {
        // disable();
    }

    PdsModule::error_E PowerStage::enable()
    {
        return writeModuleProperty(propertyId_E::ENABLE, true);
    }

    PdsModule::error_E PowerStage::disable()
    {
        return writeModuleProperty(propertyId_E::ENABLE, false);
    }

    PdsModule::error_E PowerStage::getStatus(status_S& status)
    {
        error_E result     = error_E::UNKNOWN_ERROR;
        u32     statusWord = 0;

        result = readModuleProperty(propertyId_E::STATUS_WORD, statusWord);

        status.ENABLED   = (statusWord & static_cast<u32>(status_E::ENABLED));
        status.OCD_EVENT = (statusWord & static_cast<u32>(status_E::OVER_CURRENT_EVENT));
        status.OVT_EVENT = (statusWord & static_cast<u32>(status_E::OVER_TEMPERATURE_EVENT));

        return result;
    }

    PdsModule::error_E PowerStage::clearStatus(status_S status)
    {
        u32 statusClearWord = 0;

        if (status.ENABLED)
            statusClearWord |= static_cast<u32>(status_E::ENABLED);

        if (status.OCD_EVENT)
            statusClearWord |= static_cast<u32>(status_E::OVER_CURRENT_EVENT);

        if (status.OVT_EVENT)
            statusClearWord |= static_cast<u32>(status_E::OVER_TEMPERATURE_EVENT);

        return writeModuleProperty(propertyId_E::STATUS_CLEAR, statusClearWord);
    }

    PdsModule::error_E PowerStage::getEnabled(bool& enabled)
    {
        return readModuleProperty(propertyId_E::ENABLE, enabled);
    }

    PdsModule::error_E PowerStage::bindBrakeResistor(socketIndex_E brakeResistorSocketIndex)
    {
        return writeModuleProperty(propertyId_E::BR_SOCKET_INDEX, brakeResistorSocketIndex);
    }

    PdsModule::error_E PowerStage::setBrakeResistorTriggerVoltage(uint32_t brTriggerVoltage)
    {
        return writeModuleProperty(propertyId_E::BR_TRIGGER_VOLTAGE, brTriggerVoltage);
    }

    PdsModule::error_E PowerStage::getBrakeResistorTriggerVoltage(u32& brTriggerVoltage)
    {
        return readModuleProperty(propertyId_E::BR_TRIGGER_VOLTAGE, brTriggerVoltage);
    }

    PdsModule::error_E PowerStage::getOutputVoltage(u32& outputVoltage)
    {
        return readModuleProperty(propertyId_E::BUS_VOLTAGE, outputVoltage);
    }

    PdsModule::error_E PowerStage::getLoadCurrent(s32& loadCurrent)
    {
        return readModuleProperty(propertyId_E::LOAD_CURRENT, loadCurrent);
    }

    PdsModule::error_E PowerStage::getPower(s32& power)
    {
        return readModuleProperty(propertyId_E::LOAD_POWER, power);
    }

    PdsModule::error_E PowerStage::getEnergy(s32& energy)
    {
        return readModuleProperty(propertyId_E::TOTAL_ENERGY, energy);
    }

    PdsModule::error_E PowerStage::getTemperature(f32& temperature)
    {
        return readModuleProperty(propertyId_E::TEMPERATURE, temperature);
    }

    PdsModule::error_E PowerStage::setOcdLevel(u32 ocdLevel)
    {
        return writeModuleProperty(propertyId_E::OCD_LEVEL, ocdLevel);
    }

    PdsModule::error_E PowerStage::getOcdLevel(u32& ocdLevel)
    {
        return readModuleProperty(propertyId_E::OCD_LEVEL, ocdLevel);
    }

    PdsModule::error_E PowerStage::setOcdDelay(u32 ocdDelay)
    {
        return writeModuleProperty(propertyId_E::OCD_DELAY, ocdDelay);
    }

    PdsModule::error_E PowerStage::getOcdDelay(u32& ocdDelay)
    {
        return readModuleProperty(propertyId_E::OCD_DELAY, ocdDelay);
    }

    PdsModule::error_E PowerStage::setTemperatureLimit(f32 temperatureLimit)
    {
        return writeModuleProperty(propertyId_E::TEMPERATURE_LIMIT, temperatureLimit);
    }

    PdsModule::error_E PowerStage::getTemperatureLimit(f32& temperatureLimit)
    {
        return readModuleProperty(propertyId_E::TEMPERATURE_LIMIT, temperatureLimit);
    }

}  // namespace mab