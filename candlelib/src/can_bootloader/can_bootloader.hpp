#pragma once

#include <array>
#include <cstring>
#include <string_view>
#include "candle_v2.hpp"
#include "logger.hpp"
#include "mab_def.hpp"
#include "mab_types.hpp"
#include "register.hpp"

namespace mab
{
    class CanBootloader
    {
        static constexpr std::string_view DEFAULT_REPONSE     = "OK";
        static constexpr size_t           CHUNK_SIZE          = 64;
        static constexpr u32              CHUNKS_PER_TRANSFER = 32;
        static constexpr u32              TRANSFER_SIZE       = CHUNK_SIZE * CHUNKS_PER_TRANSFER;

        enum class Command_t : u8
        {
            INIT = 0xB1,   // Get bootloader ready for transactions, decides what protocol is used
                           // for data transfer
            ERASE = 0xB2,  // Erase flash region
            PROG  = 0xB3,  // Initialize program datatransfer (possible encryption)
            WRITE = 0xB4,  // Submit data chunks sent earlier (is always at the end of 32 transfers
                           // of 64 bytes of data transfers)
            BOOT = 0xB5,   // Boot to app
            META = 0xB6    // Set metadata and save it to flash
        };

        const canId_t m_id;
        CandleV2*     mp_candle;
        Logger        m_log = Logger(Logger::ProgramLayer_E::LAYER_2, "CAN BOOTLOADER");

      public:
        enum Error_t
        {
            OK,
            NOT_CONNNECTED,
            DATA_TRANSFER_ERROR
        };

        CanBootloader(const canId_t id, CandleV2* candle);
        ~CanBootloader();

        Error_t init(const u32 bootAdress, const u32 appSize) const;
        Error_t erase(const u32 address, const u32 size) const;
        Error_t startTransfer(const bool                encrypted,
                              const std::array<u8, 16>& initializationVector) const;
        Error_t transferData(const std::array<u8, TRANSFER_SIZE>& data, const u32 crc32) const;
        Error_t transferMetadata(const bool save, const std::array<u8, 32>& firmwareSHA256) const;
        Error_t boot(const u32 bootAddress) const;

      private:
        Error_t sendCommand(const Command_t command, const std::vector<u8>& data) const;
        Error_t sendFrame(const std::vector<u8>& data) const;

        template <typename T>
        static constexpr std::array<u8, sizeof(T)> serializeData(T data)
        {
            std::array<u8, sizeof(T)> arr = {0};
            std::memcpy(arr.data(), &data, sizeof(T));
            return arr;
        }
    };
}  // namespace mab
