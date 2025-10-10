#pragma once

#include <array>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>
#include <utility>
#include <iomanip>
#include <map>

#include "candle_types.hpp"
#include "logger.hpp"
#include "I_communication_interface.hpp"
#include "USB.hpp"
#include "SPI.hpp"
#include "mab_types.hpp"

namespace mab
{
    /// @brief Backbone object for CANdle ecosystem, contains all the peripheral handles. When it is
    /// destroyed all the handles become invalid so manage its lifetime carefully.
    class Candle
    {
      public:
        static constexpr u32 DEFAULT_CAN_TIMEOUT = 1;
        /// @brief Command IDs to control Candle device behavior. With APIv1 it was prepended at the
        /// begining of the frame.
        enum CandleCommands_t : u8
        {
            NONE                   = 0,
            CANDLE_CONFIG_DATARATE = 2,
            GENERIC_CAN_FRAME      = 4,
            RESET                  = 9,
            ENTER_BOOTLOADER       = 10,
        };

        static constexpr u32 CANDLE_VID = 0x69;
        static constexpr u32 CANDLE_PID = 0x1000;

        Candle() = delete;

        Candle(const Candle&) = delete;

        ~Candle();

        /// @brief Create CANdle device object based on provided communication interface
        /// @param canDatarate CAN network datarate
        /// @param bus Initialized communication interface
        explicit Candle(const CANdleDatarate_E                           canDatarate,
                        std::unique_ptr<mab::I_CommunicationInterface>&& bus,
                        bool useRegularCANFrames = false);

        /// @brief Method for transfering CAN packets via CANdle device
        /// @param canId Target CAN node ID
        /// @param dataToSend Data to be transferred via CAN bus
        /// @param responseSize Size of the expected device response (0 for not expecting a
        /// response)
        /// @param timeoutMs Time after which candle will stop waiting for node response in
        /// miliseconds
        /// @return
        const std::pair<std::vector<u8>, candleTypes::Error_t> transferCANFrame(
            const canId_t         canId,
            const std::vector<u8> dataToSend,
            const size_t          responseSize,
            const u32             timeoutMs = DEFAULT_CAN_TIMEOUT) const;

        /// @brief Initialize candle
        candleTypes::Error_t init();

        /// @brief Reset candle device
        /// @return Error on failure
        candleTypes::Error_t reset();

        std::optional<version_ut> getCandleVersion();

        /// @brief Command the application to reboot into a bootloader and await commands.
        /// @param usb initialized usb interface (bootloader only works via USB)
        /// @return Error on failure
        static candleTypes::Error_t enterBootloader(
            std::unique_ptr<mab::I_CommunicationInterface>&& usb);

        const CANdleDatarate_E m_canDatarate;

      private:
        static constexpr u32 DEFAULT_CONFIGURATION_TIMEOUT = 10;

        Logger m_log = Logger(Logger::ProgramLayer_E::TOP, "CANDLE");

        std::unique_ptr<mab::I_CommunicationInterface> m_bus;

        bool         m_isInitialized       = false;
        const bool   m_useRegularCanFrames = false;
        const size_t m_maxCANFrameSize     = 64;

        candleTypes::Error_t busTransfer(std::vector<u8>* data,
                                         size_t           responseLength = 0,
                                         const u32 timeoutMs = DEFAULT_CAN_TIMEOUT + 1) const;

        // TODO: this method is temporary and must be changed, must have some way for bus to check
        // functional connection
        candleTypes::Error_t legacyCheckConnection();

        static constexpr std::array<u8, 2> resetCommandFrame()
        {
            return std::array<u8, 2>({RESET, 0x0});
        }
        static constexpr std::array<u8, 2> enterBootloaderFrame()
        {
            return std::array<u8, 2>({ENTER_BOOTLOADER, 0x0});
        }

        static inline std::vector<u8> datarateCommandFrame(const CANdleDatarate_E datarate,
                                                           const u8               regularCanFormat)
        {
            return std::vector<u8>({CANDLE_CONFIG_DATARATE, datarate, regularCanFormat});
        }

        static inline std::vector<u8> sendCanFrameHeader(const u8&&  length,
                                                         const u16&& id,
                                                         const u8    timeout = DEFAULT_CAN_TIMEOUT)
        {
            return std::vector<u8>(
                {GENERIC_CAN_FRAME, u8(length /*id + DLC*/), timeout, u8(id), u8(id >> 8)});
        }

