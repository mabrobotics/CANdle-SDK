#include "candle_v2.hpp"

namespace mab
{
    CandleV2::CandleV2(CANdleBaudrate_E canBaudrate, std::unique_ptr<mab::Bus>&& bus)
        : m_canBaudrate(canBaudrate), m_bus(std::move(bus))
    {
    }
    CandleV2::CandleV2(CANdleBaudrate_E canBaudrate, mab::Bus&& bus)
    {
        auto busPtr = std::unique_ptr<mab::Bus>(&bus);
        CandleV2(canBaudrate, std::move(busPtr));
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

    std::pair<std::vector<u8>, I_CommunicationDevice::Error_t> CandleV2::transferData(
        std::vector<u8> dataToSend)
    {
        // TODO: placeholder
        const std::vector<u8> vec = {};
        const Error_t         err = Error_t::OK;
        return std::pair<std::vector<u8>, Error_t>(vec, err);
    }
}  // namespace mab