#include "brake_resistor.hpp"

namespace mab
{
    BrakeResistor::BrakeResistor(socketIndex_E socket, Candle& candle, u16& canId)
        : PdsModule(socket, moduleType_E::BRAKE_RESISTOR, candle, canId)
    {
        m_log.m_tag = "BR  :: " + std::to_string(static_cast<int>(socket) + 1);
        m_log.debug("Object created");
    }

    void BrakeResistor::printModuleInfo(void)
    {
        brakeResistorStatus_S status           = {};
        moduleVersion_E       hwVersion        = moduleVersion_E::UNKNOWN;
        float                 temperature      = 0.0f;
        float                 temperatureLimit = 0.0f;

        getBoardVersion(hwVersion);
        getStatus(status);
        getTemperature(temperature);
        getTemperatureLimit(temperatureLimit);

        m_log.info("Module type: %s", moduleType2String(m_type).c_str());
        m_log.info("Module version: %u", (u8)hwVersion);
        m_log.info("Module status: %s", status.ENABLED ? "ENABLED" : "DISABLED");
        m_log.info("Module temperature: %.2f", temperature);
        m_log.info("Module temperature limit: %.2f", temperatureLimit);
    }

    PdsModule::error_E BrakeResistor::enable()
    {
        return writeModuleProperty(propertyId_E::ENABLE, true);
    }

    PdsModule::error_E BrakeResistor::disable()
    {
        return writeModuleProperty(propertyId_E::ENABLE, false);
    }

    PdsModule::error_E BrakeResistor::getStatus(brakeResistorStatus_S& status)
    {
        u32                statusWord = 0;
        PdsModule::error_E result     = readModuleProperty(propertyId_E::STATUS_WORD, statusWord);

        if (result != PdsModule::error_E::OK)
            return result;

        status.ENABLED          = statusWord & (u32)statusBits_E::ENABLED;
        status.OVER_TEMPERATURE = statusWord & (u32)statusBits_E::OVER_TEMPERATURE;

        return result;
    }

    PdsModule::error_E BrakeResistor::getEnabled(bool& enabled)
    {
        return readModuleProperty(propertyId_E::ENABLE, enabled);
    }

    PdsModule::error_E BrakeResistor::getTemperature(f32& temperature)
    {
        return readModuleProperty(propertyId_E::TEMPERATURE, temperature);
    }

    PdsModule::error_E BrakeResistor::setTemperatureLimit(f32 temperatureLimit)
    {
        return writeModuleProperty(propertyId_E::TEMPERATURE_LIMIT, temperatureLimit);
    }

    PdsModule::error_E BrakeResistor::getTemperatureLimit(f32& temperatureLimit)
    {
        return readModuleProperty(propertyId_E::TEMPERATURE_LIMIT, temperatureLimit);
    }

}  // namespace mab