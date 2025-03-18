#pragma once
#include <memory>
#include <utility>
#include <vector>

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
        Logger m_log = Logger(Logger::ProgramLayer_E::TOP, "USB_BOOTLOADER");

        candleTypes::Error_t sendCmd(const BootloaderCommand_E cmd,
                                     const std::vector<u8>     payload) const;

      public:
        static constexpr u32 BOOTLOADER_VID = 0x69;
        static constexpr u32 BOOTLOADER_PID = 0x2000;

        static constexpr size_t PAGE_SIZE_STM32G474 = 0x800;
        /* implementation inside candle bootloader */
        static constexpr size_t CANDLE_BOOTLOADER_BUFFER_SIZE = 0x800;
        /* u8 id + 0xAA + 0xAA */
        static constexpr size_t PROTOCOL_HEADER_SIZE = 0x003;

        explicit CandleBootloader(std::unique_ptr<mab::I_CommunicationInterface> bus)
            : m_usb(std::move(bus))
        {
            m_log.debug("Created");
        }
        ~CandleBootloader();
        candleTypes::Error_t init();

        candleTypes::Error_t enterAppFromBootloader();
        candleTypes::Error_t writePage(const std::array<u8, PAGE_SIZE_STM32G474> page,
                                       const u32                                 crc32);
    };

    inline std::optional<std::unique_ptr<CandleBootloader>> attachCandleBootloader()
    {
        auto busApp = std::make_unique<USBv2>(CandleV2::CANDLE_VID, CandleV2::CANDLE_PID);
        if (busApp->connect() != I_CommunicationInterface::Error_t::OK)
            return {};
        if (CandleV2::enterBootloader(std::move(busApp)) != candleTypes::Error_t::OK)
            return {};
        std::cout << "REBOOTING\n";
        sleep(2);  // wait for reboot
        auto busBoot = std::make_unique<USBv2>(CandleBootloader::BOOTLOADER_VID,
                                               CandleBootloader::BOOTLOADER_PID);
        if (busBoot->connect() == I_CommunicationInterface::Error_t::OK)
            return std::make_unique<CandleBootloader>(std::move(busBoot));

        return {};
    }
}  // namespace mab