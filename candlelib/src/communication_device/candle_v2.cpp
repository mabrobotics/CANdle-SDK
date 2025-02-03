#include "candle_v2.hpp"

namespace mab
{
    CandleV2::CandleV2(const CANdleBaudrate_E canBaudrate, std::unique_ptr<mab::Bus>&& bus)
        : m_canBaudrate(canBaudrate), m_bus(std::move(bus))
    {
    }

    CandleV2::Error_t CandleV2::init()
    {
        if (isInitialized)
            return CandleV2::Error_t::OK;

        return CandleV2::Error_t::OK;
    }

    CandleV2::Error_t CandleV2::legacyBusTransfer(std::vector<u8>* data, u32 timeout_ms)
    {
        if (data->size() == 0)
        {
            m_log.error("Data empty!");
            return CandleV2::Error_t::DATA_EMPTY;
        }
        if (data->size() > 63)
        {
            m_log.error("Data too long!");
            return CandleV2::Error_t::DATA_TOO_LONG;
        }

        // char* rx = m_bus->getRxBuffer(0);

        if (m_bus->transmit((char*)data->data(), data->size(), true, timeout_ms, data->size()))
            return CandleV2::Error_t::OK;

        m_log.error("Transmission failed!");
        return CandleV2::Error_t::UNKNOWN_ERROR;
    }

    const std::pair<std::vector<u8>, I_CommunicationDevice::Error_t> CandleV2::transferData(
        const std::vector<u8> dataToSend)
    {
        if (!isInitialized)
            init();
        // else
        //     checkConnection();
        // TODO: placeholder
        const Error_t err = Error_t::OK;
        return std::pair<std::vector<u8>, Error_t>(dataToSend, err);
    }
}  // namespace mab