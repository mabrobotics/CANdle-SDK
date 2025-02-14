#pragma once

#include <array>
#include <memory>
#include <vector>
#include <utility>

#include "logger.hpp"

#include "bus.hpp"
#include "I_communication_device.hpp"
#include "I_communication_interface.hpp"
#include "mab_types.hpp"

namespace mab
{
    class CandleV2 : I_CommunicationDevice
    {
      public:
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

        /// @brief Create an instance of candle
        /// @param canBaudrate Candle datarate to communicate via the CAN-FD
        /// @param bus Interface to communicate with the Candle
        explicit CandleV2(const CANdleBaudrate_E                           canBaudrate,
                          std::unique_ptr<mab::I_CommunicationInterface>&& bus);

        /// @brief Exchange frames on the CAN-FD bus
        /// @param dataToSend Data to be sent to can bus
        /// @param responseSize Length of the expected response CAN frame
        /// @return Pair of response frame and Candle device errors. If error is not OK the data is
        /// undefined.
        const std::pair<std::vector<u8>, Error_t> transferCANFrame(
            const std::vector<u8> dataToSend, const size_t responseSize) override;

        Error_t init() override;

      private:
        static constexpr u32 DEFAULT_CONFIGURATION_TIMEOUT = 10;
        static constexpr u32 DEFAULT_CAN_TIMEOUT           = 5;

        CANdleBaudrate_E m_canBaudrate = CANdleBaudrate_E::CAN_BAUD_1M;
        Logger           m_log         = Logger(Logger::ProgramLayer_E::TOP, "CANDLE");
        std::unique_ptr<mab::I_CommunicationInterface> m_bus;

        bool m_isInitialized = false;

        // TODO: this method is temporary until bus rework
        Error_t legacyBusTransfer(std::shared_ptr<std::vector<u8>> data, size_t responseLength = 0);

        Error_t legacyBusTransfer(const std::vector<u8>&& data);

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

        static inline std::vector<u8> sendCanFrameHeader(const u8&& length)
        {
            return std::vector<u8>(
                {GENERIC_CAN_FRAME, u8(length - 2 /*id + DLC*/), DEFAULT_CAN_TIMEOUT});
        }
    };

    // TODO: make baudrate as template so it can be constexpred in helper methods
    inline std::shared_ptr<mab::CandleV2> attachCandle(
        const CANdleBaudrate_E baudrate, std::unique_ptr<I_CommunicationInterface>&& bus)
    {
        if (bus == nullptr)
            throw std::runtime_error("Could not create CANdle from an undefined bus!");

        auto candle = std::make_shared<mab::CandleV2>(baudrate, std::move(bus));
        if (candle->init() != I_CommunicationDevice::Error_t::OK)
        {
            throw std::runtime_error("Could not initialize CANdle device!");
        }
        return candle;
    }
}  // namespace mab