#include "candle_v2.hpp"

#include <exception>
#include <MD.hpp>

namespace mab
{
    CandleV2::~CandleV2()
    {
        m_log.debug("Deconstructing Candle, do not reuse any handles provided by it!\n");
        m_mdMap->clear();
        m_bus->disconnect();
        m_bus = nullptr;
    }

    CandleV2::CandleV2(const CANdleBaudrate_E                           canBaudrate,
                       std::unique_ptr<mab::I_CommunicationInterface>&& bus)
        : m_canBaudrate(canBaudrate), m_bus(std::move(bus))
    {
        m_mdMap = std::make_shared<std::map<canId_t, MD>>();
    }

    candleTypes::Error_t CandleV2::init(std::weak_ptr<CandleV2> thisSharedRef)
    {
        m_thisSharedReference = thisSharedRef;
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

    candleTypes::Error_t CandleV2::discoverDevices()
    {
        using namespace std::placeholders;
        m_mdMap->clear();

        constexpr canId_t minValidId = 10;     // ids less than that are reserved for special
        constexpr canId_t maxValidId = 0x7FF;  // 11-bit value (standard can ID max)
        m_log.info("Looking for MDs");

        for (canId_t id = minValidId; id < maxValidId; id++)
        {
            m_log.debug("Trying to bind MD with id %d", id);
            m_log.progress(float(id) / float(maxValidId));
            // workaround for ping error spam
            Logger::Verbosity_E prevVerbosity =
                Logger::g_m_verbosity.value_or(Logger::Verbosity_E::VERBOSITY_1);
            Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;
            std::function<canTransmitFrame_t> transmitCanFrameMethod =
                std::bind(CandleV2::transferCANFrame, m_thisSharedReference, _1, _2, _3, _4);
            m_mdMap->emplace(id, MD(id, transmitCanFrameMethod));
            auto initStatus = m_mdMap->at(id).init();
            if (initStatus == MD::Error_t::OK)
            {
                Logger::g_m_verbosity = prevVerbosity;
            }
            else
                m_mdMap->erase(id);

            Logger::g_m_verbosity = prevVerbosity;
        }
        for (const auto& mdPair : *m_mdMap)
        {
            m_log.info("Discovered MD device with ID: %d", mdPair.first);
        }
        if (m_mdMap->size() > 0)
            return candleTypes::Error_t::OK;

        m_log.warn("Have not found any MD devices on the CAN bus!");
        // TODO: add pds discovery
        return candleTypes::Error_t::CAN_DEVICE_NOT_RESPONDING;
    }

    std::shared_ptr<std::map<canId_t, MD>> CandleV2::getMDmapHandle()
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
        std::weak_ptr<CandleV2> candle,
        const canId_t           canId,
        const std::vector<u8>   dataToSend,
        const size_t            responseSize,
        const u32               timeoutMs)
    {
        candleTypes::Error_t communicationStatus = candleTypes::Error_t::OK;
        auto                 candleSp            = candle.lock();
        if (!candleSp)
            throw std::runtime_error("Candle is empty");

        if (!candleSp->m_isInitialized)
            return std::pair<std::vector<u8>, candleTypes::Error_t>(
                dataToSend, candleTypes::Error_t::UNINITIALIZED);
        if (communicationStatus != candleTypes::Error_t::OK)
            return std::pair<std::vector<u8>, candleTypes::Error_t>(dataToSend,
                                                                    communicationStatus);

        candleSp->m_log.debug("SEND");
        candleSp->frameDump(dataToSend);

        if (dataToSend.size() > 64)
        {
            candleSp->m_log.error("CAN frame too long!");
            return std::pair<std::vector<u8>, candleTypes::Error_t>(
                dataToSend, candleTypes::Error_t::DATA_TOO_LONG);
        }

        auto buffer = std::vector<u8>(dataToSend);

        const auto candleCommandCANframe = sendCanFrameHeader(dataToSend.size(), u16(canId));

        buffer.insert(buffer.begin(), candleCommandCANframe.begin(), candleCommandCANframe.end());

        communicationStatus =
            candleSp->busTransfer(&buffer, responseSize + 2 /*response header size*/);

        if (buffer.at(1) != 0x01)
        {
            candleSp->m_log.error("CAN frame did not reach target device with id: %d!", canId);
            return std::pair<std::vector<u8>, candleTypes::Error_t>(
                dataToSend, candleTypes::Error_t::CAN_DEVICE_NOT_RESPONDING);
        }

        if (buffer.size() > 3)
            buffer.erase(buffer.begin(), buffer.begin() + 2 /*response header size*/);

        auto response = buffer;

        candleSp->m_log.debug("Expected received len: %d", responseSize);
        candleSp->m_log.debug("RECEIVE");
        candleSp->frameDump(response);

        return std::pair<std::vector<u8>, candleTypes::Error_t>(response, communicationStatus);
    }

    // TODO: this must be changed to something less invasive
    candleTypes::Error_t CandleV2::legacyCheckConnection()
    {
        auto baudrateFrame = baudrateCommandFrame(m_canBaudrate);

        auto testConnectionFrame = std::vector<u8>(baudrateFrame.begin(), baudrateFrame.end());

        const candleTypes::Error_t connectionStatus = busTransfer(&testConnectionFrame, 5);
        if (connectionStatus != candleTypes::Error_t::OK)
            return connectionStatus;
        return candleTypes::Error_t::OK;
    }
}  // namespace mab