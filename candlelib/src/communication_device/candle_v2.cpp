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

    candleTypes::Error_t CandleV2::init(std::weak_ptr<CandleV2> thisSharedRef)
    {
        m_thisSharedReference = thisSharedRef;
        // m_bus->disconnect();
        // I_CommunicationInterface::Error_t connectStatus = m_bus->connect();
        // if (connectStatus != I_CommunicationInterface::Error_t::OK)
        // {
        //     m_isInitialized = false;
        //     return candleTypes::Error_t::INITIALIZATION_ERROR;
        // }

        candleTypes::Error_t initStatus = legacyCheckConnection();
        if (initStatus == candleTypes::Error_t::OK)
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

    candleTypes::Error_t CandleV2::discoverDevices()
    {
        using namespace std::placeholders;
        m_mdMap.clear();

        // TODO: change those to real values
        constexpr canId_t minValidId = 97;   // ids less than that are reserved for special uses
        constexpr canId_t maxValidId = 102;  // 11-bit value (standard can ID max)
        // constexpr canId_t minValidId = 10;     // ids less than that are reserved for special
        // uses constexpr canId_t maxValidId = 0x7FF;  // 11-bit value (standard can ID max)

        for (canId_t id = minValidId; id < maxValidId; id++)
        {
            m_log.debug("Trying to bind MD with id %d", id);
            std::function<canTransmitFrame_t> transmitCanFrameMethod =
                std::bind(CandleV2::transferCANFrame, m_thisSharedReference.lock(), _1, _2, _3, _4);

            auto md         = std::make_shared<MD>(id, transmitCanFrameMethod);
            auto initStatus = md->init();
            if (initStatus != MD::Error_t::OK)
                continue;
            m_log.info("Discovered MD device with ID: %d", id);
            m_mdMap[id] = md;
        }
        if (m_mdMap.size() > 0)
            return candleTypes::Error_t::OK;
        m_log.warn("Have not found any MD devices on the CAN bus!");
        // TODO: add pds discovery
        return candleTypes::Error_t::CAN_DEVICE_NOT_RESPONDING;
    }

    std::map<canId_t, std::shared_ptr<MD>> CandleV2::getMDMap()
    {
        return m_mdMap;
    }

    candleTypes::Error_t CandleV2::busTransfer(std::vector<u8>* data,
                                               size_t           responseLength,
                                               const u32        timeoutMs)
    {
        if (data == nullptr)
        {
            m_log.error("Data vector broken!");
            return candleTypes::Error_t::DATA_EMPTY;
        }
        if (data->size() == 0)
        {
            m_log.error("Data empty!");
            return candleTypes::Error_t::DATA_EMPTY;
        }

        if (responseLength == 0)
        {
            I_CommunicationInterface::Error_t comError = m_bus->transfer(*data, timeoutMs);
            if (comError)
                return candleTypes::Error_t::UNKNOWN_ERROR;
        }
        else
        {
            std::pair<std::vector<u8>, I_CommunicationInterface::Error_t> result =
                m_bus->transfer(*data, timeoutMs, responseLength);

            data->clear();
            data->insert(data->begin(), result.first.begin(), result.first.end());
            // *data = result.first;

            if (result.second)
                return candleTypes::Error_t::UNKNOWN_ERROR;
        }
        return candleTypes::Error_t::OK;
    }

    const std::pair<std::vector<u8>, candleTypes::Error_t> CandleV2::transferCANFrame(
        std::shared_ptr<CandleV2> candle,
        const canId_t             canId,
        const std::vector<u8>     dataToSend,
        const size_t              responseSize,
        const u32                 timeoutMs)
    {
        candleTypes::Error_t communicationStatus = candleTypes::Error_t::OK;
        if (!candle->m_isInitialized)
            return std::pair<std::vector<u8>, candleTypes::Error_t>(
                dataToSend, candleTypes::Error_t::UNINITIALIZED);
        if (communicationStatus != candleTypes::Error_t::OK)
            return std::pair<std::vector<u8>, candleTypes::Error_t>(dataToSend,
                                                                    communicationStatus);

        candle->m_log.debug("SEND");
        candle->frameDump(dataToSend);

        if (dataToSend.size() > 64)
        {
            candle->m_log.error("CAN frame too long!");
            return std::pair<std::vector<u8>, candleTypes::Error_t>(
                dataToSend, candleTypes::Error_t::DATA_TOO_LONG);
        }

        auto buffer = std::vector<u8>(dataToSend);

        const auto candleCommandCANframe = sendCanFrameHeader(dataToSend.size(), u16(canId));

        buffer.insert(buffer.begin(), candleCommandCANframe.begin(), candleCommandCANframe.end());

        communicationStatus =
            candle->busTransfer(&buffer, responseSize + 2 /*response header size*/);

        // retransmission if wrong ID in the response, see Candle Commands v1 description in
        // documentation
        if (buffer.at(1) != 0x01)
        {
            buffer.clear();
            buffer = dataToSend;
            communicationStatus =
                candle->busTransfer(&buffer, responseSize + 2 /*response header size*/);
            if (buffer.at(1) != 0x01 || buffer.at(0) != GENERIC_CAN_FRAME)
            {
                candle->m_log.debug("Retransmitting can message to ID: %d", canId);
                candle->m_log.error("CAN frame did not reach target device with id: %d!", canId);
                return std::pair<std::vector<u8>, candleTypes::Error_t>(
                    dataToSend, candleTypes::Error_t::CAN_DEVICE_NOT_RESPONDING);
            }
        }

        if (buffer.size() > 3)
            buffer.erase(buffer.begin(), buffer.begin() + 2 /*response header size*/);

        auto response = buffer;

        candle->m_log.debug("Expected received len: %d", responseSize);
        candle->m_log.debug("RECEIVE");
        candle->frameDump(response);

        return std::pair<std::vector<u8>, candleTypes::Error_t>(response, communicationStatus);
    }

    // TODO: this must be changed to something less invasive
    candleTypes::Error_t CandleV2::legacyCheckConnection()
    {
        // auto baudrateFrame = baudrateCommandFrame(m_canBaudrate);

        // auto testConnectionFrame = std::vector<u8>(baudrateFrame.begin(), baudrateFrame.end());

        // const candleTypes::Error_t connectionStatus = busTransfer(&testConnectionFrame, 5);
        // if (connectionStatus != candleTypes::Error_t::OK)
        //     return connectionStatus;
        return candleTypes::Error_t::OK;
    }
}  // namespace mab