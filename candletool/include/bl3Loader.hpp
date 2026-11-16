#pragma once

#include "candle.hpp"
#include "mabFileParser.hpp"
#include "mab_types.hpp"

namespace mab
{
    /// @brief Firmware update of MD drives on the G5 bootloader (Bl3), CAN update protocol v1
    ///
    /// Every request gets one reply [command, status, ...]:
    ///   [C0]                     IDENTIFY -> [C0, 0, protocol, deviceType, deviceRevision,
    ///                                         blVersion u32, maxImageSize u32]
    ///   [C1, header 56 B]        START    -> [C1, status]
    ///   [C2, seq, n, n <= 61 B]  DATA     -> [C2, status, seq]
    ///   [C3]                     END      -> [C3, status]
    ///   [C4]                     BOOT     -> [C4, status]
    /// A lost reply is recovered by sending the same request again: a repeated DATA seq gets the
    /// stored reply and is not written twice, END and BOOT can be repeated.
    namespace bl3Loader
    {
        constexpr u8 PROTOCOL = 1;  // protocol version this loader speaks

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
            G5,        // Bl3 answered, the update session is open
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
    }  // namespace bl3Loader
}  // namespace mab
