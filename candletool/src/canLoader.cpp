#include "canLoader.hpp"
#include "can_bootloader.hpp"
#include "candle.hpp"
#include "mabFileParser.hpp"
#include "mab_crc.hpp"

#include <array>
#include <cmath>
#include <cstring>
#include <string>

namespace mab
{

    CanLoader::CanLoader(mab::Candle* candle, MabFileParser* mabFile, mab::canId_t canId)
        : m_mabFile(mabFile), m_candle(candle), m_canId(canId)
    {
        m_log.m_tag   = "CAN LOADER";
        m_log.m_layer = Logger::ProgramLayer_E::LAYER_2;
    }

    CanLoader::~CanLoader()
    {
    }

    bool CanLoader::flashAndBoot(bool recovery)
    {
        if (m_candle == nullptr)
        {
            m_log.error("Candle not initialized");
            return false;
        }
        if (m_mabFile == nullptr)
        {
            m_log.error("MAB file not initialized");
            return false;
        }

        CanBootloader                 bootloader     = mab::CanBootloader(m_canId, m_candle);
        const u32                     appSize        = m_mabFile->m_fwEntry.size;
        const u32                     swAddress      = m_mabFile->m_fwEntry.bootAddress;
        const std::span<u8>           firmware       = *(m_mabFile->m_fwEntry.data);
        const std::span<const u8, 32> firmwareSHA256 = (m_mabFile->m_fwEntry.checksum);

        while (true)
        {
            if (bootloader.init(swAddress, appSize) == CanBootloader::Error_t::OK)
                break;
            if (!recovery)
            {
                m_log.error("Failed to initialize bootloader");
                return false;
            }
            usleep(250'000);
        }

        m_log.info("Bootloader Connected!");
        sleep(1);

        // Clearing memory
        if (bootloader.erase(swAddress, appSize) != CanBootloader::Error_t::OK)
        {
            m_log.error("Failed to erase memory");
            return false;
        }

        // Enter "transfer mode"
        std::array<u8, 16> iv = std::to_array(m_mabFile->m_fwEntry.aes_iv);
        if (bootloader.startTransfer(!iv.empty(), iv) != CanBootloader::Error_t::OK)
        {
            m_log.error("Failed to start transfer");
            return false;
        }

        // Transfering fw
        for (size_t i = 0; i <= appSize; i += CanBootloader::TRANSFER_SIZE)
        {
            m_log.progress(((float)i) / ((float)appSize));

            std::span<u8, CanBootloader::TRANSFER_SIZE> chunk =
                firmware.subspan(i)
                    .first<CanBootloader::TRANSFER_SIZE>();  // Take a chunk of data from firmware
            const u32 crc = crc32(chunk.data(), chunk.size());
            if (bootloader.transferData(chunk, crc) != CanBootloader::Error_t::OK)
            {
                m_log.error("Failed to transfer firmware");
                return false;
            }
        }
        m_log.progress(1.0);  // Transfering finish indication

        // Transfering metadata about firmware
        if (bootloader.init(swAddress, appSize) != CanBootloader::Error_t::OK ||
            bootloader.transferMetadata(true, firmwareSHA256) != CanBootloader::Error_t::OK)
        {
            m_log.error("Failed to transfer metadata");
            return false;
        }

        // Send boot cmd
        bootloader.boot(swAddress);
        if (bootloader.boot(swAddress) != CanBootloader::Error_t::OK)
        {
            m_log.error("Failed to boot");
            return false;
        }
        m_log.success("Flashing sucessful!");

        return true;
    }
    bool CanLoader::forceEraseConfig()
    {
        if (m_candle == nullptr)
        {
            m_log.error("Candle not initialized");
            return false;
        }
        CanBootloader bootloader          = mab::CanBootloader(m_canId, m_candle);
        const u32     appSize             = 0;
        const u32     swAddress           = 0x800'0000;
        const u32     legacyConfigAddress = 0x801'B800;
        const u32     configAddress       = 0x801'B000;

        while (true)
        {
            if (bootloader.init(swAddress, appSize) == CanBootloader::Error_t::OK)
                break;
            usleep(250'000);
        }

        m_log.info("Bootloader Connected!");
        sleep(1);

        // Clearing memory
        if (bootloader.erase(legacyConfigAddress, 1) != CanBootloader::Error_t::OK)
        {
            m_log.error("Failed to erase legacy config!");
            return false;
        }
        if (bootloader.erase(configAddress, 1) != CanBootloader::Error_t::OK)
        {
            m_log.error("Failed to erase config!");
            return false;
        }

        // Send boot cmd
        bootloader.boot(swAddress);
        if (bootloader.boot(swAddress) != CanBootloader::Error_t::OK)
        {
            m_log.error("Failed to boot");
            return false;
        }
        return true;
    }
}  // namespace mab
