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

        struct status_S
        {
            bool ENABLED;
            bool OVT_EVENT;  // Over-temperature event
            bool STO1_EVENT;
            bool STO2_EVENT;
            bool FDCAN_TIMEOUT_EVENT;
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

        modulesSet_S getModules(void);

        std::unique_ptr<BrakeResistor>  attachBrakeResistor(socketIndex_E socket);
        std::unique_ptr<PowerStage>     attachPowerStage(socketIndex_E socket);
        std::unique_ptr<IsolatedConv12> attachIsolatedConverter12(socketIndex_E socket);
        std::unique_ptr<IsolatedConv5>  attachIsolatedConverter5(socketIndex_E socket);

        error_E getStatus(status_S& status);
        error_E clearStatus(status_S status);

        error_E setCanId(u16 canId);
        error_E setCanBaudrate(canBaudrate_E canBaudrate);

        error_E getBusVoltage(u32& busVoltage);
        error_E getTemperature(f32& temperature);

        static const char* moduleTypeToString(moduleType_E type);

      private:
        /**
         * @brief Member reference to Candle object representing Candle device the PDS is
         * connected to over CANBus
         */
        Candle& m_candle;

        Logger   m_log;
        uint16_t m_canId = 0;

        modulesSet_S m_modulesSet = {moduleType_E::UNDEFINED};

        std::vector<std::unique_ptr<BrakeResistor>>  m_brakeResistors;
        std::vector<std::unique_ptr<PowerStage>>     m_powerStages;
        std::vector<std::unique_ptr<IsolatedConv12>> m_IsolatedConv12s;
        std::vector<std::unique_ptr<IsolatedConv5>>  m_IsolatedConv5s;

        error_E createModule(moduleType_E type, socketIndex_E socket);

        error_E readModules(void);

        static moduleType_E decodeModuleType(uint8_t moduleTypeCode);
    };

}  // namespace mab