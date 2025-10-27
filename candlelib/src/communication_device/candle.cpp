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

    Candle::Candle(const CANdleDatarate_E                           canDatarate,
                   std::unique_ptr<mab::I_CommunicationInterface>&& bus,
                   bool                                             useRegularCANFrames)
        : m_canDatarate(canDatarate),
          m_bus(std::move(bus)),
          m_useRegularCanFrames(useRegularCANFrames),
          m_maxCANFrameSize(useRegularCANFrames ? 8 : 64),
          m_cfsync(std::make_shared<std::function<void(void)>>()),
          m_cfAdapter(m_cfsync)
    {
        if (m_useRegularCanFrames)
            m_log.debug("CANdle initialized with regular CAN format, max frame size is %u",
                        m_maxCANFrameSize);
        else
            m_log.debug("CANdle initialized with CAN-FD format, max frame size is %u",
                        m_maxCANFrameSize);

        if (canDatarate != CANdleDatarate_E::CAN_DATARATE_1M && useRegularCANFrames)
        {
            throw std::runtime_error(
                "Regular CAN does not support datarate above 1Mbps. Either use CAN-FD or lower the "
                "datarate to 1M.");
        }
        *m_cfsync = std::function<void(void)>(
            [this]()
            {
                std::unique_lock lock(this->m_cfSyncMux);
                this->m_log.debug("CF transfer requested");
                // Start CF transfer thread if not already running
                if (!this->m_cfTransferThread.joinable() || !this->m_cfTransferAlive.load())
                {
                    this->m_log.debug("Spinning up CF transfer thread");
                    this->m_cfTransferAlive.store(true);
                    this->m_cfTransferThread = std::jthread([this](std::stop_token stoken)
                                                            { this->cfTransferLoop(stoken); });
                }
                this->m_log.debug("Releasing CF transfer semaphore");
                this->m_cfTransferSemaphore.release();
            });
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
        auto buffer       = datarateCommandFrame(m_canDatarate, m_useRegularCanFrames);
        auto dataResponse = busTransfer(&buffer, 6);
        if (dataResponse != candleTypes::Error_t::OK || buffer.size() < 6)
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

    void Candle::cfTransferLoop(std::stop_token stopToken) noexcept
    {
        m_cfTransferAlive.store(true);
        while (!stopToken.stop_requested())
        {
            m_log.debug("CF transfer thread waiting for semaphore...");
            auto result = m_cfTransferSemaphore.try_acquire_for(DEFAULT_CONFIGURATION_TIMEOUT);
            if (result)
            {
                while (m_cfAdapter.getCount() > 0)
                {
                    if (stopToken.stop_requested())
                        break;
                    std::vector<u8> packedFrame = m_cfAdapter.getPackedFrame();
                    if (packedFrame.size() < 4)
                    {
                        m_log.warn("CF transfer packed frame empty!");
                        break;
                    }
                    m_log.debug("CF transfer thread sending frame");
                    candleTypes::Error_t transferStatus = busTransfer(
                        &packedFrame, packedFrame.size(), DEFAULT_CONFIGURATION_TIMEOUT.count());
                    if (transferStatus != candleTypes::Error_t::OK)
                    {
                        m_log.error("Candle transfer failed!");
                        break;
                    }
                    if (m_cfAdapter.parsePackedFrame(packedFrame) !=
                        CANdleFrameAdapter::Error_t::OK)
                    {
                        m_log.error("CF transfer parsing failed!");
                        break;
                    }
                }
            }
            else
            {
                m_log.debug("CF transfer thread timeout");
                break;
            }
        }
        m_cfTransferAlive.store(false);
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

        if (dataToSend.size() > m_maxCANFrameSize)
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
            busTransfer(&buffer,
                        64 /*TODO: this is legacy Candle stuff*/ + 2 /*response header size*/,
                        timeoutMs + 1);

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
    }

    // TODO: this must be changed to something less invasive
    candleTypes::Error_t Candle::legacyCheckConnection()
    {
        auto datarateFrame = datarateCommandFrame(m_canDatarate, m_useRegularCanFrames);

        auto testConnectionFrame = std::vector<u8>(datarateFrame.begin(), datarateFrame.end());

        const candleTypes::Error_t connectionStatus = busTransfer(&testConnectionFrame, 6);
        if (connectionStatus != candleTypes::Error_t::OK)
            return connectionStatus;
        return candleTypes::Error_t::OK;
    }

    candleTypes::Error_t Candle::enterBootloader(
        std::unique_ptr<mab::I_CommunicationInterface>&& usb)
    {
        Candle candleApp = Candle(CANdleDatarate_E::CAN_DATARATE_1M, std::move(usb));

        std::vector<u8> enterBootloaderCmd;
        for (auto byte : Candle::enterBootloaderFrame())
        {
            enterBootloaderCmd.push_back(byte);
        }
        auto result = candleApp.busTransfer(&enterBootloaderCmd);
        return result;
    }
}  // namespace mab
