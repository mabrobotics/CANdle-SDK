#pragma once

#include "candle.hpp"
#include "mabFileParser.hpp"
#include "mab_types.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

/// @brief Firmware update of MD drives. Each function does one thing, the caller (CLI) decides
/// which one from its arguments.
namespace mab::mdUpdate
{
    /// @brief What the drive runs now, decides how it is put into its bootloader
    enum class Drive_E
    {
        MD,           // MD protocol firmware: version read + confirm, reset over MD protocol
        MDCO,         // CANopen firmware: version read + confirm, reset over CANopen (CAN 2.0)
        MDCO_LEGACY,  // CANopen firmware < 3.0.0: warning + confirm, reset over CANopen, version
                      // not read
        BOOTLOADER,   // already in the bootloader (recovery): no reset, waits until it answers
    };

    struct Target
    {
        canId_t                              canId;
        Drive_E                              drive;
        std::shared_ptr<const CandleBuilder> candleBuilder;  // Candle& once the frame format can
                                                             // be switched at runtime
        std::filesystem::path etcPath;  // package etc, candletool.ini points at the .eds (MDCO)
    };

    /// @brief Parse "X.Y.Z" with optional suffix (e.g. "3.0.0_3b52568")
    bool parseVersion(const std::string& str, version_ut* version);

    /// @brief Flash a local .mab file: checks the file against the drive (G4/G5, MDCO >= 3.0.1),
    /// confirms with the user, resets the drive and flashes it with bl2Loader / bl3Loader
    bool flash(const Target& target, const std::filesystem::path& mabFile);

    /// @brief Download a .mab file from the firmware server
    /// @param version "latest" or "X.Y.Z", 3.0.0 or newer
    /// @return local path of the file
    std::optional<std::filesystem::path> download(const std::string&       version,
                                                  MabFileParser::Variant_E variant);

    /// @brief Firmware older than 3.0.0: downloads and runs its flasher executable (Linux only)
    bool flashLegacy(const Target& target, const std::string& version);

    /// @brief Wipe the configuration, CAN ID back to 100
    bool eraseConfig(const Target& target);
}  // namespace mab::mdUpdate
