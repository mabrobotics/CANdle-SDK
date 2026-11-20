#pragma once

#include "candle.hpp"
#include "mabFileParser.hpp"
#include "mab_types.hpp"

namespace mab
{
    namespace bl3Loader
    {
        constexpr u8  PROTOCOL     = 1;       // protocol version this loader speaks
        constexpr u32 STORAGE_SIZE = 0x8000;  // Storage pages of the drive (config), MDG5_STORAGE_SIZE

        struct Info
        {
            u8  protocol;
            u8  deviceType;      // OTP manufacturer data, 0xFF when blank
            u8  deviceRevision;  // OTP, 0xFF when blank
            u32 blVersion;       // mab::version_ut
            u32 maxImageSize;    // bytes
        };

        enum class Identify_E
        {
            BL3,       // Bl3 answered, the update session is open
            OTHER,     // a G4 bootloader answered ("NO")
            NO_REPLY,  // nothing usable came back
        };

        enum class Result_E
        {
            OK,
            FAILED,
            BOOT_UNCONFIRMED,  // image written and checked, the BOOT reply did not come
        };

        /// @brief Ask the bootloader who it is, also opens the Bl3 update session
        /// @param attempts how many times IDENTIFY is sent while nothing answers, 0 = until an
        /// answer comes
        /// @param info filled when the result is G5
        Identify_E identify(Candle* candle, canId_t canId, u32 attempts, Info* info);

        /// @brief Send the firmware of the .mab file and start it. identify() must have returned
        /// G5 before.
        Result_E flashAndBoot(Candle*              candle,
                              canId_t              canId,
                              const MabFileParser& mabFile,
                              const Info&          info);

        /// @brief Erase the config page (CAN ID back to 100 at the next start) and start the
        /// application. identify() must have returned G5 before.
        bool eraseConfig(Candle* candle, canId_t canId);

        /// @brief Read the Storage pages (config of the application and the bootloader), e.g. to
        /// keep the config before the flash is erased. identify() must have returned BL3 before.
        /// @param startApp start the application afterwards (BOOT)
        bool readStorage(Candle* candle, canId_t canId, std::vector<u8>* out, bool startApp);
    }  // namespace bl3Loader
}  // namespace mab
