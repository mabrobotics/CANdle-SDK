#include "candle.hpp"

#include <exception>
#include <MD.hpp>
#include <optional>
#include "candle_types.hpp"
#include "mab_types.hpp"

namespace mab
{
    Candle::~Candle()
    {
        m_log.debug("Deconstructing Candle, do not reuse any handles provided by it!\n");
        m_bus->disconnect();
        m_bus = nullptr;
    }

    Candle::Candle(const CANdleBaudrate_E                           canBaudrate,
                   std::unique_ptr<mab::I_CommunicationInterface>&& bus)
        : m_canBaudrate(canBaudrate), m_bus(std::move(bus))
    {
    }

    candleTypes::Error_t Candle::init()
    {
        if (m_bus == nullptr)
        {
            m_log.error("Bus not initialized!");
            return candleTypes::Error_t::INITIALIZATION_ERROR;
        }
        m_bus->disconnect();
        I_CommunicationInterface::Error_t connectStatus = m_bus->connect();
        if (connectStatus != I_CommunicationInterface::Error_t::OK)
        {
            m_isInitialized = false;
            return candleTypes::Error_t::INITIALIZATION_ERROR;
        }

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

    candleTypes::Error_t Candle::reset()
    {
        std::vector<u8> resetCmd;
        for (auto byte : resetCommandFrame())
        {
            resetCmd.push_back(byte);
        }
        auto result = busTransfer(&resetCmd, 2);
        if (result != candleTypes::Error_t::OK)
        {
            m_log.error("Reset failed!");
            return result;
        }
        if (resetCmd.at(1) != 0x1)
        {
            m_log.error("Reset failed!");
            return candleTypes::Error_t::UNKNOWN_ERROR;
        }
        return candleTypes::Error_t::OK;
    }
    std::optional<version_ut> Candle::getCandleVersion()
    {
        auto buffer       = baudrateCommandFrame(m_canBaudrate);
        auto baudResponse = busTransfer(&buffer, 6);
        if (baudResponse != candleTypes::Error_t::OK || buffer.size() < 6)
        {
            return std::nullopt;
        }
        else
        {
            version_ut candleVersion;
            candleVersion.s.tag      = buffer[2];
            candleVersion.s.revision = buffer[3];
            candleVersion.s.minor    = buffer[4];
            candleVersion.s.major    = buffer[5];
            return candleVersion;
        }
    }

    candleTypes::Error_t Candle::busTransfer(std::vector<u8>* data,
                                             size_t           responseLength,
                                             const u32        timeoutMs) const
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

    candleTypes::Error_t Candle::busTransferPDO(std::vector<u8>* data,
                                                size_t           responseLength,
                                                const u32        timeoutMs) const
    {
        // return candleTypes::Error_t::OK;
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
            I_CommunicationInterface::Error_t comError = m_bus->transferPDO(*data, timeoutMs);
            if (comError)
                return candleTypes::Error_t::UNKNOWN_ERROR;
        }
        else
        {
            std::pair<std::vector<u8>, I_CommunicationInterface::Error_t> result =
                m_bus->transferPDO(*data, timeoutMs, responseLength);

            data->clear();
            data->insert(data->begin(), result.first.begin(), result.first.end());
            // *data = result.first;
            // cout << "Result second:" << result.second << endl;
            if (result.second)
                return candleTypes::Error_t::UNKNOWN_ERROR;
        }
        return candleTypes::Error_t::OK;
    }

