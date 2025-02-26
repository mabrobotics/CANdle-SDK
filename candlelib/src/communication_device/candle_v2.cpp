#include "candle_v2.hpp"

#include <exception>
#include <MD.hpp>

namespace mab
{
    CandleV2::CandleV2(const CANdleBaudrate_E                           canBaudrate,
                       std::unique_ptr<mab::I_CommunicationInterface>&& bus)
        : m_canBaudrate(canBaudrate), m_bus(std::move(bus))
    {
    }

    CandleV2::Error_t CandleV2::init(std::shared_ptr<CandleV2>* thisSharedRef)
    {
        m_thisSharedReference = thisSharedRef;
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
            m_log.error("Failed to initialize communication with CANdle device");
            m_isInitialized = false;
        }
        return initStatus;
    }

    CandleV2::Error_t CandleV2::discoverDevices()
    {
        m_mdMap.clear();
        constexpr canId_t minValidId = 10;     // ids less than that are reserved for special uses
        constexpr canId_t maxValidId = 0x7FF;  // 11-bit value (standard can ID max)

        for (canId_t id = minValidId; id < maxValidId; id++)
        {
            m_log.debug("Trying to bind MD with id %d", id);
            auto md         = std::make_shared<MD>(id, *m_thisSharedReference);
            auto initStatus = md->init();
            if (initStatus != MD::Error_t::OK)
                continue;
            m_log.info("Discovered MD device with ID: %d", id);
            m_mdMap[id] = md;
        }
        if (m_mdMap.size() > 0)
            return Error_t::OK;
        m_log.warn("Have not found any MD devices on the CAN bus!");
        return Error_t::CAN_DEVICE_NOT_RESPONDING;
    }

    CandleV2::Error_t CandleV2::busTransfer(std::shared_ptr<std::vector<u8>> data,
                                            size_t                           responseLength,
                                            const u32                        timeoutMs)
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

        if (responseLength == 0)
        {
            I_CommunicationInterface::Error_t comError = m_bus->transfer(*data, timeoutMs);
            if (comError)
                return Error_t::UNKNOWN_ERROR;
        }
        else
        {
            std::pair<std::vector<u8>, I_CommunicationInterface::Error_t> result =
                m_bus->transfer(*data, timeoutMs, responseLength);

            *data = result.first;

            if (result.second)
                return Error_t::UNKNOWN_ERROR;
        }
        return Error_t::OK;
    }

    CandleV2::Error_t CandleV2::busTransfer(const std::vector<u8>&& data)
    {
        auto sharedData = std::make_shared<std::vector<u8>>(data);
        return busTransfer(sharedData);
    }

    const std::pair<std::vector<u8>, CandleV2::Error_t> CandleV2::transferCANFrame(
        const canId_t         canId,
        const std::vector<u8> dataToSend,
        const size_t          responseSize,
        const u32             timeoutMs)
    {
        Error_t communicationStatus = Error_t::OK;
        if (!m_isInitialized)
            return std::pair<std::vector<u8>, Error_t>(dataToSend, Error_t::UNINITIALIZED);
        if (communicationStatus != Error_t::OK)
            return std::pair<std::vector<u8>, Error_t>(dataToSend, communicationStatus);

        m_log.debug("SEND");
        frameDump(dataToSend);

        if (dataToSend.size() > 64)
        {
            m_log.error("CAN frame too long!");
            return std::pair<std::vector<u8>, Error_t>(dataToSend, Error_t::DATA_TOO_LONG);
        }

        auto buffer = std::make_shared<std::vector<u8>>(dataToSend);

        const auto candleCommandCANframe = sendCanFrameHeader(dataToSend.size(), u16(canId));

        buffer->insert(buffer->begin(), candleCommandCANframe.begin(), candleCommandCANframe.end());

        communicationStatus = busTransfer(buffer, responseSize + 2 /*response header size*/);

        // retransmission if wrong ID in the response, see Candle Commands v1 description in
        // documentation
        if (buffer->at(1) != 0x01)
        {
            buffer->clear();
            *buffer             = dataToSend;
            communicationStatus = busTransfer(buffer, responseSize + 2 /*response header size*/);
            if (buffer->at(1) != 0x01 || buffer->at(0) != GENERIC_CAN_FRAME)
            {
                m_log.debug("Retransmitting can message to ID: %d", canId);
                m_log.error("CAN frame did not reach target device with id: %d!", canId);
                return std::pair<std::vector<u8>, Error_t>(dataToSend,
                                                           Error_t::CAN_DEVICE_NOT_RESPONDING);
            }
        }

        if (buffer->size() > 3)
            buffer->erase(buffer->begin(), buffer->begin() + 2 /*response header size*/);

        auto response = *buffer;

        m_log.debug("Expected received len: %d", responseSize);
        m_log.debug("RECEIVE");
        frameDump(response);

        return std::pair<std::vector<u8>, Error_t>(response, communicationStatus);
    }

    // TODO: this must be changed to something less invasive
    CandleV2::Error_t CandleV2::legacyCheckConnection()
    {
        auto baudrateFrame = baudrateCommandFrame(m_canBaudrate);

        auto testConnectionFrame = std::make_shared<std::vector<u8>>(
            std::vector<u8>(baudrateFrame.begin(), baudrateFrame.end()));

        const Error_t connectionStatus = busTransfer(testConnectionFrame, 5);
        if (connectionStatus != Error_t::OK)
            return connectionStatus;
        return Error_t::OK;
    }
}  // namespace mab