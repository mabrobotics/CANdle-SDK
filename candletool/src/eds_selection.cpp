#include "eds_selection.hpp"

#include "edsParser.hpp"
#include "mini/ini.h"

namespace mab
{
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
