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
      private:
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

        CANdleBaudrate_E          m_canBaudrate = CANdleBaudrate_E::CAN_BAUD_1M;
        Logger                    m_log         = Logger(Logger::ProgramLayer_E::TOP, "CANDLE");
        std::unique_ptr<mab::Bus> m_bus;

        bool isInitialized = false;

        Error_t init();

        // this method is temporary until bus rework
        Error_t legacyBusTransfer(std::vector<u8>* data, u32 timeout_ms);

      public:
        CandleV2() = delete;
        explicit CandleV2(CANdleBaudrate_E canBaudrate, std::unique_ptr<mab::Bus>&& bus);
        explicit CandleV2(CANdleBaudrate_E canBaudrate, mab::Bus&& bus);

        std::pair<std::vector<u8>, Error_t> transferData(std::vector<u8> dataToSend) override;
    };
}  // namespace mab