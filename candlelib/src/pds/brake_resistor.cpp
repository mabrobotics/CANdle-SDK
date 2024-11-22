#include "brake_resistor.hpp"

namespace mab
{
    BrakeResistor::BrakeResistor(socketIndex_E socket, Candle& candle, u16 canId)
        : PdsModule(socket, moduleType_E::BRAKE_RESISTOR, candle, canId)
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

    PdsModule::error_E BrakeResistor::getEnabled(bool& enabled)
    {
        return readModuleProperty(properties_E::ENABLED, enabled);
    }

    PdsModule::error_E BrakeResistor::getStatus(status_S& status)
    {
        error_E result     = error_E::UNKNOWN_ERROR;
        u32     statusWord = 0;

        result = readModuleProperty(properties_E::STATUS, statusWord);

        status.ENABLED   = (statusWord & static_cast<u32>(status_E::ENABLED));
        status.OVT_EVENT = (statusWord & static_cast<u32>(status_E::OVER_TEMPERATURE_EVENT));

        return result;
    }

    PdsModule::error_E BrakeResistor::clearStatus(status_S status)
    {
        u32 statusClearWord = 0;

        if (status.ENABLED)
            statusClearWord |= static_cast<u32>(status_E::ENABLED);

        if (status.OVT_EVENT)
            statusClearWord |= static_cast<u32>(status_E::OVER_TEMPERATURE_EVENT);

        return writeModuleProperty(properties_E::STATUS_CLEAR, statusClearWord);
    }

    PdsModule::error_E BrakeResistor::getTemperature(f32& temperature)
    {
        return readModuleProperty(properties_E::TEMPERATURE, temperature);
    }

    PdsModule::error_E BrakeResistor::setTemperatureLimit(f32 temperatureLimit)
    {
        return writeModuleProperty(properties_E::TEMPERATURE_LIMIT, temperatureLimit);
    }

    PdsModule::error_E BrakeResistor::getTemperatureLimit(f32& temperatureLimit)
    {
        return readModuleProperty(properties_E::TEMPERATURE_LIMIT, temperatureLimit);
    }

}  // namespace mab