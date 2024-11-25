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
        /*
          Properties indexes used internally for creating protocol messages
          for this particular module type. Note that the properties may differ
          from type to type so they all provide own enumerator definition even if they share
          exact same set of properties.
        */
        enum class properties_E : uint8_t
        {

            STATUS            = 0x00,
            STATUS_CLEAR      = 0x01,
            ENABLED           = 0x02,  // [ BOOL ] Indicates if the module is enabled or not
            TEMPERATURE       = 0x03,  // [ uint32_t ]
            TEMPERATURE_LIMIT = 0x04,  // [ uint32_t ]
            LOAD_CURRENT      = 0x05,
            LOAD_POWER        = 0x06,
            TOTAL_ENERGY      = 0x07,

        };

        struct status_S
        {
            bool ENABLED;
            bool OCD_EVENT;  // Over-current detection event
            bool OVT_EVENT;  // Over-temperature event
        };

        BrakeResistor() = delete;
        BrakeResistor(socketIndex_E socket, Candle& candle, u16 canId);
        ~BrakeResistor() = default;

        error_E enable();
        error_E disable();

        error_E getEnabled(bool& enabled);

        error_E getStatus(status_S& status);
        error_E clearStatus(status_S status);

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