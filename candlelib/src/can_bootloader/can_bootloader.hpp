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
        static constexpr std::string_view DEFAULT_REPONSE = "OK";

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

        Error_t init(const u32 bootAdress, const u32 appSize);
        Error_t erase(const u32 address, const u32 size);
        Error_t startTransfer(const bool encrypted, const std::array<u8, 16>& initializationVector);
        Error_t transferData(const std::vector<u8>& data);
        Error_t transferMetadata(bool save, std::array<u8, 32>& firmwareSHA256);
        Error_t boot(const u32 bootAddress);

      private:
        Error_t sendCommand(const Command_t command, const std::vector<u8>& data);
        Error_t sendFrame(const std::vector<u8>& data);

        template <typename T>
        constexpr std::array<u8, sizeof(T)> serializeData(T data)
        {
            std::array<u8, sizeof(T)> arr = {0};
            std::memcpy(arr.data(), &data, sizeof(T));
            return arr;
        }
    };
}  // namespace mab
