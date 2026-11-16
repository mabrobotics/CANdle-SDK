#include "bl2Loader.hpp"

#include "mab_crc.hpp"

#include <cstring>
#include <unistd.h>
#include <vector>

namespace mab
{
    namespace
    {
        enum Command : u8
        {
            INIT  = 0xB1,  // [address, size] starts a session
            ERASE = 0xB2,  // [address, size] flash region
            PROG  = 0xB3,  // [encrypted, iv] starts the transfer
            WRITE = 0xB4,  // [crc32] writes the TRANSFER_SIZE bytes sent before it
            BOOT  = 0xB5,  // [address] starts the application
            META  = 0xB6,  // [save, sha256] saves the checksum of the firmware
        };

        constexpr u32 PAGE_SIZE     = 2048;  // STM32G4 flash page
        constexpr u32 CHUNK_SIZE    = 64;    // data bytes per frame
        constexpr u32 TRANSFER_SIZE = 2048;  // data bytes between WRITEs
        constexpr u32 TIMEOUT_MS    = 2000;  // the CANdle takes it as u8, so it waits 208 ms

        // Configuration locations used by eraseConfig()
        constexpr u32 FLASH_START           = 0x0800'0000;
        constexpr u32 CONFIG_ADDRESS        = 0x0801'B000;
        constexpr u32 LEGACY_CONFIG_ADDRESS = 0x0801'B800;

        Logger log(Logger::ProgramLayer_E::LAYER_2, "BL2 LOADER");

        /// @return true when the bootloader answered "OK"
        bool sendFrame(Candle* candle, canId_t canId, const std::vector<u8>& frame)
        {
            auto [reply, err] = candle->transferCANFrame(canId, frame, 2, TIMEOUT_MS);
            if (err != candleTypes::Error_t::OK)
                return false;
            for (size_t i = 0; i + 1 < reply.size(); i++)
                if (reply[i] == 'O' && reply[i + 1] == 'K')
                    return true;
            return false;
        }

        void put32(std::vector<u8>& frame, u32 value)
        {
            u8 bytes[4];
            memcpy(bytes, &value, sizeof(bytes));
            frame.insert(frame.end(), bytes, bytes + sizeof(bytes));
        }

        bool command(Candle* candle, canId_t canId, Command cmd, u32 a, u32 b)
        {
            std::vector<u8> frame = {cmd};
            put32(frame, a);
            put32(frame, b);
            return sendFrame(candle, canId, frame);
        }

        bool boot(Candle* candle, canId_t canId, u32 address)
        {
            std::vector<u8> frame = {BOOT};
            put32(frame, address);
            return sendFrame(candle, canId, frame);
        }

        /// @brief Page by page: the CANdle does not wait long enough for a larger erase
        bool erase(Candle* candle, canId_t canId, u32 address, u32 size)
        {
            const u32 fullPages = size / PAGE_SIZE;
            for (u32 i = 0; i < fullPages; i++)
            {
                if (!command(candle, canId, ERASE, address + i * PAGE_SIZE, PAGE_SIZE))
                {
                    log.error("Failed to erase page %u", i);
                    return false;
                }
            }
            return command(candle,
                           canId,
                           ERASE,
                           address + fullPages * PAGE_SIZE,
                           size - fullPages * PAGE_SIZE);
        }

        /// @brief TRANSFER_SIZE bytes as raw data frames, then WRITE with their CRC
        bool transferData(Candle* candle, canId_t canId, const u8* data)
        {
            for (u32 i = 0; i < TRANSFER_SIZE; i += CHUNK_SIZE)
                if (!sendFrame(candle, canId, std::vector<u8>(data + i, data + i + CHUNK_SIZE)))
                    return false;
            std::vector<u8> frame = {WRITE};
            put32(frame, crc32(data, TRANSFER_SIZE));
            return sendFrame(candle, canId, frame);
        }

        /// @brief Saving the metadata takes longer than the CANdle waits, so its reply comes late:
        /// an empty frame picks it up. The reply to that frame is left on the bus, which is why
        /// BOOT is sent twice afterwards.
        bool transferMetadata(Candle* candle, canId_t canId, const u8 sha256[32])
        {
            std::vector<u8> frame = {META, 1 /* save */};
            frame.insert(frame.end(), sha256, sha256 + 32);
            frame.resize(CHUNK_SIZE);
            if (sendFrame(candle, canId, frame))
                return true;
            return sendFrame(candle, canId, {});
        }
    }  // namespace

    bool bl2Loader::flashAndBoot(Candle*              candle,
                                 canId_t              canId,
                                 const MabFileParser& mabFile,
                                 bool                 recovery)
    {
        const MabFileParser::FirmwareEntry& fw = mabFile.m_fwEntry;

        if (recovery)
            log.info("Waiting for the bootloader...");
        while (!command(candle, canId, INIT, fw.bootAddress, fw.size))
        {
            if (!recovery)
            {
                log.error("Failed to initialize bootloader");
                return false;
            }
        }
        log.info("Bootloader Connected!");
        sleep(1);

        if (!erase(candle, canId, fw.bootAddress, fw.size))
        {
            log.error("Failed to erase memory");
            return false;
        }

        std::vector<u8> prog = {PROG, 1 /* encrypted */};
        prog.insert(prog.end(), fw.aes_iv, fw.aes_iv + sizeof(fw.aes_iv));
        if (!sendFrame(candle, canId, prog))
        {
            log.error("Failed to start transfer");
            return false;
        }

        // Whole transfers, the last one padded from the zeroed rest of the buffer
        const u8* data = fw.data->data();
        for (u32 i = 0; i <= fw.size; i += TRANSFER_SIZE)
        {
            log.progress((double)i / fw.size);
            if (!transferData(candle, canId, data + i))
            {
                log.error("Failed to transfer firmware");
                return false;
            }
        }
        log.progress(1.0);

        if (!command(candle, canId, INIT, fw.bootAddress, fw.size) ||
            !transferMetadata(candle, canId, fw.checksum))
        {
            log.error("Failed to transfer metadata");
            return false;
        }

        boot(candle, canId, fw.bootAddress);  // takes the reply left by transferMetadata()
        if (!boot(candle, canId, fw.bootAddress))
        {
            log.error("Failed to boot");
            return false;
        }
        log.success("Flashing sucessful!");
        return true;
    }

    bool bl2Loader::eraseConfig(Candle* candle, canId_t canId)
    {
        while (!command(candle, canId, INIT, FLASH_START, 0))
            usleep(250'000);
        log.info("Bootloader Connected!");
        sleep(1);

        if (!erase(candle, canId, LEGACY_CONFIG_ADDRESS, 1))
        {
            log.error("Failed to erase legacy config!");
            return false;
        }
        if (!erase(candle, canId, CONFIG_ADDRESS, 1))
        {
            log.error("Failed to erase config!");
            return false;
        }

        boot(candle, canId, FLASH_START);
        if (!boot(candle, canId, FLASH_START))
        {
            log.error("Failed to boot");
            return false;
        }
        return true;
    }
}  // namespace mab
