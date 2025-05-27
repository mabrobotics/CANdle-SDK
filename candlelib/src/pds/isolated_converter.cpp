#include "isolated_converter.hpp"
#include <string>

namespace mab
{

    IsolatedConv::IsolatedConv(socketIndex_E socket, Candle* p_candle, u16& canId)
        : PdsModule(socket, moduleType_E::ISOLATED_CONVERTER, p_candle, canId)
    {
        m_log.m_tag = "IC12:: " + std::to_string(static_cast<u8>(socket));
        m_log.debug("Object created");
    }

    void IsolatedConv::printModuleInfo(void)
    {
        isolatedConverterStatus_S status;
        moduleVersion_E           hwVersion;
        float                     temperature      = 0.0f;
        float                     temperatureLimit = 0.0f;
        s32                       current          = 0;
        u32                       voltage          = 0;
        u32                       ocdLevel         = 0;
        u32                       ocdDelay         = 0;

        getBoardVersion(hwVersion);
        getStatus(status);
        getTemperature(temperature);
        getTemperatureLimit(temperatureLimit);
        getLoadCurrent(current);
        getOutputVoltage(voltage);
        getOcdLevel(ocdLevel);
        getOcdDelay(ocdDelay);

        m_log.info("Module type: %s", mType2Str(m_type));
        m_log.info("Module version: %u", (u8)hwVersion);
        m_log.info("Module status: %s", status.ENABLED ? "ENABLED   " : "DISABLED");
        m_log.info("Module temperature: %.2f", temperature);
        m_log.info("Module temperature limit: %.2f", temperatureLimit);
        m_log.info("Module load current: %d", current);
        m_log.info("Module output voltage: %u", voltage);
        m_log.info("Module OCD level: %u", ocdLevel);
        m_log.info("Module OCD delay: %u", ocdDelay);
    }

    PdsModule::error_E IsolatedConv::enable()
    {
        return writeModuleProperty(propertyId_E::ENABLE, true);
    }

    PdsModule::error_E IsolatedConv::disable()
    {
        return writeModuleProperty(propertyId_E::ENABLE, false);
    }

    PdsModule::error_E IsolatedConv::getStatus(isolatedConverterStatus_S& status)
    {
        u32                statusWord = 0;
        PdsModule::error_E result     = readModuleProperty(propertyId_E::STATUS_WORD, statusWord);

        if (result != PdsModule::error_E::OK)
            return result;

        status.ENABLED          = statusWord & (u32)statusBits_E::ENABLED;
        status.OVER_TEMPERATURE = statusWord & (u32)statusBits_E::OVER_TEMPERATURE;
        status.OVER_CURRENT     = statusWord & (u32)statusBits_E::OVER_CURRENT;

        return result;
    }

    PdsModule::error_E IsolatedConv::clearStatus(isolatedConverterStatus_S status)
    {
        u32 statusWord = 0;

        // if (status.ENABLED)
        //     statusWord |= (u32)statusBits_E::ENABLED;

        if (status.OVER_TEMPERATURE)
            statusWord |= (u32)statusBits_E::OVER_TEMPERATURE;

        if (status.OVER_CURRENT)
            statusWord |= (u32)statusBits_E::OVER_CURRENT;

        return writeModuleProperty(propertyId_E::STATUS_CLEAR, statusWord);
    }

    PdsModule::error_E IsolatedConv::getEnabled(bool& enabled)
    {
        return readModuleProperty(propertyId_E::ENABLE, enabled);
    }

    PdsModule::error_E IsolatedConv::getOutputVoltage(u32& outputVoltage)
    {
        return readModuleProperty(propertyId_E::BUS_VOLTAGE, outputVoltage);
    }

    PdsModule::error_E IsolatedConv::getLoadCurrent(s32& loadCurrent)
    {
        return readModuleProperty(propertyId_E::LOAD_CURRENT, loadCurrent);
    }

    PdsModule::error_E IsolatedConv::getPower(s32& power)
    {
        return readModuleProperty(propertyId_E::LOAD_POWER, power);
    }

    PdsModule::error_E IsolatedConv::getEnergy(s32& energy)
    {
        return readModuleProperty(propertyId_E::TOTAL_DELIVERED_ENERGY, energy);
    }

    PdsModule::error_E IsolatedConv::getTemperature(f32& temperature)
    {
        return readModuleProperty(propertyId_E::TEMPERATURE, temperature);
    }

    PdsModule::error_E IsolatedConv::setOcdLevel(u32 ocdLevel)
    {
        return writeModuleProperty(propertyId_E::OCD_LEVEL, ocdLevel);
    }

    PdsModule::error_E IsolatedConv::getOcdLevel(u32& ocdLevel)
    {
        return readModuleProperty(propertyId_E::OCD_LEVEL, ocdLevel);
    }

    PdsModule::error_E IsolatedConv::setOcdDelay(u32 ocdDelay)
    {
        return writeModuleProperty(propertyId_E::OCD_DELAY, ocdDelay);
    }

    PdsModule::error_E IsolatedConv::getOcdDelay(u32& ocdDelay)
    {
        return readModuleProperty(propertyId_E::OCD_DELAY, ocdDelay);
    }

    PdsModule::error_E IsolatedConv::setTemperatureLimit(f32 temperatureLimit)
    {
        return writeModuleProperty(propertyId_E::TEMPERATURE_LIMIT, temperatureLimit);
    }

    PdsModule::error_E IsolatedConv::getTemperatureLimit(f32& temperatureLimit)
    {
        return readModuleProperty(propertyId_E::TEMPERATURE_LIMIT, temperatureLimit);
    }

}  // namespace mab
