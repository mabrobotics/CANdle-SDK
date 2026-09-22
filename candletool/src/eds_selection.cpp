#include "eds_selection.hpp"

#include <algorithm>
#include <cctype>
#include <system_error>

#include "edsParser.hpp"
#include "mini/ini.h"

namespace mab
{
    namespace
    {
        /// @brief Tell a version argument like "1.2" apart from a path to an .eds file
        bool looksLikeVersion(const std::string& selection)
        {
            if (selection.empty())
                return false;

            bool digitSeen = false;
            for (const char c : selection)
            {
                if (std::isdigit(static_cast<unsigned char>(c)) != 0)
                {
                    digitSeen = true;
                    continue;
                }
                if (c != '.')
                    return false;
            }
            return digitSeen;
        }

        /// @brief Read the version an .eds declares in its [FileInfo] section
        std::string readEdsFileVersion(const std::filesystem::path& edsPath)
        {
            mINI::INIFile      edsFile(edsPath);
            mINI::INIStructure edsStruct;
            if (!edsFile.read(edsStruct))
                return {};

            return edsStruct["FileInfo"]["FileVersion"];
        }

        /// @brief Check that a file candletool is told to use really is an object dictionary
        ///
        /// A file the parser cannot make sense of is reported by logging and handing back an
        /// empty dictionary rather than by an error code, so the outcome is judged here
        bool isUsableEds(const std::filesystem::path& edsPath, const Logger& log)
        {
            mINI::INIFile      edsFile(edsPath);
            mINI::INIStructure edsStruct;
            if (!edsFile.read(edsStruct) || !edsStruct.has("DeviceInfo"))
            {
                log.error("%s is not an .eds file, keeping the current one", edsPath.c_str());
                return false;
            }

            const auto odPair = EDSParser::load(edsPath);
            if (odPair.second != EDSParser::Error_t::OK || odPair.first == nullptr ||
                odPair.first->size() == 0)
            {
                log.error("%s does not describe a single object, keeping the current one",
                          edsPath.c_str());
                return false;
            }

            return true;
        }

        void logBundledEds(const std::vector<BundledEds_S>& bundled,
                           const std::filesystem::path&     edsDir,
                           const Logger&                    log)
        {
            if (bundled.empty())
            {
                log.warn("no .eds files found in %s", edsDir.c_str());
                return;
            }

            log.info("Available .eds files:");
            for (const auto& eds : bundled)
                log.info("- %s (%s)",
                         eds.version.empty() ? "no version declared" : eds.version.c_str(),
                         eds.path.c_str());
        }

        /// @brief Turn the argument of the eds command into the path of an .eds file
        /// @return the path, nothing when the argument matches no usable file
        std::optional<std::filesystem::path> resolveEdsSelection(
            const std::string& selection, const std::filesystem::path& edsDir, const Logger& log)
        {
            std::error_code ec;

            if (!looksLikeVersion(selection))
            {
                const std::filesystem::path path(selection);
                if (!std::filesystem::is_regular_file(path, ec))
                {
                    log.error(
                        "%s is not a file - pass a path to an .eds file or the version of one of "
                        "the .eds files that come with candletool",
                        selection.c_str());
                    logBundledEds(listBundledEds(edsDir), edsDir, log);
                    return std::nullopt;
                }

                // The path is stored in candletool.ini and read back from wherever the next run
                // is started, so a relative one has to be resolved while the cwd still holds
                const auto absolutePath = std::filesystem::weakly_canonical(path, ec);
                return ec ? path : absolutePath;
            }

            const auto bundled = listBundledEds(edsDir);

            std::vector<BundledEds_S> matches;
            for (const auto& eds : bundled)
                if (eds.version == selection)
                    matches.push_back(eds);

            // An .eds without a [FileInfo] version is still selectable by the version in its name
            if (matches.empty())
                for (const auto& eds : bundled)
                    if (eds.path.stem().string().find(selection) != std::string::npos)
                        matches.push_back(eds);

            if (matches.empty())
            {
                log.error("none of the .eds files in %s is version %s",
                          edsDir.c_str(),
                          selection.c_str());
                logBundledEds(bundled, edsDir, log);
                return std::nullopt;
            }

            if (matches.size() > 1)
            {
                log.error(
                    "version %s matches more than one .eds - pass the path of the one you want "
                    "instead",
                    selection.c_str());
                logBundledEds(matches, edsDir, log);
                return std::nullopt;
            }

            return matches.front().path;
        }
    }  // namespace

    std::filesystem::path bundledEdsDir(const std::filesystem::path& configFilePath)
    {
        return configFilePath.parent_path() / "eds";
    }

