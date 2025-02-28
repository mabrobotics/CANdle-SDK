#pragma once

#include <array>
#include <memory>
#include <vector>
#include <utility>
#include <iomanip>
#include <map>

#include "candle_types.hpp"
#include "MD.hpp"
#include "logger.hpp"
#include "bus.hpp"
#include "I_communication_interface.hpp"
#include "mab_types.hpp"

namespace mab
{
    class CandleV2
    {
      public:
        static constexpr u32 DEFAULT_CAN_TIMEOUT = 1;
        /// @brief Command IDs to control Candle device behavior. With APIv1 it was prepended at the
        /// begining of the frame.
        enum CandleCommands_t : u8
        {
            NONE                       = 0,
            CANDLE_CONFIG_BAUDRATE     = 2,
            GENERIC_CAN_FRAME          = 4,
            RESET                      = 9,
            USB_FRAME_ENTER_BOOTLOADER = 10,
        };

        static constexpr u32 CANDLE_VID = 0x69;
        static constexpr u32 CANDLE_PID = 0x1000;

        CandleV2() = delete;

        ~CandleV2();

        /// @brief Create CANdle device object based on provided communication interface
        /// @param canBaudrate CAN network datarate
        /// @param bus Initialized communication interface
        explicit CandleV2(const CANdleBaudrate_E                           canBaudrate,
                          std::unique_ptr<mab::I_CommunicationInterface>&& bus);

        /// @brief Method for transfering CAN packets via CANdle device
        /// @param canId Target CAN node ID
        /// @param dataToSend Data to be transferred via CAN bus
        /// @param responseSize Size of the expected device response (0 for not expecting a
        /// response)
        /// @param timeoutMs Time after which candle will stop waiting for node response in
        /// miliseconds
        /// @return
        static const std::pair<std::vector<u8>, candleTypes::Error_t> transferCANFrame(
            std::weak_ptr<CandleV2> candle,
            const canId_t           canId,
            const std::vector<u8>   dataToSend,
            const size_t            responseSize,
            const u32               timeoutMs = DEFAULT_CAN_TIMEOUT);

        /// @brief Initialize candle
        candleTypes::Error_t init(std::weak_ptr<CandleV2> thisSharedRef);

        /// @brief This method clears currently known devices and discovers any MAB device that is
        /// on the CAN network
        candleTypes::Error_t discoverDevices();

        std::shared_ptr<std::map<canId_t, MD>> getMDmapHandle();

      private:
        static constexpr u32 DEFAULT_CONFIGURATION_TIMEOUT = 10;

        CANdleBaudrate_E m_canBaudrate = CANdleBaudrate_E::CAN_BAUD_1M;
        Logger           m_log         = Logger(Logger::ProgramLayer_E::TOP, "CANDLE");

        std::weak_ptr<CandleV2> m_thisSharedReference;

        std::unique_ptr<mab::I_CommunicationInterface> m_bus;
        std::shared_ptr<std::map<canId_t, MD>>         m_mdMap;

        bool m_isInitialized = false;

        candleTypes::Error_t busTransfer(std::vector<u8>* data,
                                         size_t           responseLength = 0,
                                         const u32        timeoutMs      = DEFAULT_CAN_TIMEOUT + 1);

        // TODO: this method is temporary and must be changed, must have some way for bus to check
        // functional connection
        candleTypes::Error_t legacyCheckConnection();

        static constexpr std::array<u8, 2> resetCommandFrame()
        {
            return std::array<u8, 2>({RESET, 0x0});
        }

        static inline std::vector<u8> baudrateCommandFrame(const CANdleBaudrate_E baudrate)
        {
            return std::vector<u8>({CANDLE_CONFIG_BAUDRATE, baudrate});
        }

        static inline std::vector<u8> sendCanFrameHeader(const u8&& length, const u16&& id)
        {
            return std::vector<u8>({GENERIC_CAN_FRAME,
                                    u8(length /*id + DLC*/),
                                    DEFAULT_CAN_TIMEOUT,
                                    u8(id),
                                    u8(id >> 8)});
        }

        inline void frameDump(std::vector<u8> frame) const
        {
            m_log.debug("FRAME DUMP");
            for (const auto byte : frame)
            {
                std::stringstream ss;
                ss << " 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)byte << " ";
                m_log.debug(ss.str().c_str());
            }
        }
    };

    /// @brief Initialize CANdle device
    /// @param baudrate CAN network datarate
    /// @param bus Initialized communication interface
    /// @return Initialized CANdle instance object
    inline std::shared_ptr<mab::CandleV2> attachCandle(
        const CANdleBaudrate_E baudrate, std::unique_ptr<I_CommunicationInterface>&& bus)
    {
        if (bus == nullptr)
            throw std::runtime_error("Could not create CANdle from an undefined bus!");

        auto candle = std::make_shared<mab::CandleV2>(baudrate, std::move(bus));
        if (candle->init(candle) != candleTypes::Error_t::OK)
        {
            throw std::runtime_error("Could not initialize CANdle device!");
        }
        return candle;
    }
}  // namespace mab