#pragma once

#include <array>
#include <memory>
#include <vector>
#include <utility>

#include "logger.hpp"

#include "bus.hpp"
#include "I_communication_device.hpp"
#include "mab_types.hpp"

namespace mab
{
    class CandleV2 : I_CommunicationDevice
    {
      public:
        enum CandleCommands_t : u8
        {
            NONE                       = 0,
            PING_START                 = 1,
            CANDLE_CONFIG_BAUDRATE     = 2,
            MD_ADD                     = 3,
            MD_GENERIC_FRAME           = 4,
            MD_CONFIG_CAN              = 5,
            BEGIN                      = 6,
            END                        = 7,
            UPDATE                     = 8,
            RESET                      = 9,
            USB_FRAME_ENTER_BOOTLOADER = 10,
        };

      private:
        static constexpr u32 DEFAULT_CONFIGURATION_TIMEOUT = 10;
        static constexpr u32 DEFAULT_CAN_TIMEOUT           = 50;

        const std::array<u8, 2> CAN_FRAME_CANDLE_COMMAND = {MD_GENERIC_FRAME, 0x0};

        CANdleBaudrate_E          m_canBaudrate = CANdleBaudrate_E::CAN_BAUD_1M;
        Logger                    m_log         = Logger(Logger::ProgramLayer_E::TOP, "CANDLE");
        std::unique_ptr<mab::Bus> m_bus;

        bool m_isInitialized = false;

        Error_t init();
        Error_t reinit();

        // TODO: this method is temporary until bus rework
        Error_t legacyBusTransfer(std::shared_ptr<std::vector<u8>> datas);

        // TODO: this method is temporary and must be changed, must have some way for bus to check
        // functional connection
        Error_t legacyCheckConnection();

      public:
        CandleV2() = delete;
        explicit CandleV2(const CANdleBaudrate_E canBaudrate, std::unique_ptr<mab::Bus>&& bus);

        const std::pair<std::vector<u8>, Error_t> transferData(
            const std::vector<u8> dataToSend) override;
    };

    inline std::shared_ptr<mab::CandleV2> attachCandle(const CANdleBaudrate_E      baudrate,
                                                       std::unique_ptr<mab::Bus>&& bus)
    {
        return std::make_shared<mab::CandleV2>(baudrate, std::move(bus));
    }
}  // namespace mab