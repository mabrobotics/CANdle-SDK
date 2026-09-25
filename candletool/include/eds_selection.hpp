#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "MDCO.hpp"
#include "edsEntry.hpp"
#include "logger.hpp"
#include "mab_types.hpp"

namespace mab
{
    /// @brief Drives with firmware older than this major version are described by the legacy .eds
    inline constexpr u8 LEGACY_EDS_BELOW_FW_MAJOR = 3;

    /// @brief Bundled .eds describing drives with firmware LEGACY_EDS_BELOW_FW_MAJOR and newer
    inline constexpr const char* STANDARD_EDS_FILE = "md_1.2.eds";

    /// @brief The .eds files an installation provides
    struct EdsPaths_S
    {
        /// @brief dictionary of drives running current firmware
        std::filesystem::path current;
        /// @brief dictionary of drives older than LEGACY_EDS_BELOW_FW_MAJOR, empty when the
        /// installation configures only one .eds
        std::filesystem::path legacy;
        /// @brief bundled STANDARD_EDS_FILE, empty when the installation does not ship it
        std::filesystem::path standard;
    };

    /// @brief An .eds file shipped with the installation
    struct BundledEds_S
    {
        /// @brief full path of the .eds
        std::filesystem::path path;
        /// @brief version the .eds declares as FileVersion in its [FileInfo] section, empty when
        /// it declares none
        std::string version;
    };

    /// @brief Directory the installation keeps its .eds files in
    /// @param configFilePath path of the candletool.ini, the .eds files sit next to it
    std::filesystem::path bundledEdsDir(const std::filesystem::path& configFilePath);

    /// @brief List the .eds files the installation ships, ordered by the version they declare
    /// @param edsDir directory to look in, an unreadable one yields an empty list
    std::vector<BundledEds_S> listBundledEds(const std::filesystem::path& edsDir);

    /// @brief Point candletool.ini at another .eds so the following runs use it
    ///
    /// The selection is either a version of one of the bundled .eds files ("1.2") or a path to
    /// any .eds. The file is parsed before it is written to the configuration, so a selection
    /// candletool could not work with never replaces a working one.
    ///
    /// @param selection version of a bundled .eds or path to an .eds file
    /// @param configFilePath path of the candletool.ini to update
    /// @param log logger used to report what was selected or why it was not
    /// @return true when candletool.ini now points at the selected .eds
    bool selectEds(const std::string&           selection,
                   const std::filesystem::path& configFilePath,
                   const Logger&                log);

    /// @brief Report which .eds candletool.ini points at and which ones are available
    /// @param configFilePath path of the candletool.ini to read
    /// @param log logger used to report the selection
    void reportEdsSelection(const std::filesystem::path& configFilePath, const Logger& log);

    /// @brief Read the .eds paths out of candletool.ini
    /// @param configFilePath path of the candletool.ini to read
    /// @param log logger used to report a missing or unusable configuration
    /// @return the configured paths, empty when the current .eds could not be located
    std::optional<EdsPaths_S> readEdsPaths(const std::filesystem::path& configFilePath,
                                           const Logger&                log);

    /// @brief Replace the loaded dictionary with the one matching the drive firmware
    ///
    /// The drive is asked for its firmware version through the loaded dictionary. When that
    /// dictionary does not describe the drive (md_1.1 on a 3.x drive) the version is asked for
    /// again through the standard .eds, which is then kept. A drive that does not answer at all
    /// predates the current layout, which leads to the same conclusion as reading a version below
    /// the threshold. The dictionary is replaced in place, so every holder of the shared pointer
    /// keeps working and sees the new layout. Every automatic change is reported to the user, as
    /// is a non-standard .eds used with a drive the standard one describes.
    ///
    /// @param md drive to ask for its firmware version
    /// @param od dictionary loaded from paths.current, replaced in place
    /// @param paths .eds files the installation provides
    /// @param log logger used to report which dictionary is in use
    void useEdsMatchingFirmware(MDCO&                                md,
                                std::shared_ptr<EDSObjectDictionary> od,
                                const EdsPaths_S&                    paths,
                                const Logger&                        log);
}  // namespace mab
