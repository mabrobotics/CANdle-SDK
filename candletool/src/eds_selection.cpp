#include "eds_selection.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <system_error>
#include <tuple>

#include "curl_handler.hpp"
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

        /// @brief Read the version and revision an .eds declares
        BundledEds_S readEdsInfo(const std::filesystem::path& edsPath)
        {
            BundledEds_S       info{edsPath, {}, 0};
            mINI::INIFile      edsFile(edsPath);
            mINI::INIStructure edsStruct;
            if (!edsFile.read(edsStruct))
                return info;

            info.version = edsStruct["FileInfo"]["FileVersion"];
            // Written in hex (0x00010003) by the firmware tooling, base 0 accepts decimal as well
            info.revision =
                (u32)std::strtoul(edsStruct["DeviceInfo"]["RevisionNumber"].c_str(), nullptr, 0);
            return info;
        }

        u16 revisionMajor(u32 revision)
        {
            return (u16)(revision >> 16);
        }

        u16 revisionMinor(u32 revision)
        {
            return (u16)(revision & 0xFFFF);
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

        /// @brief Ask the drive for its firmware version, a version of 0.0.0 counts as no answer
        ///
        /// A dictionary that does not describe the drive (md_1.1 on a 3.x drive) may point at an
        /// object the drive does serve but which holds 0 rather than the version
        std::pair<version_ut, MDCO::Error_t> readFirmwareVersion(MDCO& md)
        {
            auto result = md.getFirmwareVersion();
            if (result.second == MDCO::Error_t::OK && result.first.s.major == 0 &&
                result.first.s.minor == 0 && result.first.s.revision == 0)
                result.second = MDCO::Error_t::REQUEST_INVALID;
            return result;
        }
    }  // namespace

    std::filesystem::path bundledEdsDir(const std::filesystem::path& configFilePath)
    {
        return configFilePath.parent_path() / "eds";
    }

    std::filesystem::path edsCacheDir()
    {
#ifdef _WIN32
        const char* base = std::getenv("LOCALAPPDATA");
        if (base != nullptr && *base != '\0')
            return std::filesystem::path(base) / "candletool" / "eds";
#else
        const char* base = std::getenv("XDG_CACHE_HOME");
        if (base != nullptr && *base != '\0')
            return std::filesystem::path(base) / "candletool" / "eds";
        const char* home = std::getenv("HOME");
        if (home != nullptr && *home != '\0')
            return std::filesystem::path(home) / ".cache" / "candletool" / "eds";
#endif
        std::error_code ec;
        return std::filesystem::temp_directory_path(ec) / "candletool" / "eds";
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

            bundled.push_back(readEdsInfo(entry.path()));
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
            log.info("Used only to update firmware older than %u.0.0: %s",
                     (unsigned)LEGACY_EDS_BELOW_FW_MAJOR,
                     legacy.c_str());

        const auto edsDir = bundledEdsDir(configFilePath);
        logBundledEds(listBundledEds(edsDir), edsDir, log);

        const auto cached = listBundledEds(edsCacheDir());
        if (!cached.empty())
        {
            log.info("Downloaded for newer drives:");
            for (const auto& eds : cached)
                log.info("- %u.%u (%s)",
                         (unsigned)revisionMajor(eds.revision),
                         (unsigned)revisionMinor(eds.revision),
                         eds.path.c_str());
        }
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

        // Older drives use a different object dictionary layout, which is only needed to put them
        // into the bootloader for a firmware update
        paths.legacy = std::filesystem::path(configStruct["eds"]["legacy path"]);
        if (!paths.legacy.empty() && !std::filesystem::exists(paths.legacy))
        {
            log.warn(
                "legacy .eds configured in %s does not exist: %s - drives with firmware older "
                "than %u.0.0 can not be updated over CANopen",
                configFilePath.c_str(),
                paths.legacy.c_str(),
                (unsigned)LEGACY_EDS_BELOW_FW_MAJOR);
            paths.legacy.clear();
        }

        paths.bundledDir = bundledEdsDir(configFilePath);
        paths.cacheDir   = edsCacheDir();

        paths.standard = paths.bundledDir / STANDARD_EDS_FILE;
        if (!std::filesystem::exists(paths.standard))
            paths.standard.clear();

        return paths;
    }

    /// @brief Pick the dictionary by firmware version, for drives that report no interface
    /// revision (0x1018:03 = 0, firmware 3.0.0 and older)
    static bool useEdsMatchingFirmwareVersion(MDCO&                                md,
                                              std::shared_ptr<EDSObjectDictionary> od,
                                              const EdsPaths_S&                    paths,
                                              const Logger&                        log,
                                              bool                                 allowLegacy)
    {
        std::error_code ec;
        const bool usingStandard = !paths.standard.empty() &&
                                   std::filesystem::equivalent(paths.current, paths.standard, ec);

        version_ut    firmwareVersion;
        MDCO::Error_t err;
        std::tie(firmwareVersion, err) = readFirmwareVersion(md);

        // The loaded .eds may not describe where a newer drive keeps its version (md_1.1 on a
        // 3.x drive), so the drive is asked again through the standard one
        if (err != MDCO::Error_t::OK && !usingStandard && !paths.standard.empty())
        {
            auto standardPair = EDSParser::load(paths.standard);
            if (standardPair.first != nullptr)
            {
                EDSObjectDictionary selected   = std::move(*od);
                *od                            = std::move(*standardPair.first);
                std::tie(firmwareVersion, err) = readFirmwareVersion(md);
                if (err == MDCO::Error_t::OK &&
                    firmwareVersion.s.major >= LEGACY_EDS_BELOW_FW_MAJOR)
                {
                    log.warn(
                        "Drive firmware is %u.%u.%u, which the selected %s does not describe - "
                        "the .eds was automatically changed to %s",
                        (unsigned)firmwareVersion.s.major,
                        (unsigned)firmwareVersion.s.minor,
                        (unsigned)firmwareVersion.s.revision,
                        paths.current.filename().string().c_str(),
                        paths.standard.filename().string().c_str());
                    log.warn("To select it permanently use: candletool mdco eds %s",
                             paths.standard.string().c_str());
                    return true;
                }
                *od = std::move(selected);
            }
        }

        const bool legacyDrive =
            err != MDCO::Error_t::OK || firmwareVersion.s.major < LEGACY_EDS_BELOW_FW_MAJOR;

        if (!legacyDrive)
        {
            if (!usingStandard && !paths.standard.empty())
                log.warn(
                    "Drive firmware is %u.%u.%u and the non-standard %s is in use - the standard "
                    ".eds for firmware %u.0.0 and newer is %s. Make sure the selected .eds "
                    "matches the drive firmware!",
                    (unsigned)firmwareVersion.s.major,
                    (unsigned)firmwareVersion.s.minor,
                    (unsigned)firmwareVersion.s.revision,
                    paths.current.string().c_str(),
                    (unsigned)LEGACY_EDS_BELOW_FW_MAJOR,
                    STANDARD_EDS_FILE);
            else
                log.debug("Drive firmware is %u.%u.%u, keeping the default .eds",
                          (unsigned)firmwareVersion.s.major,
                          (unsigned)firmwareVersion.s.minor,
                          (unsigned)firmwareVersion.s.revision);
            return true;
        }

        if (!allowLegacy)
        {
            if (err == MDCO::Error_t::OK)
                log.error("Drive firmware is %u.%u.%u, CANopen firmware older than %u.0.0 is "
                          "not supported by this candletool",
                          (unsigned)firmwareVersion.s.major,
                          (unsigned)firmwareVersion.s.minor,
                          (unsigned)firmwareVersion.s.revision,
                          (unsigned)LEGACY_EDS_BELOW_FW_MAJOR);
            else
                log.error("Drive did not report its firmware version, it predates %u.0.0 which "
                          "is not supported by this candletool",
                          (unsigned)LEGACY_EDS_BELOW_FW_MAJOR);
            log.error("Update the drive firmware with: candletool mdco update latest -i %u",
                      (unsigned)md.m_canId);
            return false;
        }

        // Nothing to change when the legacy .eds is already the loaded one
        if (paths.legacy.empty() || std::filesystem::equivalent(paths.legacy, paths.current, ec))
            return true;

        auto legacyPair = EDSParser::load(paths.legacy);
        if (legacyPair.second != EDSParser::Error_t::OK || legacyPair.first == nullptr)
        {
            log.error("Could not load the legacy .eds from %s, keeping the default one",
                      paths.legacy.c_str());
            return true;
        }

        *od = std::move(*legacyPair.first);

        if (err == MDCO::Error_t::OK)
            log.info(
                "Drive firmware is %u.%u.%u (older than %u.0.0) - the .eds was automatically "
                "changed from %s to %s",
                (unsigned)firmwareVersion.s.major,
                (unsigned)firmwareVersion.s.minor,
                (unsigned)firmwareVersion.s.revision,
                (unsigned)LEGACY_EDS_BELOW_FW_MAJOR,
                paths.current.filename().string().c_str(),
                paths.legacy.filename().string().c_str());
        else
            log.info(
                "Drive did not report its firmware version, assuming it predates %u.0.0 - the "
                ".eds was automatically changed from %s to %s",
                (unsigned)LEGACY_EDS_BELOW_FW_MAJOR,
                paths.current.filename().string().c_str(),
                paths.legacy.filename().string().c_str());
        return true;
    }

    /// @brief Ask the drive which CANopen interface revision it implements (0x1018:03)
    ///
    /// The identity object is mandatory in CiA 301 and its address never changes, so it is read
    /// without the loaded dictionary, which may not describe the drive yet
    /// @return the revision, 0 when the drive predates it or did not answer
    static u32 readDriveRevision(MDCO& md)
    {
        // The default value only satisfies the constructor, the value is read from the drive
        EDSEntry entry(EDSEntry::EDSEntryMetaData{
            .address         = {0x1018, u8{0x03}},
            .parameterName   = "Revision Number",
            .objectType      = EDSEntry::ObjectType_E::VALUE,
            .storageLocation = EDSEntry::StorageLocation_E::RAM,
            .edsValueMeta    = EDSEntry::EDSValueMetaData{.dataType =
                                                           EDSEntry::DataType_E::UNSIGNED32,
                                                       .accessType =
                                                           EDSEntry::AccessRights_E::READ_ONLY,
                                                       .PDOMapping      = false,
                                                       .defaultValueStr = "0"},
            .edsContainerMeta = std::nullopt});
        if (md.readSDO(entry) != MDCO::Error_t::OK)
            return 0;
        return (u32)(u64)entry;
    }

    /// @brief Replace the loaded dictionary with the one in edsPath
    /// @return false when the file could not be parsed, the dictionary is left unchanged then
    static bool loadEdsInto(std::shared_ptr<EDSObjectDictionary> od,
                            const std::filesystem::path&         edsPath,
                            const Logger&                        log)
    {
        auto odPair = EDSParser::load(edsPath);
        if (odPair.second != EDSParser::Error_t::OK || odPair.first == nullptr ||
            odPair.first->size() == 0)
        {
            log.error("Could not load the .eds from %s", edsPath.c_str());
            return false;
        }
        *od = std::move(*odPair.first);
        return true;
    }

    /// @brief Download md_<major>.<minor>.eds from the firmware server into the cache
    /// @return path of the cached file, nothing when the server has none or it is not usable
    static std::optional<std::filesystem::path> downloadEds(u32                          revision,
                                                            const std::filesystem::path& cacheDir,
                                                            const Logger&                log)
    {
        const std::string fileName = "md_" + std::to_string(revisionMajor(revision)) + "." +
                                     std::to_string(revisionMinor(revision)) + ".eds";

        std::error_code ec;
        std::filesystem::create_directories(cacheDir, ec);
        if (ec)
        {
            log.debug("Could not create %s: %s", cacheDir.c_str(), ec.message().c_str());
            return std::nullopt;
        }

        // Downloaded next to its final name and moved there only once it proved usable, so an
        // interrupted or wrong download is never picked up by a later run
        const auto partPath  = cacheDir / (fileName + ".part");
        const auto finalPath = cacheDir / fileName;
        std::filesystem::remove(partPath, ec);

        const std::string url = std::string(CurlHandler::FW_SERVER_ROOT) + "eds/" + fileName;
        log.info("Downloading %s for this drive...", fileName.c_str());
        if (!CurlHandler::download(url, partPath, true))
        {
            log.debug("Could not download %s", url.c_str());
            return std::nullopt;
        }

        const auto info   = readEdsInfo(partPath);
        const auto odPair = EDSParser::load(partPath);
        if (info.revision != revision || odPair.second != EDSParser::Error_t::OK ||
            odPair.first == nullptr || odPair.first->size() == 0)
        {
            log.warn("Downloaded %s is not a usable .eds for revision %u.%u, discarding it",
                     fileName.c_str(),
                     (unsigned)revisionMajor(revision),
                     (unsigned)revisionMinor(revision));
            std::filesystem::remove(partPath, ec);
            return std::nullopt;
        }

        std::filesystem::rename(partPath, finalPath, ec);
        if (ec)
        {
            log.debug("Could not move %s into place: %s", partPath.c_str(), ec.message().c_str());
            std::filesystem::remove(partPath, ec);
            return std::nullopt;
        }
        log.success("Saved %s", finalPath.c_str());
        return finalPath;
    }

    /// @brief Pick the dictionary by the interface revision the drive reports
    static bool useEdsMatchingRevision(u32                                  revision,
                                       std::shared_ptr<EDSObjectDictionary> od,
                                       const EdsPaths_S&                    paths,
                                       const Logger&                        log,
                                       bool                                 allowLegacy)
    {
        const unsigned major = revisionMajor(revision);
        const unsigned minor = revisionMinor(revision);

        std::error_code ec;
        const bool      selectedByUser =
            !paths.standard.empty() && !std::filesystem::equivalent(paths.current, paths.standard, ec);

        // An .eds chosen with "mdco eds" wins as long as it describes the same interface major
        // revision, a minor revision only adds objects
        if (selectedByUser)
        {
            const u32 selected = readEdsInfo(paths.current).revision;
            if (revisionMajor(selected) == major)
            {
                if (selected != revision)
                    log.warn("The selected %s describes CANopen interface %u.%u, the drive "
                             "implements %u.%u - objects may be missing or not match",
                             paths.current.filename().string().c_str(),
                             (unsigned)revisionMajor(selected),
                             (unsigned)revisionMinor(selected),
                             major,
                             minor);
                return true;
            }
            log.warn("The selected %s does not describe the drive (CANopen interface %u.%u), "
                     "choosing an .eds automatically",
                     paths.current.filename().string().c_str(),
                     major,
                     minor);
        }

        std::vector<BundledEds_S> known = listBundledEds(paths.bundledDir);
        const auto                cached = listBundledEds(paths.cacheDir);
        known.insert(known.end(), cached.begin(), cached.end());

        std::optional<std::filesystem::path> exact;
        for (const auto& eds : known)
            if (eds.revision == revision)
            {
                exact = eds.path;
                break;
            }

        if (!exact.has_value())
            exact = downloadEds(revision, paths.cacheDir, log);

        if (exact.has_value())
        {
            if (std::filesystem::equivalent(exact.value(), paths.current, ec))
            {
                log.debug("Drive implements CANopen interface %u.%u, keeping %s",
                          major,
                          minor,
                          paths.current.c_str());
                return true;
            }
            if (loadEdsInto(od, exact.value(), log))
            {
                log.debug("Drive implements CANopen interface %u.%u, using %s",
                          major,
                          minor,
                          exact.value().c_str());
                return true;
            }
        }

        // Closest older minor revision of the same major one describes every object the drive
        // had back then, which covers everything this candletool uses
        const BundledEds_S* closest = nullptr;
        for (const auto& eds : known)
            if (revisionMajor(eds.revision) == major && revisionMinor(eds.revision) < minor &&
                (closest == nullptr || eds.revision > closest->revision))
                closest = &eds;

        if (closest != nullptr &&
            (std::filesystem::equivalent(closest->path, paths.current, ec) ||
             loadEdsInto(od, closest->path, log)))
        {
            log.warn("Drive implements CANopen interface %u.%u, the closest available .eds is "
                     "%u.%u (%s) - objects added since are not available",
                     major,
                     minor,
                     (unsigned)revisionMajor(closest->revision),
                     (unsigned)revisionMinor(closest->revision),
                     closest->path.filename().string().c_str());
            log.warn("Update candletool, or select md_%u.%u.eds with: candletool mdco eds <path>",
                     major,
                     minor);
            return true;
        }

        if (allowLegacy)
        {
            log.warn("No .eds describes CANopen interface %u.%u of the drive, keeping %s",
                     major,
                     minor,
                     paths.current.filename().string().c_str());
            return true;
        }

        log.error("No .eds describes CANopen interface %u.%u of the drive - update candletool, "
                  "or select a matching .eds with: candletool mdco eds <path>",
                  major,
                  minor);
        return false;
    }

    bool useEdsMatchingFirmware(MDCO&                                md,
                                std::shared_ptr<EDSObjectDictionary> od,
                                const EdsPaths_S&                    paths,
                                const Logger&                        log,
                                bool                                 allowLegacy)
    {
        if (od == nullptr)
            return true;

        const u32 revision = readDriveRevision(md);
        if (revision != 0)
            return useEdsMatchingRevision(revision, od, paths, log, allowLegacy);

        return useEdsMatchingFirmwareVersion(md, od, paths, log, allowLegacy);
    }
}  // namespace mab
