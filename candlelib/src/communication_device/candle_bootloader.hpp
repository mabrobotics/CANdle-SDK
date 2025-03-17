#pragma once
#include <memory>
#include <utility>

#include "I_communication_interface.hpp"
#include "USB_v2.hpp"
#include "logger.hpp"
#include "mab_types.hpp"
#include "candle_types.hpp"
#include "candle_v2.hpp"
namespace mab
{
    class CandleBootloader
    {
        enum BootloaderCommand_E : u8
        {
            NONE                           = 0,
            BOOTLOADER_FRAME_CHECK_ENTERED = 100,
            BOOTLOADER_FRAME_SEND_PAGE     = 101,
            BOOTLOADER_FRAME_WRITE_PAGE    = 102,
            BOOTLOADER_FRAME_BOOT_TO_APP   = 103,
        };

        std::unique_ptr<mab::I_CommunicationInterface> m_usb;

      public:
        static constexpr u32 BOOTLOADER_VID = 0x69;
        static constexpr u32 BOOTLOADER_PID = 0x2000;
        CandleBootloader()
        {
        }
        explicit CandleBootloader(std::unique_ptr<mab::I_CommunicationInterface> bus)
            : m_usb(std::move(bus))
        {
        }
        candleTypes::Error_t init();

        candleTypes::Error_t enterBootloaderFromApp(
            std::unique_ptr<mab::I_CommunicationInterface> bus);
        candleTypes::Error_t enterAppFromBootloader(
            std::unique_ptr<mab::I_CommunicationInterface> bus);
    };

    inline std::optional<std::unique_ptr<CandleBootloader>> attachCandleBootloader()
    {
        auto busApp = std::make_unique<USBv2>(CandleV2::CANDLE_VID, CandleV2::CANDLE_PID);
        if (busApp->connect() != I_CommunicationInterface::Error_t::OK)
            return {};
        if (CandleV2::enterBootloader(std::move(busApp)) != candleTypes::Error_t::OK)
            return {};
        sleep(2);  // wait for reboot
        auto busBoot = std::make_unique<USBv2>(CandleBootloader::BOOTLOADER_VID,
                                               CandleBootloader::BOOTLOADER_PID);
        if (busBoot->connect() == I_CommunicationInterface::Error_t::OK)
            return std::make_unique<CandleBootloader>(std::move(busBoot));

        return {};
    }
}  // namespace mab