    const std::pair<std::vector<u8>, candleTypes::Error_t> Candle::transferCANFrame(
        const canId_t         canId,
        const std::vector<u8> dataToSend,
        const size_t          responseSize,
        const u32             timeoutMs) const
    {
        candleTypes::Error_t communicationStatus = candleTypes::Error_t::OK;

        if (!m_isInitialized)
            return std::pair<std::vector<u8>, candleTypes::Error_t>(
                dataToSend, candleTypes::Error_t::UNINITIALIZED);
        if (communicationStatus != candleTypes::Error_t::OK)
            return std::pair<std::vector<u8>, candleTypes::Error_t>(dataToSend,
                                                                    communicationStatus);

        m_log.debug("SEND");
        // frameDump(dataToSend);  // can be enabled for in depth debugging

        if (dataToSend.size() > 64)
        {
            m_log.error("CAN frame too long!");
            return std::pair<std::vector<u8>, candleTypes::Error_t>(
                dataToSend, candleTypes::Error_t::DATA_TOO_LONG);
        }

        auto buffer = std::vector<u8>(dataToSend);

        const auto candleCommandCANframe =
            sendCanFrameHeader(dataToSend.size(), u16(canId), timeoutMs);

        buffer.insert(buffer.begin(), candleCommandCANframe.begin(), candleCommandCANframe.end());

        communicationStatus =
            busTransfer(&buffer, responseSize + 2 /*response header size*/, timeoutMs + 1);

        if (buffer.at(1) != 0x01)
        {
            m_log.error("CAN frame did not reach target device with id: %d!", canId);
            return std::pair<std::vector<u8>, candleTypes::Error_t>(
                dataToSend, candleTypes::Error_t::CAN_DEVICE_NOT_RESPONDING);
        }

        if (buffer.size() > 3)
            buffer.erase(buffer.begin(), buffer.begin() + 2 /*response header size*/);

        auto response = buffer;

        m_log.debug("Expected received len: %d", responseSize);
        m_log.debug("RECEIVE");
        // frameDump(response);

        return std::pair<std::vector<u8>, candleTypes::Error_t>(response, communicationStatus);

        // auto buffer = std::vector<u8>(dataToSend);

        // const auto candleCommandCANframe =
        //     sendCanFrameHeader(dataToSend.size(), u16(canId), timeoutMs);

        // buffer.insert(buffer.begin(), candleCommandCANframe.begin(),
        // candleCommandCANframe.end());

        // communicationStatus =
        //     busTransfer(&buffer, responseSize + 2 /*response header size*/, timeoutMs + 1);

        // auto response = buffer;

        // m_log.debug("Expected received len: %d", responseSize);
        // m_log.debug("RECEIVE");
        // // frameDump(response);

        // return std::pair<std::vector<u8>, candleTypes::Error_t>(response, communicationStatus);
    }

    const std::pair<std::vector<u8>, candleTypes::Error_t> Candle::transferCANPDOFrame(
        const canId_t         canId,
        const std::vector<u8> dataToSend,
        const size_t          responseSize,
        const u32             timeoutMs) const
    {
        candleTypes::Error_t communicationStatus = candleTypes::Error_t::OK;

        if (!m_isInitialized)
            return std::pair<std::vector<u8>, candleTypes::Error_t>(
                dataToSend, candleTypes::Error_t::UNINITIALIZED);
        if (communicationStatus != candleTypes::Error_t::OK)
            return std::pair<std::vector<u8>, candleTypes::Error_t>(dataToSend,
                                                                    communicationStatus);

        m_log.debug("SEND");
        // frameDump(dataToSend);  // can be enabled for in depth debugging

        if (dataToSend.size() > 64)
        {
            m_log.error("CAN frame too long!");
            return std::pair<std::vector<u8>, candleTypes::Error_t>(
                dataToSend, candleTypes::Error_t::DATA_TOO_LONG);
        }

        auto buffer = std::vector<u8>(dataToSend);

        const auto candleCommandCANframe =
            sendCanFrameHeader(dataToSend.size(), u16(canId), timeoutMs);

        buffer.insert(buffer.begin(), candleCommandCANframe.begin(), candleCommandCANframe.end());

        communicationStatus =
            busTransferPDO(&buffer, responseSize + 2 /*response header size*/, timeoutMs + 1);

        auto response = buffer;

        m_log.debug("Expected received len: %d", responseSize);
        m_log.debug("RECEIVE");
        // frameDump(response);

        return std::pair<std::vector<u8>, candleTypes::Error_t>(response, communicationStatus);
    }

    // TODO: this must be changed to something less invasive
    candleTypes::Error_t Candle::legacyCheckConnection()
    {
        auto baudrateFrame = baudrateCommandFrame(m_canBaudrate);

        auto testConnectionFrame = std::vector<u8>(baudrateFrame.begin(), baudrateFrame.end());

        const candleTypes::Error_t connectionStatus = busTransfer(&testConnectionFrame, 5);
        if (connectionStatus != candleTypes::Error_t::OK)
            return connectionStatus;
        return candleTypes::Error_t::OK;
    }

    candleTypes::Error_t Candle::enterBootloader(
        std::unique_ptr<mab::I_CommunicationInterface>&& usb)
    {
        Candle candleApp = Candle(CANdleBaudrate_E::CAN_BAUD_1M, std::move(usb));

        std::vector<u8> enterBootloaderCmd;
        for (auto byte : Candle::enterBootloaderFrame())
        {
            enterBootloaderCmd.push_back(byte);
        }
        auto result = candleApp.busTransfer(&enterBootloaderCmd);
        return result;
    }
}  // namespace mab
