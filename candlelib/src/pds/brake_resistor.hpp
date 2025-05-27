#pragma once

#include "pds_module.hpp"

namespace mab
{
    /**
     * @brief Brake resistor module class
     * @note The instances of the PDS modules are not intended to be created by user manually!!!
     *       The idea is that PDS base class detects connected modules automatically and
     */
    class BrakeResistor : public PdsModule
    {
      public:
        BrakeResistor() = delete;
        BrakeResistor(socketIndex_E socket, Candle* p_candle, u16& canId);
        ~BrakeResistor() = default;

        virtual void printModuleInfo(void) override;

        error_E enable();
        error_E disable();

        error_E getStatus(brakeResistorStatus_S& status);

        error_E clearStatus(brakeResistorStatus_S status);

        error_E getEnabled(bool& enabled);

        /**
         * @brief Get the Temperature of the module
         *
         * @param temperature
         * @return error_E
         */
        error_E getTemperature(f32& temperature);

        /**
         * @brief Set the Temperature Limit
         *
         * @param temperatureLimit
         * @return error_E
         */
        error_E setTemperatureLimit(f32 temperatureLimit);

        /**
         * @brief Get the Temperature Limit
         *
         * @param temperatureLimit
         * @return error_E
         */
        error_E getTemperatureLimit(f32& temperatureLimit);
    };

}  // namespace mab