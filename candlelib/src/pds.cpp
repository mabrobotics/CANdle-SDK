#include "pds.hpp"

#include <string>

enum moduleVersion_E : uint8_t
{
    UNKNOWN = 0x00,
    V0_1,  // 0.1
    V0_2,  // 0.2
    V0_3,  // 0.3
           /* NEW MODULE VERSIONS HERE */
};

namespace mab
{

    Pds::Pds(uint16_t canId, std::shared_ptr<Candle> sp_Candle)
        : msp_Candle(sp_Candle), m_canId(canId)
    {
        m_log.m_tag   = "PDS";
        m_log.m_layer = Logger::ProgramLayer_E::LAYER_2;
        if (readModules() != error_E::OK)
            m_log.error("Unable to read modules data from PDS...");
    }

    Pds::error_E Pds::readModules(void)
    {
        const char txBuffer[]   = {FRAME_GET_INFO, 0x00};
        char       rxBuffer[64] = {0};

        PdsModule::type_E modules[MAX_MODULES] = {PdsModule::type_E::UNDEFINED};

        if (msp_Candle->sendGenericFDCanFrame(m_canId, sizeof(txBuffer), txBuffer, rxBuffer, 100))
        {
            memcpy(&modules, rxBuffer, sizeof(modules));

            for (uint8_t i = 0; i < MAX_MODULES; i++)
            {
                PdsModule::socket_E socketIndex = static_cast<PdsModule::socket_E>(i);
                switch (modules[i])
                {
                    case PdsModule::type_E::BRAKE_RESISTOR:
                        m_brakeResistors.push_back(std::make_unique<BrakeResistor>(socketIndex));
                        break;

                    case PdsModule::type_E::ISOLATED_CONVERTER_12V:
                        m_IsolatedConv12s.push_back(std::make_unique<IsolatedConv12>(socketIndex));
                        break;

                    case PdsModule::type_E::ISOLATED_CONVERTER_5V:
                        m_IsolatedConv5s.push_back(std::make_unique<IsolatedConv5>(socketIndex));
                        break;

                    case PdsModule::type_E::POWER_STAGE_V1:
                        m_powerStageV1s.push_back(std::make_unique<PowerStageV1>(socketIndex));
                        break;

                    case PdsModule::type_E::POWER_STAGE_V2:
                        m_powerStageV2s.push_back(std::make_unique<PowerStageV2>(socketIndex));
                        break;

                    case PdsModule::type_E::UNDEFINED:
                    default:
                        break;
                }
            }
        }
        else
        {
            m_log.error("PDS not responding");
            return error_E::COMMUNICATION_ERROR;
        }

        return error_E::OK;
    }

    void Pds::getModules(modules_S& modules)
    {
        modules.brakeResistor       = m_brakeResistors.size();
        modules.isolatedConv12V     = m_IsolatedConv12s.size();
        modules.isolatedConverter5V = m_IsolatedConv5s.size();
        modules.powerStageV1        = m_powerStageV1s.size();
        modules.powerStageV2        = m_powerStageV2s.size();
    }

    std::unique_ptr<BrakeResistor> Pds::attachBrakeResistor(const PdsModule::socket_E socket)
    {
        if (!m_brakeResistors.empty())
        {
            for (auto& module : m_brakeResistors)
            {
                if (module->getSocket() == socket)
                    return std::move(module);
            }
            m_log.error("No brake resistor module connected to socket [ %u ]!",
                        static_cast<uint8_t>(socket) + 1);

            return nullptr;
        }

        m_log.error("No Brake Resistor modules connected to PDS device!");
        return nullptr;
    }

    std::unique_ptr<PowerStageV1> Pds::attachPowerStageV1(const PdsModule::socket_E socket)
    {
        if (!m_powerStageV1s.empty())
        {
            for (auto& module : m_powerStageV1s)
            {
                if (module->getSocket() == socket)
                    return std::move(module);
            }

            m_log.error("No power stage module connected to socket [ %u ]!",
                        static_cast<uint8_t>(socket) + 1);

            return nullptr;
        }

        m_log.error("No power stage modules connected to PDS device!");
        return nullptr;
    }

    std::unique_ptr<PowerStageV2> Pds::attachPowerStageV2(const PdsModule::socket_E socket)
    {
        if (!m_powerStageV2s.empty())
        {
            for (auto& module : m_powerStageV2s)
            {
                if (module->getSocket() == socket)
                    return std::move(module);
            }

            m_log.error("No power stage V2 module connected to socket [ %u ]!",
                        static_cast<uint8_t>(socket) + 1);

            return nullptr;
        }

        m_log.error("No power stage V2 modules connected to PDS device!");
        return nullptr;
    }

    std::unique_ptr<IsolatedConv12> Pds::attachIsolatedConverter12(const PdsModule::socket_E socket)
    {
        if (!m_IsolatedConv12s.empty())
        {
            for (auto& module : m_IsolatedConv12s)
            {
                if (module->getSocket() == socket)
                    return std::move(module);
            }

            m_log.error("No Isolated Converter 12V module connected to socket [ %u ]!",
                        static_cast<uint8_t>(socket) + 1);

            return nullptr;
        }

        m_log.error("No Isolated Converter 12V modules connected to PDS device!");
        return nullptr;
    }

    std::unique_ptr<IsolatedConv5> Pds::attachIsolatedConverter5(const PdsModule::socket_E socket)
    {
        if (!m_IsolatedConv5s.empty())
        {
            for (auto& module : m_IsolatedConv5s)
            {
                if (module->getSocket() == socket)
                    return std::move(module);
            }

            m_log.error("No Isolated Converter 5V module connected to socket [ %u ]!",
                        static_cast<uint8_t>(socket) + 1);

            return nullptr;
        }

        m_log.error("No Isolated Converter 5V modules connected to PDS device!");
        return nullptr;
    }

}  // namespace mab