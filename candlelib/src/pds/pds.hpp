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

        using modulesSet_t = std::array<moduleType_E, MAX_MODULES>;

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
            BUS_VOLTAGE       = 0x05,
            SOCKET_1_MODULE   = 0x06,  // SOCKET 1 Connected module type
            SOCKET_2_MODULE   = 0x07,  // SOCKET 2 Connected module type
            SOCKET_3_MODULE   = 0x08,  // SOCKET 3 Connected module type
            SOCKET_4_MODULE   = 0x09,  // SOCKET 4 Connected module type
            SOCKET_5_MODULE   = 0x0A,  // SOCKET 5 Connected module type
            SOCKET_6_MODULE   = 0x0B,  // SOCKET 6 Connected module type

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

        modulesSet_t getModules(void);

        std::unique_ptr<BrakeResistor>  attachBrakeResistor(socketIndex_E socket);
        std::unique_ptr<PowerStage>     attachPowerStage(socketIndex_E socket);
        std::unique_ptr<IsolatedConv12> attachIsolatedConverter12(socketIndex_E socket);
        std::unique_ptr<IsolatedConv5>  attachIsolatedConverter5(socketIndex_E socket);

        PdsModule::error_E getBusVoltage(u32& busVoltage);
        PdsModule::error_E getTemperature(f32& temperature);

        static const char* moduleTypeToString(moduleType_E type);

      private:
        /**
         * @brief Member reference to Candle object representing Candle device the PDS is
         * connected to over CANBus
         */
        Candle& m_candle;

        Logger   m_log;
        uint16_t m_canId = 0;

        modulesSet_t m_moduleTypes = {moduleType_E::UNDEFINED};

        std::vector<std::unique_ptr<BrakeResistor>>  m_brakeResistors;
        std::vector<std::unique_ptr<PowerStage>>     m_powerStages;
        std::vector<std::unique_ptr<IsolatedConv12>> m_IsolatedConv12s;
        std::vector<std::unique_ptr<IsolatedConv5>>  m_IsolatedConv5s;

        PdsModule::error_E readModules(void);

        static moduleType_E decodeModuleType(uint8_t moduleTypeCode);
    };

}  // namespace mab