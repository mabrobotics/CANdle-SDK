#pragma once

#include "candle.hpp"
#include "mabFileParser.hpp"
#include "mab_types.hpp"

namespace mab
{
    /// @brief Firmware update over CAN of devices on the STM32G4 bootloader (Bl2): MD G4 and PDS
    ///
    /// Every frame is answered with "OK" or "NO". The candle stays owned by the caller.
    namespace bl2Loader
    {
        /// @brief Send the firmware of the .mab file and start it
        /// @param recovery keep calling the bootloader until it answers, for a drive that is
        /// being power cycled
        bool flashAndBoot(Candle*              candle,
                          canId_t              canId,
                          const MabFileParser& mabFile,
                          bool                 recovery);

        /// @brief Erase the configuration (both locations used over time) and start the
        /// application; waits until the bootloader answers
        bool eraseConfig(Candle* candle, canId_t canId);
    }  // namespace bl2Loader
}  // namespace mab
