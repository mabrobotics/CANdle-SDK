#pragma once

#include <filesystem>
#include <memory>
#include <optional>

#include "MDCO.hpp"
#include "edsEntry.hpp"
#include "logger.hpp"
#include "mab_types.hpp"

namespace mab
{
    /// @brief Drives with firmware older than this major version are described by the legacy .eds
    inline constexpr u8 LEGACY_EDS_BELOW_FW_MAJOR = 3;

    /// @brief The .eds files an installation provides
    struct EdsPaths_S
    {
        /// @brief dictionary of drives running current firmware
        std::filesystem::path current;
        /// @brief dictionary of drives older than LEGACY_EDS_BELOW_FW_MAJOR, empty when the
        /// installation configures only one .eds
        std::filesystem::path legacy;
    };

    /// @brief Read the .eds paths out of candletool.ini
    /// @param configFilePath path of the candletool.ini to read
    /// @param log logger used to report a missing or unusable configuration
    /// @return the configured paths, empty when the current .eds could not be located
    std::optional<EdsPaths_S> readEdsPaths(const std::filesystem::path& configFilePath,
                                           const Logger&                log);

    /// @brief Replace the loaded dictionary with the legacy one when the drive firmware predates
    /// the layout the current .eds describes
    ///
    /// The drive is asked for its firmware version, which the resolver finds in either revision.
    /// A drive that does not answer at all predates the current layout, which leads to the same
    /// conclusion as reading a version below the threshold. The dictionary is replaced in place,
    /// so every holder of the shared pointer keeps working and sees the legacy layout.
    ///
    /// @param md drive to ask for its firmware version
    /// @param od dictionary to replace in place
    /// @param legacyEdsPath .eds to load for an older drive, nothing happens when it is empty
    /// @param log logger used to report which dictionary is in use
    void useEdsMatchingFirmware(MDCO&                                md,
                                std::shared_ptr<EDSObjectDictionary> od,
                                const std::filesystem::path&         legacyEdsPath,
                                const Logger&                        log);
}  // namespace mab
