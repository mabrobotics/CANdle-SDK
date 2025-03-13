#pragma once
#include "I_communication_interface_mock.hpp"
#include "logger.hpp"
#include "mab_types.hpp"

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
    };
}  // namespace mab