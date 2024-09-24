#pragma once

#include <stdint.h>

#include "candle.hpp"
#include "logger.hpp"

#include "pds_module.hpp"

namespace mab
{

    /**
     * @brief Power distribution system class
     *
     */
    class Pds
    {
      public:
        enum class error_E : int8_t
        {
            COMMUNICATION_ERROR = -2,
            UNKNOWN_ERROR       = -1,
            OK                  = 0,
        };

        /**
         * @brief This struct holds the amount of each connected module type
         */
        struct modules_S
        {
            uint8_t powerStageV1;
            uint8_t powerStageV2;
            uint8_t brakeResistor;
            uint8_t isolatedConv12V;
            uint8_t isolatedConverter5V;
            /* */
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
        Pds(uint16_t canId, std::shared_ptr<Candle> sp_Candle);

        /**
         * @brief Get the Modules structure
         *
         * @param modules reference to modules structure
         */
        void getModules(modules_S& modules);

        std::unique_ptr<BrakeResistor>  attachBrakeResistor(PdsModule::socket_E socket);
        std::unique_ptr<PowerStageV1>   attachPowerStageV1(PdsModule::socket_E socket);
        std::unique_ptr<PowerStageV2>   attachPowerStageV2(PdsModule::socket_E socket);
        std::unique_ptr<IsolatedConv12> attachIsolatedConverter12(PdsModule::socket_E socket);
        std::unique_ptr<IsolatedConv5>  attachIsolatedConverter5(PdsModule::socket_E socket);

      private:
        //   Maximum pds modules number
        static constexpr size_t MAX_MODULES = 6u;
        /**
         * @brief Member pointer to Candle object representing Candle device the PDS is
         * connected to over CANBus is connected over the CANBus
         *
         */
        std::shared_ptr<Candle> msp_Candle = nullptr;

        logger   m_log;
        uint16_t m_canId = 0;

        std::vector<std::unique_ptr<BrakeResistor>>  m_brakeResistors;
        std::vector<std::unique_ptr<PowerStageV1>>   m_powerStageV1s;
        std::vector<std::unique_ptr<PowerStageV2>>   m_powerStageV2s;
        std::vector<std::unique_ptr<IsolatedConv12>> m_IsolatedConv12s;
        std::vector<std::unique_ptr<IsolatedConv5>>  m_IsolatedConv5s;

        /**
         * @brief Read information about connected modules from physical PDS device
         * @return error_E
         */
        error_E readModules(void);
    };

}  // namespace mab