        inline void frameDump(std::vector<u8> frame) const
        {
            m_log.debug("FRAME DUMP");
            for (const auto byte : frame)
            {
                std::stringstream ss;
                ss << " 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)byte << " "
                   << "[ " << ((char)byte == 0 ? '0' : (char)byte) << " ]";
                m_log.debug(ss.str().c_str());
            }
        }
    };

    /// @brief Create CANdle device instance
    /// @param datarate Target data-rate of the CAN bus
    /// @param bus Initialized CANdle communication interface
    /// @return Configured candle object or nullptr
    inline mab::Candle* attachCandle(const CANdleDatarate_E                      datarate,
                                     std::unique_ptr<I_CommunicationInterface>&& bus)
    {
        Logger log(Logger::ProgramLayer_E::TOP, "CANDLE_BUILDER");
        if (bus == nullptr)
        {
            log.error("Could not create CANdle from an undefined bus!");
            return {};
        }

        mab::Candle* candle = new mab::Candle(datarate, std::move(bus));
        if (candle == nullptr || candle->init() != candleTypes::Error_t::OK)
        {
            log.error("Could not initialize CANdle device!");
            return {};
        }
        return candle;
    }

    /// @brief Create CANdle device instance
    /// @param datarate Target data-rate of the CAN bus
    /// @param bus Initialized CANdle communication interface
    /// @return Configured candle object or nullptr
    inline mab::Candle* attachCandle(const CANdleDatarate_E  datarate,
                                     candleTypes::busTypes_t busType)
    {
        std::unique_ptr<mab::I_CommunicationInterface> bus;
        switch (busType)
        {
            case candleTypes::busTypes_t::USB:
                bus = std::make_unique<mab::USB>(mab::Candle::CANDLE_VID, mab::Candle::CANDLE_PID);
                if (bus->connect() != mab::I_CommunicationInterface::Error_t::OK)
                    throw std::runtime_error("Could not connect USB device!");
                return attachCandle(datarate, std::move(bus));
            case candleTypes::busTypes_t::SPI:
                bus = std::make_unique<mab::SPI>();
                if (bus->connect() != mab::I_CommunicationInterface::Error_t::OK)
                    throw std::runtime_error("Could not connect SPI device!");
                return attachCandle(datarate, std::move(bus));
            default:
                throw std::runtime_error("Wrong communication interface provided!");
                return {};
        }
    }

    /// @brief Destroy candle object. Must be called after each initialization of CANdle
    /// @param candle Candle object to be destroyed
    inline void detachCandle(Candle* candle)
    {
        if (candle != nullptr)
            delete candle;
    }

    class CandleBuilder
    {
        Logger m_logger = Logger(Logger::ProgramLayer_E::TOP, "CANDLE_BUILDER");

      public:
        CandleBuilder() = default;

        std::shared_ptr<CANdleDatarate_E>        datarate = nullptr;
        std::shared_ptr<candleTypes::busTypes_t> busType  = nullptr;
        std::optional<std::string_view>          pathOrId;

        std::function<void()> preBuildTask = []() {};

        std::optional<Candle*> build() const
        {
            preBuildTask();
            if (datarate == nullptr || busType == nullptr)
            {
                m_logger.error("Parameters missing. Could create Candle.");
                return {};
            }
            std::unique_ptr<I_CommunicationInterface> bus;
            switch (*busType)
            {
                case candleTypes::busTypes_t::USB:
                    bus = std::make_unique<mab::USB>(Candle::CANDLE_VID,
                                                     Candle::CANDLE_PID,
                                                     std::string(pathOrId.value_or(std::string())));
                    if (bus->connect() != I_CommunicationInterface::Error_t::OK)
                    {
                        m_logger.error("Could not connect USB device!");
                        return {};
                    }
                    break;
                case mab::candleTypes::SPI:
                    bus = std::make_unique<mab::SPI>(
                        std::string(pathOrId.value_or("/dev/spidev0.0")));
                    if (bus->connect() != I_CommunicationInterface::Error_t::OK)
                    {
                        m_logger.error("Could not connect USB device!");
                        return {};
                    }
                    break;
                default:
                    m_logger.error("Unimplemented bus type");
                    return {};
            }
            Candle* candle = new Candle(*datarate, std::move(bus));
            if (candle == nullptr || candle->init() != candleTypes::Error_t::OK)
            {
                m_logger.error("Could not initialize CANdle device!");
                return {};
            }
            return candle;
        }
    };
}  // namespace mab
