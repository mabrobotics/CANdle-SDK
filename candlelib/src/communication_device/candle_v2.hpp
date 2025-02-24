#pragma once

#include <array>
#include <memory>
#include <vector>
#include <utility>
#include <iomanip>

#include "logger.hpp"

#include "bus.hpp"
#include "I_communication_interface.hpp"
#include "mab_types.hpp"

namespace mab
{
    class CandleV2
    {
      public:
        static constexpr u32 DEFAULT_CAN_TIMEOUT = 5;
        enum Error_t
        {
            OK,
            DEVICE_NOT_CONNECTED,
            INITIALIZATION_ERROR,
            UNINITIALIZED,
            DATA_TOO_LONG,
            DATA_EMPTY,
            RESPONSE_TIMEOUT,
            CAN_DEVICE_NOT_RESPONDING,
            UNKNOWN_ERROR
        };
        /// @brief Command IDs to control Candle device behavior
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

        explicit CandleV2(const CANdleBaudrate_E                           canBaudrate,
                          std::unique_ptr<mab::I_CommunicationInterface>&& bus);

        const std::pair<std::vector<u8>, Error_t> transferCANFrame(
            const u32             canId,
            const std::vector<u8> dataToSend,
            const size_t          responseSize,
            const u32             timeoutMs = DEFAULT_CAN_TIMEOUT);

        Error_t init();

      private:
        static constexpr u32 DEFAULT_CONFIGURATION_TIMEOUT = 10;

        CANdleBaudrate_E m_canBaudrate = CANdleBaudrate_E::CAN_BAUD_1M;
        Logger           m_log         = Logger(Logger::ProgramLayer_E::TOP, "CANDLE");
        std::unique_ptr<mab::I_CommunicationInterface> m_bus;

        bool m_isInitialized = false;

        // TODO: this method is temporary until bus rework
        Error_t busTransfer(std::shared_ptr<std::vector<u8>> data,
                            size_t                           responseLength = 0,
                            const u32                        timeoutMs      = DEFAULT_CAN_TIMEOUT);

        Error_t busTransfer(const std::vector<u8>&& data);

        // TODO: this method is temporary and must be changed, must have some way for bus to check
        // functional connection
        Error_t legacyCheckConnection();

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

    // TODO: make baudrate as template so it can be constexpred in helper methods
    inline std::shared_ptr<mab::CandleV2> attachCandle(
        const CANdleBaudrate_E baudrate, std::unique_ptr<I_CommunicationInterface>&& bus)
    {
        if (bus == nullptr)
            throw std::runtime_error("Could not create CANdle from an undefined bus!");

        auto candle = std::make_shared<mab::CandleV2>(baudrate, std::move(bus));
        if (candle->init() != CandleV2::Error_t::OK)
        {
            throw std::runtime_error("Could not initialize CANdle device!");
        }
        return candle;
    }
}  // namespace mab