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
        PowerStage() = delete;
        PowerStage(socketIndex_E socket, Candle* p_candle, u16& canId);
        ~PowerStage();

        void printModuleInfo(void) override;

        error_E enable();
        error_E disable();

        error_E getStatus(powerStageStatus_S& status);

        error_E clearStatus(powerStageStatus_S status);

        /**
         * @brief Check if the module is enabled or not
         *
         * @param enabled the flag that will be updated
         * @return error_E
         */
        error_E getEnabled(bool& enabled);

        /**
         * @brief Set the socket index which the Brake resistor we want to bind is connected to.
         *
         * @param brakeResistorSocketIndex
         * @return error_E
         */
        error_E bindBrakeResistor(socketIndex_E brakeResistorSocketIndex);

        /**
         * @brief Reads the socket of binded BR. ( Undefined socket if not binded )
         *
         * @param brakeResistorSocketIndex
         * @return error_E
         */
        error_E getBindBrakeResistor(socketIndex_E& brakeResistorSocketIndex);

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
         * @brief Enable or disable the Power Stage autostart feature.
         * @note  If the autostart is enabled, the Power Stage module will be enabled
         * by default when device is powered on. Notice that setting this feature in
         * runtime will not take effect until the user will explicitly save the configuration
         * to the device. Otherwise the autostart will be set to default value ( disabled )
         * @param autoStart
         * @return error_E
         */
        error_E setAutostart(bool autoStart);

        /**
         * @brief Check if the Power Stage module is set to autostart or not.
         *
         * @param autoStart
         * @return error_E
         */
        error_E getAutostart(bool& autoStart);

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
    };
}  // namespace mab