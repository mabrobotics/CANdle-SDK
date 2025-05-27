#pragma once
#include <memory>
#include <utility>
#include <vector>

#include "I_communication_interface.hpp"
#include "USB.hpp"
#include "logger.hpp"
#include "mab_types.hpp"
#include "candle_types.hpp"
#include "candle.hpp"
namespace mab
{
    /// @brief Representation of CANdle USB bootloader
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

        /// @brief Command to boot forward, handle no longer valid at this point
        /// @return Error on failure
        candleTypes::Error_t enterAppFromBootloader();

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
        /// @brief Initialization method, should be called after the construction of the object
        /// @return Error on failure
        candleTypes::Error_t init();

        /// @brief Write a consecutive page to the bootloader memory. If write fails it is the best
        /// to reconstruct the object as a the state of a memory load inside CANdle is undefined.
        /// @param page filled flash page to be written
        /// @param crc32 checksum from the candle implementation TODO: this will be unified in CS-40
        /// @return Error on failure
        candleTypes::Error_t writePage(const std::array<u8, PAGE_SIZE_STM32G474> page,
                                       const u32                                 crc32) const;
    };

    /// @brief Helper function to initialise CandleBootloader object
    /// @return Initialised CandleBootloader or nullptr on failure
    inline std::unique_ptr<CandleBootloader> attachCandleBootloader()
    {
        Logger log(Logger::ProgramLayer_E::TOP, "BOOTLOADER_PRELOADER");
        log.info("Looking for CANdle...");

        // Detecting candle app and enter bootloader command if present
        auto busApp = std::make_unique<USB>(Candle::CANDLE_VID, Candle::CANDLE_PID);
        if (busApp->connect() == I_CommunicationInterface::Error_t::OK)
        {
            log.info("Found! Rebooting to bootloader...");
            if (Candle::enterBootloader(std::move(busApp)) != candleTypes::Error_t::OK)
            {
                log.error("Failed to communicate with candle");
                return {};
            }
            usleep(1000000);  // wait for reboot TODO: maybe change to something more robust
        }
        else
        {
            log.warn("Candle may not have an app");
        }

        // Bootloader connection
        log.info("Looking for bootloader...");
        busApp = nullptr;  // necessary because context exit in libusb destroys ALL of the contexts
                           // for some reason! So no implementation can live while the
                           // libusb_exit(context) is called.

        auto busBoot = std::make_unique<USB>(CandleBootloader::BOOTLOADER_VID,
                                             CandleBootloader::BOOTLOADER_PID);
        if (busBoot->connect() == I_CommunicationInterface::Error_t::OK)
        {
            log.info("Found!");
            auto cb = std::make_unique<CandleBootloader>(std::move(busBoot));
            cb->init();
            return cb;
        }
        log.error("Bootloader not found!");
        return {};
    }
}  // namespace mab