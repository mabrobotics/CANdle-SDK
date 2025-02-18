#pragma once

#include <stdint.h>

#include "candle.hpp"
#include "logger.hpp"

#include "pds_module.hpp"
#include "power_stage.hpp"
#include "brake_resistor.hpp"
#include "isolated_converter.hpp"

#include "pds_types.hpp"

namespace mab
{

    /**
     * @brief Power distribution system class
     *
     */
    class Pds : public PdsModule
    {
      public:
        //   Maximum pds modules number
        static constexpr size_t MAX_MODULES = 6u;

        struct modulesSet_S
        {
            moduleType_E moduleTypeSocket1;
            moduleType_E moduleTypeSocket2;
            moduleType_E moduleTypeSocket3;
            moduleType_E moduleTypeSocket4;
            moduleType_E moduleTypeSocket5;
            moduleType_E moduleTypeSocket6;
        };

        Pds() = delete;

        /**
         * @brief Construct a new Pds object
         *
         * @param sp_Candle shared pointer to the Candle Object
         * @param canId CANBus node ID of the PDS instance being created
         * @note Note that default constructor is deleted so PDS Class is forced to take Candle
         * dependency during creation
         */
        Pds(uint16_t canId, Candle& candle);

        void init(void);

        modulesSet_S getModules(void);

        std::shared_ptr<BrakeResistor> attachBrakeResistor(socketIndex_E socket);
        std::shared_ptr<PowerStage>    attachPowerStage(socketIndex_E socket);
        std::shared_ptr<IsolatedConv>  attachIsolatedConverter(socketIndex_E socket);

        error_E getFwVersion(version_ut& version);

        error_E getStatus(controlBoardStatus_S& status);
        error_E clearStatus(controlBoardStatus_S status);

        u16     getCanId();
        error_E setCanId(u16 canId);
        error_E setCanBaudrate(canBaudrate_E canBaudrate);

        error_E getBusVoltage(u32& busVoltage);
        error_E getTemperature(f32& temperature);

        error_E getTemperatureLimit(f32& temperatureLimit);
        error_E setTemperatureLimit(f32 temperatureLimit);

        error_E getShutdownTime(u32& shutdownTime);
        error_E setShutdownTime(u32 shutdownTime);

        error_E getBatteryVoltageLevels(u32& batteryLvl1, u32& batteryLvl2);
        error_E setBatteryVoltageLevels(u32 batteryLvl1, u32 batteryLvl2);

        error_E shutdown(void);
        error_E saveConfig(void);

        static const char* moduleTypeToString(moduleType_E type);

      private:
        /**
         * @brief Member reference to Candle object representing Candle device the PDS is
         * connected to over CANBus
         */
        Candle& m_candle;

        Logger   m_log;
        uint16_t m_rootCanId = 0;

        modulesSet_S m_modulesSet = {moduleType_E::UNDEFINED};

        std::vector<std::shared_ptr<BrakeResistor>> m_brakeResistors;
        std::vector<std::shared_ptr<PowerStage>>    m_powerStages;
        std::vector<std::shared_ptr<IsolatedConv>>  m_IsolatedConvs;

        error_E createModule(moduleType_E type, socketIndex_E socket);

        error_E readModules(void);

        static moduleType_E decodeModuleType(uint8_t moduleTypeCode);
    };

}  // namespace mab