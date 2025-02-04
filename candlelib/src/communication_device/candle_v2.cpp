#include "candle_v2.hpp"

#include <exception>

namespace mab
{
    CandleV2::CandleV2(const CANdleBaudrate_E canBaudrate, std::unique_ptr<mab::Bus>&& bus)
        : m_canBaudrate(canBaudrate), m_bus(std::move(bus))
    {
        if (init() != CandleV2::Error_t::OK)
        {
            m_log.error("Could not connect to candle!");
            throw std::runtime_error("Could not connect to candle!");
        }
    }

    CandleV2::Error_t CandleV2::init()
    {
        if (m_isInitialized)
            return CandleV2::Error_t::OK;
        Error_t initStatus = legacyCheckConnection();
        if (initStatus == OK)
        {
            m_isInitialized = true;
        }
        else
        {
            m_isInitialized = false;
        }
        return initStatus;
    }

    CandleV2::Error_t CandleV2::reinit()
    {
        m_isInitialized = false;
        return init();
    }

    CandleV2::Error_t CandleV2::legacyBusTransfer(std::shared_ptr<std::vector<u8>> data)
    {
        if (data == nullptr)
        {
            m_log.error("Data vector broken!");
            return CandleV2::Error_t::DATA_EMPTY;
        }
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

        // temporary buffer operations

        // char* rx = m_bus->getRxBuffer(0);

        if (m_bus->transmit((char*)data->data(),
                            data->size(),
                            true,
                            CandleV2::DEFAULT_CONFIGURATION_TIMEOUT,
                            data->size()))
            return CandleV2::Error_t::OK;

        m_log.error("Transmission failed!");
        return CandleV2::Error_t::UNKNOWN_ERROR;
    }

    const std::pair<std::vector<u8>, I_CommunicationDevice::Error_t> CandleV2::transferData(
        const std::vector<u8> dataToSend)
    {
        Error_t communicationStatus = Error_t::OK;
        if (!m_isInitialized)
            communicationStatus = init();

        if (communicationStatus != Error_t::OK)
            return std::pair<std::vector<u8>, Error_t>(dataToSend, communicationStatus);

        auto buffer = std::make_shared<std::vector<u8>>(dataToSend);

        buffer->insert(
            buffer->begin(), CAN_FRAME_CANDLE_COMMAND.begin(), CAN_FRAME_CANDLE_COMMAND.end());

        communicationStatus = legacyBusTransfer(buffer);

        return std::pair<std::vector<u8>, Error_t>(dataToSend, communicationStatus);
    }

    CandleV2::Error_t CandleV2::legacyCheckConnection()
    {
        auto testConnectionFrame = std::make_shared<std::vector<u8>>();
        testConnectionFrame->reserve(2);
        testConnectionFrame->push_back(CandleCommands_t::CANDLE_CONFIG_BAUDRATE);
        testConnectionFrame->push_back(CANdleBaudrate_E::CAN_BAUD_1M);

        const Error_t connectionStatus = legacyBusTransfer(testConnectionFrame);
        if (connectionStatus != Error_t::OK)
            return connectionStatus;
        return Error_t::OK;
    }
}  // namespace mab