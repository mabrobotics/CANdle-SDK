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
        m_log.tag   = "PDS";
        m_log.level = logger::LogLevel_E::DEBUG;
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
                switch (modules[i])
                {
                    case PdsModule::type_E::BRAKE_RESISTOR:
                        m_modules.brakeResistor++;
                        break;

                    case PdsModule::type_E::ISOLATED_CONVERTER_12V:
                        m_modules.isolatedConverter12V++;
                        break;

                    case PdsModule::type_E::ISOLATED_CONVERTER_5V:
                        m_modules.isolatedConverter5V++;
                        break;

                    case PdsModule::type_E::POWER_STAGE_V1:
                        m_modules.powerStageV1++;
                        break;

                    case PdsModule::type_E::POWER_STAGE_V2:
                        m_modules.powerStageV2++;
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
        memcpy(&modules, &m_modules, sizeof(modules_S));
    }

}  // namespace mab