    std::vector<BundledEds_S> listBundledEds(const std::filesystem::path& edsDir)
    {
        std::vector<BundledEds_S> bundled;

        std::error_code ec;
        if (!std::filesystem::is_directory(edsDir, ec))
            return bundled;

        for (const auto& entry : std::filesystem::directory_iterator(edsDir, ec))
        {
            if (!entry.is_regular_file(ec))
                continue;

            std::string extension = entry.path().extension().string();
            std::transform(extension.begin(),
                           extension.end(),
                           extension.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            if (extension != ".eds")
                continue;

            bundled.push_back(BundledEds_S{entry.path(), readEdsFileVersion(entry.path())});
        }

        std::sort(bundled.begin(),
                  bundled.end(),
                  [](const BundledEds_S& lhs, const BundledEds_S& rhs)
                  {
                      if (lhs.version != rhs.version)
                          return lhs.version < rhs.version;
                      return lhs.path < rhs.path;
                  });

        return bundled;
    }

    bool selectEds(const std::string&           selection,
                   const std::filesystem::path& configFilePath,
                   const Logger&                log)
    {
        if (!std::filesystem::exists(configFilePath))
        {
            log.error(
                "could not locate candletool.ini configuration file in %s. Is the candletool "
                "installed properly?",
                configFilePath.c_str());
            return false;
        }

        const auto edsPath = resolveEdsSelection(selection, bundledEdsDir(configFilePath), log);
        if (!edsPath.has_value())
            return false;

        // Only an .eds candletool can actually work with is worth replacing the current one with
        if (!isUsableEds(edsPath.value(), log))
            return false;

        mINI::INIFile      configFile(configFilePath);
        mINI::INIStructure configStruct;
        configFile.read(configStruct);

        configStruct["eds"]["path"] = edsPath.value().string();
        if (!configFile.write(configStruct))
        {
            log.error("could not write %s - do you have permission to edit it?",
                      configFilePath.c_str());
            return false;
        }

        log.success("candletool now describes the drive with %s", edsPath.value().c_str());
        return true;
    }

    void reportEdsSelection(const std::filesystem::path& configFilePath, const Logger& log)
    {
        if (!std::filesystem::exists(configFilePath))
        {
            log.error(
                "could not locate candletool.ini configuration file in %s. Is the candletool "
                "installed properly?",
                configFilePath.c_str());
            return;
        }

        mINI::INIFile      configFile(configFilePath);
        mINI::INIStructure configStruct;
        configFile.read(configStruct);

        const std::string current = configStruct["eds"]["path"];
        const std::string legacy  = configStruct["eds"]["legacy path"];

        if (current.empty())
            log.warn("no .eds is configured in %s", configFilePath.c_str());
        else
            log.info("Currently used .eds: %s%s",
                     current.c_str(),
                     std::filesystem::exists(current) ? "" : " (missing!)");

        if (!legacy.empty())
            log.info("Used for firmware older than %u.0.0: %s",
                     (unsigned)LEGACY_EDS_BELOW_FW_MAJOR,
                     legacy.c_str());

        const auto edsDir = bundledEdsDir(configFilePath);
        logBundledEds(listBundledEds(edsDir), edsDir, log);
    }
    std::optional<EdsPaths_S> readEdsPaths(const std::filesystem::path& configFilePath,
                                           const Logger&                log)
    {
        if (!std::filesystem::exists(configFilePath))
        {
            log.error(
                "could not locate candletool.ini configuration file in %s. Is the candletool "
                "installed properly?",
                configFilePath.c_str());
            return std::nullopt;
        }

        mINI::INIFile      configFile(configFilePath);
        mINI::INIStructure configStruct;
        configFile.read(configStruct);

        EdsPaths_S paths;
        paths.current = std::filesystem::path(configStruct["eds"]["path"]);
        if (paths.current.empty() || !std::filesystem::exists(paths.current))
        {
            log.error(
                "could not locate .eds file. Please check the %s file for eds section and fill it "
                "properly. Currently read path is: %s",
                configFilePath.c_str(),
                paths.current.c_str());
            return std::nullopt;
        }

        // Older drives use a different object dictionary layout. An installation that configures
        // only one .eds keeps using it for every drive, which is how candletool behaved before
        paths.legacy = std::filesystem::path(configStruct["eds"]["legacy path"]);
        if (!paths.legacy.empty() && !std::filesystem::exists(paths.legacy))
        {
            log.warn(
                "legacy .eds configured in %s does not exist: %s - drives with firmware older "
                "than %u.0.0 will not be described correctly",
                configFilePath.c_str(),
                paths.legacy.c_str(),
                (unsigned)LEGACY_EDS_BELOW_FW_MAJOR);
            paths.legacy.clear();
        }

        return paths;
    }

    void useEdsMatchingFirmware(MDCO&                                md,
                                std::shared_ptr<EDSObjectDictionary> od,
                                const std::filesystem::path&         legacyEdsPath,
                                const Logger&                        log)
    {
        if (legacyEdsPath.empty() || od == nullptr)
            return;

        const auto [firmwareVersion, err] = md.getFirmwareVersion();

        const bool legacyDrive =
            err != MDCO::Error_t::OK || firmwareVersion.s.major < LEGACY_EDS_BELOW_FW_MAJOR;

        if (!legacyDrive)
        {
            log.debug("Drive firmware is %u.%u.%u, keeping the default .eds",
                      (unsigned)firmwareVersion.s.major,
                      (unsigned)firmwareVersion.s.minor,
                      (unsigned)firmwareVersion.s.revision);
            return;
        }

        if (err == MDCO::Error_t::OK)
            log.info("Drive firmware is %u.%u.%u, loading the .eds for firmware older than %u.0.0",
                     (unsigned)firmwareVersion.s.major,
                     (unsigned)firmwareVersion.s.minor,
                     (unsigned)firmwareVersion.s.revision,
                     (unsigned)LEGACY_EDS_BELOW_FW_MAJOR);
        else
            log.info(
                "Drive did not report its firmware version, assuming it predates %u.0.0 and "
                "loading the legacy .eds",
                (unsigned)LEGACY_EDS_BELOW_FW_MAJOR);

        auto legacyPair = EDSParser::load(legacyEdsPath);
        if (legacyPair.second != EDSParser::Error_t::OK || legacyPair.first == nullptr)
        {
            log.error("Could not load the legacy .eds from %s, keeping the default one",
                      legacyEdsPath.c_str());
            return;
        }

        *od = std::move(*legacyPair.first);
    }
}  // namespace mab
