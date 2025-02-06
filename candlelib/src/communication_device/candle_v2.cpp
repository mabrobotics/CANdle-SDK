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
            m_log.error("Failed to initialize communication with Candle device of id: %d",
                        m_bus->getId());
            m_isInitialized = false;
        }
        return initStatus;
    }

    CandleV2::Error_t CandleV2::reinit()
    {
        m_log.warn("Reinitialization for candle triggered");
        auto resetFrame = resetCommandFrame();
        legacyBusTransfer(std::vector<u8>(resetFrame.begin(), resetFrame.end()));
        sleep(0.5);
        m_isInitialized = false;
        return init();
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
        if (data->size() > 63)
        {
            m_log.error("Data too long!");
            return CandleV2::Error_t::DATA_TOO_LONG;
        }

        // temporary buffer operations

        char* rx = m_bus->getRxBuffer(0);

        if (m_bus->transmit(
                (char*)data->data(),
                data->size(),
                responseLength > 0 ? true : false,
                CandleV2::DEFAULT_CONFIGURATION_TIMEOUT,
                (responseLength < 66 ? 66 : responseLength
                 /*TODO: if len is less than 66 USB does not respond, find out why and fix it*/)))
        {
            if (responseLength > 0)
            {
                data->clear();
                int actualResponseLen = m_bus->getBytesReceived();
                data->reserve(actualResponseLen);
                data->insert(data->end(), rx, rx + responseLength);
            }
            return CandleV2::Error_t::OK;
        }

        m_isInitialized = false;
        m_log.error("Transmission failed!");
        return CandleV2::Error_t::UNKNOWN_ERROR;
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
            communicationStatus = init();

        if (communicationStatus != Error_t::OK)
            return std::pair<std::vector<u8>, Error_t>(dataToSend, communicationStatus);

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