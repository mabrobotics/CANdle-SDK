#pragma once

#include "pds_module.hpp"

namespace mab
{
    /**
     * @brief Power stage module class
     * @note The instances of the PDS modules are not intended to be created by user manually!!!
     *       The idea is that PDS base class detects connected modules automatically and
     *
     */
    class PowerStage : public PdsModule
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

            // TODO: Move
            STATUS            = 0x00,  // [ uint32_t ] Contains status bits
            STATUS_CLEAR      = 0x01,  // [ uint32_t ] Write only property used to clear status bits
            ENABLED           = 0x02,  // [ BOOL ] Indicates if the module is enabled or not
            TEMPERATURE       = 0x03,  // [ uint32_t ]
            TEMPERATURE_LIMIT = 0x04,  // [ *C/10 ]
            BUS_VOLTAGE       = 0x05,  // [ uint32_t ] ( mV )
            BR_SOCKET_INDEX   = 0x06,  // [ uint8_t ] Brake Resistor socket index
            BR_TRIGGER_VOLTAGE = 0x07,  // [ uint32_t ] Brake Resistor trigger voltage [ mV ]
            LOAD_CURRENT       = 0x08,
            LOAD_POWER         = 0x09,
            TOTAL_ENERGY       = 0x0A,
            OCD_LEVEL          = 0x0B,  // [ mA ]
            OCD_DELAY          = 0x0C,  // [ us ]

        };

        struct status_S
        {
            bool ENABLED;
            bool OCD_EVENT;  // Over-current detection event
            bool OVT_EVENT;  // Over-temperature event
        };

        PowerStage() = delete;
        PowerStage(socketIndex_E socket, Candle& candle, u16 canId);
        ~PowerStage();

        error_E enable();
        error_E disable();

        error_E getStatus(status_S& status);
        error_E clearStatus(status_S status);

        error_E getEnabled(bool& enabled);

        /**
         * @brief Set the socket index which the Brake resistor we want to bind is connected to.
         *
         * @param brakeResistorSocketIndex
         * @return error_E
         */
        error_E bindBrakeResistor(socketIndex_E brakeResistorSocketIndex);

        /**
         * @brief Set the Brake Resistor Trigger Voltage. When the bus voltage will exceed this
         * value, the binded brake resistor will trigger. If there is no Brake resistor binded this
         * method has no effect
         *
         * @param brTriggerVoltage Bus voltage in [ mV ]
         * @return error_E
         */
        error_E setBrakeResistorTriggerVoltage(u32 brTriggerVoltage);

        /**
         * @brief Get the Brake Resistor Trigger Voltage
         *
         * @param brTriggerVoltage
         * @return error_E
         */
        error_E getBrakeResistorTriggerVoltage(u32& brTriggerVoltage);

        /**
         * @brief Get the Output Voltage of Power Stage module
         *
         * @param outputVoltage
         * @return error_E
         */
        error_E getOutputVoltage(u32& outputVoltage);

        /**
         * @brief Get the Load Current of the Power Stage module
         *
         * @param loadCurrent
         * @return error_E
         */
        error_E getLoadCurrent(s32& loadCurrent);

        /**
         * @brief Get the momentary Power that goes through the Power Stage module
         * @note  Note that this parameter is calculated by the PDS device internally
         * so it may have been calculated from different current and voltage data then that
         * read by host SBC
         * @param power
         *
         * @return error_E
         */
        error_E getPower(s32& power);

        /**
         * @brief Get the total Energy that was delivered by the Power Stage module
         *
         * @param energy
         * @return error_E
         */
        error_E getEnergy(s32& energy);

        /**
         * @brief Get the Temperature of the module
         *
         * @param temperature
         * @return error_E
         */
        error_E getTemperature(f32& temperature);

        /**
         * @brief Set the Over-Current Detection Level of the Power stage module ( in mA )
         *
         * @param ocdLevel
         * @return error_E
         */
        error_E setOcdLevel(u32 ocdLevel);

        /**
         * @brief Get the Over-Current Detection Level of the Power stage module ( in mA )
         *
         * @param ocdLevel
         * @return error_E
         */
        error_E getOcdLevel(u32& ocdLevel);

        /**
         * @brief Set the Over-Current Detection Delay of the Power stage module ( in uS ).
         * If the measured current exceeds the limit ( set with setOcdLevel method )
         * it will switch to disabled state after this time. Note that the PDS Control loop
         * frequency is 5KHz so the effective value will be rounded to the multiple of 200uS.
         * @param ocdDelay
         * @return error_E
         */
        error_E setOcdDelay(u32 ocdDelay);

        /**
         * @brief Get the Over-Current Detection Delay of the Power stage module ( in uS ).
         * If the measured current exceeds the limit ( set with setOcdLevel method )
         *
         * @param ocdDelay
         * @return error_E
         */
        error_E getOcdDelay(u32& ocdDelay);

        error_E setTemperatureLimit(f32 temperatureLimit);
        error_E getTemperatureLimit(f32& temperatureLimit);
        // private:
    };
}  // namespace mab