#include "candle_v2.hpp"

#include <exception>

namespace mab
{
    CandleV2::CandleV2(const CANdleBaudrate_E                           canBaudrate,
                       std::unique_ptr<mab::I_CommunicationInterface>&& bus)
        : m_canBaudrate(canBaudrate), m_bus(std::move(bus))
    {
    }

    CandleV2::Error_t CandleV2::init()
    {
        m_bus->disconnect();
        I_CommunicationInterface::Error_t connectStatus = m_bus->connect();
        if (connectStatus != I_CommunicationInterface::Error_t::OK)
        {
            m_isInitialized = false;
            return Error_t::INITIALIZATION_ERROR;
        }

        Error_t initStatus = legacyCheckConnection();
        if (initStatus == OK)
        {
            m_isInitialized = true;
        }
        else
        {
            m_log.error("Failed to initialize communication with CANdle device of id: %d",
                        0 /*TODO: placeholder for serial id*/);
            m_isInitialized = false;
        }
        return initStatus;
    }

    CandleV2::Error_t CandleV2::legacyBusTransfer(std::shared_ptr<std::vector<u8>> data,
                                                  size_t                           responseLength)
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

        // temporary buffer operations

        // char* rx = m_bus->getRxBuffer(0);

        if (responseLength > 0)
        {
            I_CommunicationInterface::Error_t comError =
                m_bus->transfer(*data, DEFAULT_CAN_TIMEOUT);
            if (comError)
                return Error_t::UNKNOWN_ERROR;
        }
        else
        {
            std::pair<std::vector<u8>, I_CommunicationInterface::Error_t> result =
                m_bus->transfer(*data, DEFAULT_CAN_TIMEOUT, responseLength);

            *data = result.first;

            if (result.second)
                return Error_t::UNKNOWN_ERROR;
        }
        return Error_t::OK;
    }

    CandleV2::Error_t CandleV2::legacyBusTransfer(const std::vector<u8>&& data)
    {
        auto sharedData = std::make_shared<std::vector<u8>>(data);
        return legacyBusTransfer(sharedData);
    }

    const std::pair<std::vector<u8>, I_CommunicationDevice::Error_t> CandleV2::transferCANFrame(
        const std::vector<u8> dataToSend, const size_t responseSize)
    {
        Error_t communicationStatus = Error_t::OK;
        if (!m_isInitialized)
            return std::pair<std::vector<u8>, Error_t>(dataToSend, Error_t::UNINITIALIZED);
        if (communicationStatus != Error_t::OK)
            return std::pair<std::vector<u8>, Error_t>(dataToSend, communicationStatus);

        if (dataToSend.size() > 64)
        {
            m_log.error("CAN frame too long!");
            return std::pair<std::vector<u8>, Error_t>(dataToSend, Error_t::DATA_TOO_LONG);
        }

        auto buffer = std::make_shared<std::vector<u8>>(dataToSend);

        const auto candleCommandCANframe = sendCanFrameHeader(dataToSend.size());

        buffer->insert(buffer->begin(), candleCommandCANframe.begin(), candleCommandCANframe.end());

        communicationStatus = legacyBusTransfer(buffer, responseSize + 3);

        if (buffer->size() > 3)
            buffer->erase(buffer->begin(), buffer->begin() + 2 /*response header size*/);

        auto response = *buffer;

        return std::pair<std::vector<u8>, Error_t>(response, communicationStatus);
    }

    // TODO: this must be changed to something less invasive
    CandleV2::Error_t CandleV2::legacyCheckConnection()
    {
        auto baudrateFrame = baudrateCommandFrame(m_canBaudrate);

        auto testConnectionFrame = std::make_shared<std::vector<u8>>(
            std::vector<u8>(baudrateFrame.begin(), baudrateFrame.end()));

        const Error_t connectionStatus = legacyBusTransfer(testConnectionFrame);
        if (connectionStatus != Error_t::OK)
            return connectionStatus;
        return Error_t::OK;
    }
}  // namespace mab