#pragma once

#include <stdint.h>

#include "candle.hpp"
#include "logger.hpp"

#include "pds_module.hpp"
#include "pds_types.hpp"

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
            uint8_t powerStage;
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

        std::unique_ptr<BrakeResistor>  attachBrakeResistor(socketIndex_E socket);
        std::unique_ptr<PowerStage>     attachPowerStage(socketIndex_E socket);
        std::unique_ptr<IsolatedConv12> attachIsolatedConverter12(socketIndex_E socket);
        std::unique_ptr<IsolatedConv5>  attachIsolatedConverter5(socketIndex_E socket);

      private:
        //   Maximum pds modules number
        static constexpr size_t MAX_MODULES = 6u;
        /**
         * @brief Member pointer to Candle object representing Candle device the PDS is
         * connected to over CANBus is connected over the CANBus
         *
         */
        std::shared_ptr<Candle> msp_Candle = nullptr;

        Logger   m_log;
        uint16_t m_canId = 0;

        std::vector<std::unique_ptr<BrakeResistor>>  m_brakeResistors;
        std::vector<std::unique_ptr<PowerStage>>     m_powerStages;
        std::vector<std::unique_ptr<IsolatedConv12>> m_IsolatedConv12s;
        std::vector<std::unique_ptr<IsolatedConv5>>  m_IsolatedConv5s;

        /**
         * @brief Read information about connected modules from physical PDS device
         * @return error_E
         */
        error_E readModules(void);
    };

}  // namespace mab