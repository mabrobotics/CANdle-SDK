#include "md_update.hpp"

#include <cstdio>
#include <memory>
#include <string>

#include "MD.hpp"
#include "MDCO.hpp"
#include "bl2Loader.hpp"
#include "bl3Loader.hpp"
#include "candle.hpp"
#include "curl_handler.hpp"
#include "edsParser.hpp"
#include "eds_selection.hpp"
#include "flasher.hpp"
#include "mabFileParser.hpp"
#include "mab_types.hpp"
#include "utilities.hpp"
#include "mini/ini.h"

namespace mab
{
    namespace
    {
        using mdUpdate::Drive_E;
        using mdUpdate::Target;

        // Firmware server directories, relative to CurlHandler::FW_SERVER_ROOT
        constexpr const char* FW_MD_DIR        = "md/";         // .mab files, fw >= 3.0.0
        constexpr const char* FW_MD_LEGACY_DIR = "md/legacy/";  // flasher executables, fw < 3.0.0
        constexpr const char* FW_MDCO_DIR      = "mdco/";       // CANopen .mab files, fw >= 3.0.0

        // Oldest CANopen firmware candletool installs, older releases use .eds revisions this
        // candletool does not support
        constexpr version_ut MIN_CANOPEN_FW = {
            .s = {.tag = 0, .revision = 1, .minor = 0, .major = 3}};

        // Firmware older than 3.0.0 sets this u8 register to 1 on drives that can not run
        // firmware newer than MAX_LIMITED_FW, newer firmware does not have it
        constexpr u16        LIMITED_FW_REGISTER = 0x820;
        constexpr version_ut MAX_LIMITED_FW      = {
            .s = {.tag = 0, .revision = 4, .minor = 5, .major = 2}};

        // Time the drive gets after the reset before its bootloader is called
        constexpr u32 RESET_US = 200'000;
        // IDENTIFY tries after the reset, ~50 ms each, before the drive is taken for a G4
        constexpr u32 IDENTIFY_ATTEMPTS = 10;
        // Time the application gets to start before it is asked whether it runs
        constexpr u32 APP_START_US = 500'000;

        Logger log(Logger::ProgramLayer_E::TOP, "MD UPDATE");

        bool isCanOpen(Drive_E drive)
        {
            return drive == Drive_E::MDCO || drive == Drive_E::MDCO_LEGACY;
        }

        /// @brief Candle with the frame format of the builder, or CAN 2.0 for CANopen
        Candle* buildCandle(const Target& target, bool can20)
        {
            CandleBuilder builder = *target.candleBuilder;
            if (can20)
                builder.useCAN20Frames = true;
            Candle* candle = builder.build().value_or(nullptr);
            if (candle == nullptr)
                log.error("Could not connect to candle!");
            return candle;
        }

        /// @brief The candle for the running firmware: CAN 2.0 for CANopen, else the builder's
        Candle* buildAppCandle(const Target& target)
        {
            return buildCandle(target, isCanOpen(target.drive));
        }

        /// @brief The candle for the bootloader (64 B frames). A CANopen candle is replaced, the
        /// USB device takes only one at a time
        Candle* switchToBootloaderCandle(const Target& target, Candle* candle)
        {
            if (!isCanOpen(target.drive))
                return candle;
            detachCandle(candle);
            return buildCandle(target, false);
        }

        /// @brief Read the version of / reset the drive over CANopen
        /// @param versionOut when set, the version is read into it instead of resetting the drive
        bool overCanOpen(const Target& target, Candle* candle, version_ut* versionOut)
        {
            const auto edsPaths = readEdsPaths(target.etcPath / "config/candletool.ini", log);
            if (!edsPaths.has_value())
                return false;

            auto [od, parserError] = EDSParser::load(edsPaths.value().current);
            if (parserError != EDSParser::Error_t::OK)
                log.warn("EDS parsing failed!");
            if (od == nullptr)
            {
                log.error("Could not load object dictionary from %s!",
                          edsPaths.value().current.c_str());
                return false;
            }

            MDCO mdco(target.canId, candle, od);
            if (mdco.init() != MDCO::Error_t::OK)
            {
                log.error("Could not communicate with MD device with ID %d over CANopen",
                          target.canId);
                return false;
            }

            // A drive waiting to be flashed may well be running older firmware, whose dictionary
            // holds the reset command at a different address
            useEdsMatchingFirmware(mdco, od, edsPaths.value(), log, true);

            if (versionOut != nullptr)
            {
                const auto [version, err] = mdco.getFirmwareVersion();
                if (err != MDCO::Error_t::OK)
                {
                    log.error("Could not read firmware version of MD device with ID %d",
                              target.canId);
                    return false;
                }
                *versionOut = version;
                return true;
            }

            if (mdco.reset() != MDCO::Error_t::OK)
            {
                log.error("Error resetting MD device with ID %d over CANopen", target.canId);
                return false;
            }
            return true;
        }

        /// @brief compare versions ignoring tag, returns <0, 0 or >0
        int compareVersion(version_ut a, version_ut b)
        {
            u32 va = (a.s.major << 16) | (a.s.minor << 8) | a.s.revision;
            u32 vb = (b.s.major << 16) | (b.s.minor << 8) | b.s.revision;
            return (va > vb) - (va < vb);
        }

        /// @brief Refuse CANopen firmware older than MIN_CANOPEN_FW
        bool isSupportedCanOpenTarget(version_ut target)
        {
            if (compareVersion(target, MIN_CANOPEN_FW) >= 0)
                return true;
            log.error(
                "CANopen firmware v%d.%d.%d is not supported by this candletool, install "
                "v%d.%d.%d or newer (\"latest\")",
                target.s.major,
                target.s.minor,
                target.s.revision,
                MIN_CANOPEN_FW.s.major,
                MIN_CANOPEN_FW.s.minor,
                MIN_CANOPEN_FW.s.revision);
            return false;
        }

        /// @param limited set when the drive can not run firmware newer than MAX_LIMITED_FW
        bool readFirmwareVersion(const Target& target,
                                 Candle*       candle,
                                 version_ut*   version,
                                 bool*         limited)
        {
            *limited = false;
            if (isCanOpen(target.drive))
                return overCanOpen(target, candle, version);
            MD md(target.canId, candle);
            if (md.init() != MD::Error_t::OK)
            {
                log.error("Could not communicate with MD device with ID %d", target.canId);
                return false;
            }
            *version = getMdFirmwareVersion(md);
            if (version->s.major < 3)
            {
                // A drive without the register is not limited, its failed read is not an error
                u8 flag = 0;
                md.setLogLevel(Logger::LogLevel_E::SILENT);
                *limited = md.readRegisterRaw(LIMITED_FW_REGISTER, &flag, sizeof(flag)) ==
                               MD::Error_t::OK &&
                           flag == 1;
            }
            return true;
        }

        /// @brief Reset the running firmware, the drive starts its bootloader
        bool resetDrive(const Target& target, Candle* candle)
        {
            if (isCanOpen(target.drive))
            {
                if (!overCanOpen(target, candle, nullptr))
                    return false;
            }
            else
            {
                MD md(target.canId, candle);
                if (md.init() != MD::Error_t::OK)
                {
                    log.error("Could not communicate with MD device with ID %d", target.canId);
                    return false;
                }
                md.reset();
            }
            usleep(RESET_US);
            return true;
        }

        /// @brief Show what changes on the drive and ask the user
        bool confirmUpdate(const Target& target,
                           Candle*       candle,
                           version_ut    targetVersion,
                           bool          targetCanOpen)
        {
            version_ut currentVersion;
            bool       limited = false;
            if (!readFirmwareVersion(target, candle, &currentVersion, &limited))
                return false;
            if (limited && compareVersion(targetVersion, MAX_LIMITED_FW) > 0)
            {
                log.error(
                    "MD @ %d supports firmware up to v%d.%d.%d, v%d.%d.%d can not be "
                    "installed!",
                    target.canId,
                    MAX_LIMITED_FW.s.major,
                    MAX_LIMITED_FW.s.minor,
                    MAX_LIMITED_FW.s.revision,
                    targetVersion.s.major,
                    targetVersion.s.minor,
                    targetVersion.s.revision);
                return false;
            }

            const bool canOpen = isCanOpen(target.drive);
            int        cmp     = compareVersion(targetVersion, currentVersion);
            log.info("%s MD firmware v%d.%d.%d -> v%d.%d.%d",
                     cmp > 0 ? "Upgrading" : (cmp < 0 ? "Downgrading" : "Reinstalling"),
                     currentVersion.s.major,
                     currentVersion.s.minor,
                     currentVersion.s.revision,
                     targetVersion.s.major,
                     targetVersion.s.minor,
                     targetVersion.s.revision);
            if ((currentVersion.s.major < 3) != (targetVersion.s.major < 3))
            {
                log.warn(
                    "This comes with changes, that in specific conditions "
                    "(motor+encoder combinations) may require you to:\n"
                    "- reapply .cfg file,\n"
                    "- perform calibration,\n"
                    "- set zero offset.");
                if (canOpen)
                    log.warn(
                        "Firmware v2.x.x and v3.x.x use different EDS files (v1.1 and v1.2). "
                        "The drive will require a complete reconfiguration!");
            }
            if (targetCanOpen != canOpen)
                log.warn(
                    "Switching firmware communication module from %s to %s! After the update the "
                    "drive must be "
                    "handled with \"candletool %s\" commands.",
                    targetCanOpen ? "MD Protocol" : "CANOpen",
                    targetCanOpen ? "CANOpen" : "MD Protocol",
                    targetCanOpen ? "mdco" : "md");
            if (!userConfirm())
            {
                log.error("Update aborted by user!");
                return false;
            }
            return true;
        }

        /// @brief Warn about and confirm the update of a drive whose firmware is not read
        bool confirmLegacyMigration(const Target& target,
                                    version_ut    targetVersion,
                                    bool          targetCanOpen)
        {
            log.warn(
                "Migrating MD @ %d to %s firmware v%d.%d.%d without checking its current "
                "firmware!",
                target.canId,
                targetCanOpen ? "MDCO" : "MD",
                targetVersion.s.major,
                targetVersion.s.minor,
                targetVersion.s.revision);
            log.warn(
                "Firmware v2.x.x and v3.x.x use different EDS files (v1.1 and v1.2). The "
                "drive will require a complete reconfiguration!");
            if (!userConfirm())
            {
                log.error("Update aborted by user!");
                return false;
            }
            return true;
        }

        /// @brief Flash the drive sitting in its bootloader, with the loader of that bootloader
        ///
        /// G5 drives run Bl3, which answers IDENTIFY. G4 drives run Bl2, which refuses it with
        /// "NO" (or the drive does not answer at all). The file has to be built for the board.
        /// @param wait call the bootloader until it answers (drive being power cycled)
        bool flashBootloader(const Target&        target,
                             Candle*              candle,
                             const MabFileParser& mabFile,
                             bool                 wait)
        {
            bl3Loader::Info info;
            const bool      g5Drive =
                bl3Loader::identify(candle, target.canId, wait ? 0 : IDENTIFY_ATTEMPTS, &info) ==
                bl3Loader::Identify_E::BL3;
            const bool g5File =
                mabFile.m_fwEntry.targetDevice == MabFileParser::TargetDevice_E::MDG5;

            if (g5Drive != g5File)
            {
                log.error("MD @ %d is a %s drive, the file is for %s drives!",
                          target.canId,
                          g5Drive ? "G5" : "G4",
                          g5File ? "G5" : "G4");
                return false;
            }
            if (!g5Drive)
                return bl2Loader::flashAndBoot(candle, target.canId, mabFile, wait);

            const bl3Loader::Result_E result =
                bl3Loader::flashAndBoot(candle, target.canId, mabFile, info);
            if (result != bl3Loader::Result_E::BOOT_UNCONFIRMED)
                return result == bl3Loader::Result_E::OK;

            // The image is written and checked, only the reply to BOOT was lost: the application
            // answering confirms it started
            usleep(APP_START_US);
            if (mabFile.m_fwEntry.variant == MabFileParser::Variant_E::MDCO)
            {
                log.warn("Could not confirm the start of the application, check the drive");
                return true;
            }
            MD md(target.canId, candle);
            if (md.init() != MD::Error_t::OK)
            {
                log.error("The application does not answer after BOOT!");
                return false;
            }
            return true;
        }

        /// @brief Put the drive into its bootloader as target.drive says
        /// @param wait set when the bootloader has to be waited for afterwards
        bool enterBootloader(const Target&        target,
                             Candle*              candle,
                             const MabFileParser& mabFile,
                             version_ut           targetVersion,
                             bool*                wait)
        {
            const bool targetCanOpen = mabFile.m_fwEntry.variant == MabFileParser::Variant_E::MDCO;
            *wait                    = false;
            switch (target.drive)
            {
                case Drive_E::MD:
                case Drive_E::MDCO:
                    return confirmUpdate(target, candle, targetVersion, targetCanOpen) &&
                           resetDrive(target, candle);
                case Drive_E::MDCO_LEGACY:
                    // The version of a legacy drive is not trusted, it is reset right before
                    // flashing so the bootloader window is not missed, and then caught as in
                    // recovery
                    *wait = true;
                    return confirmLegacyMigration(target, targetVersion, targetCanOpen) &&
                           resetDrive(target, candle);
                case Drive_E::BOOTLOADER:
                    log.warn("Recovery mode...");
                    log.warn("Please make sure driver is in the bootloader phase (rebooting)");
                    log.info("Flashing %s firmware", targetCanOpen ? "MDCO" : "MD");
                    *wait = true;
                    return true;
            }
            return false;
        }
    }  // namespace

    bool mdUpdate::parseVersion(const std::string& str, version_ut* version)
    {
        unsigned int major = 0, minor = 0, revision = 0;
        if (sscanf(str.c_str(), "%u.%u.%u", &major, &minor, &revision) != 3 || major > 255 ||
            minor > 255 || revision > 255)
            return false;
        version->i          = 0;
        version->s.major    = major;
        version->s.minor    = minor;
        version->s.revision = revision;
        return true;
    }

    bool mdUpdate::flash(const Target& target, const std::filesystem::path& path)
    {
        std::unique_ptr<MabFileParser> mabFile;
        try
        {
            mabFile =
                std::make_unique<MabFileParser>(path.string(), MabFileParser::TargetDevice_E::MD);
        }
        catch (const std::exception&)
        {
            return false;  // the parser logged why
        }
        version_ut targetVersion = {.i = 0};
        if (!parseVersion((const char*)mabFile->m_fwEntry.version, &targetVersion))
        {
            log.error("Invalid firmware version in .mab file!");
            return false;
        }
        if (mabFile->m_fwEntry.variant == MabFileParser::Variant_E::MDCO &&
            !isSupportedCanOpenTarget(targetVersion))
            return false;

        Candle* candle = buildAppCandle(target);
        if (candle == nullptr)
            return false;
        bool wait = false;
        bool ok   = enterBootloader(target, candle, *mabFile, targetVersion, &wait);
        if (ok)
        {
            candle = switchToBootloaderCandle(target, candle);
            ok     = candle != nullptr && flashBootloader(target, candle, *mabFile, wait);
        }
        if (candle != nullptr)
            detachCandle(candle);

        if (!ok)
        {
            log.error("MD flashing failed!");
            return false;
        }
        log.success("Update complete for MD @ %d", target.canId);
        return true;
    }

    std::optional<std::filesystem::path> mdUpdate::download(const std::string&       version,
                                                            MabFileParser::Variant_E variant)
    {
        const bool         canOpen = variant == MabFileParser::Variant_E::MDCO;
        mINI::INIStructure index;
        if (!CurlHandler::loadIndex(index))
            return {};
        const std::string filename = CurlHandler::findIndexEntry(
            index, canOpen ? "mdco_app_" : "md_app_", version, "filename");
        if (filename.empty())
        {
            log.error("Firmware %s is not available on the server!", version.c_str());
            return {};
        }
        const std::filesystem::path path = std::filesystem::temp_directory_path() / filename;
        if (!CurlHandler::download(std::string(CurlHandler::FW_SERVER_ROOT) +
                                       (canOpen ? FW_MDCO_DIR : FW_MD_DIR) + filename,
                                   path))
        {
            log.error("Could not download firmware [ %s ]", filename.c_str());
            return {};
        }
        return path;
    }

    bool mdUpdate::flashLegacy(const Target& target, const std::string& version)
    {
#ifdef WIN32
        log.error("Firmware older than 3.0.0 can only be installed on Linux!");
        return false;
#else
        version_ut targetVersion = {.i = 0};
        if (!parseVersion(version, &targetVersion) || targetVersion.s.major >= 3 ||
            target.drive == Drive_E::MDCO_LEGACY)
        {
            log.error("Only firmware older than 3.0.0 is installed with its flasher!");
            return false;
        }

        constexpr sysArch_E arch    = getSysArch();
        const char*         archKey = "filename";
        if constexpr (arch == sysArch_E::X86_64)
            archKey = "filename_x86_64";
        else if constexpr (arch == sysArch_E::ARM64)
            archKey = "filename_arm64";
        else if constexpr (arch == sysArch_E::ARMHF)
            archKey = "filename_armhf";

        mINI::INIStructure index;
        if (!CurlHandler::loadIndex(index))
            return false;
        const std::string filename =
            CurlHandler::findIndexEntry(index, "mab_can_flasher_", version, archKey);
        if (filename.empty())
        {
            log.error("Firmware %s is not available for this platform!", version.c_str());
            return false;
        }

        // The flasher resets the drive itself, the candle only reads the version and is closed
        // before the flasher opens the USB device
        const bool recovery = target.drive == Drive_E::BOOTLOADER;
        if (!recovery)
        {
            Candle* candle = buildAppCandle(target);
            if (candle == nullptr)
                return false;
            const bool confirmed =
                confirmUpdate(target, candle, targetVersion, isCanOpen(target.drive));
            detachCandle(candle);
            if (!confirmed)
                return false;
        }

        WebFile_S flasherFile;
        flasherFile.m_type = WebFile_S::Type_E::MD_FLASHER;
        flasherFile.m_path = std::filesystem::temp_directory_path() / filename;
        if (!CurlHandler::download(
                std::string(CurlHandler::FW_SERVER_ROOT) + FW_MD_LEGACY_DIR + filename,
                flasherFile.m_path))
        {
            log.error("Could not download firmware [ %s ]", filename.c_str());
            return false;
        }
        Flasher flasher(flasherFile);
        if (flasher.flash(recovery ? 9 : target.canId, recovery) != Flasher::Error_E::OK)
        {
            log.error("Error while flashing firmware!");
            return false;
        }
        log.success("Update complete for MD @ %d", target.canId);
        return true;
#endif
    }

    bool mdUpdate::eraseConfig(const Target& target)
    {
        Candle* candle = buildAppCandle(target);
        if (candle == nullptr)
            return false;
        bool ok = target.drive == Drive_E::BOOTLOADER || resetDrive(target, candle);
        if (ok)
            candle = switchToBootloaderCandle(target, candle);
        if (ok && candle != nullptr)
        {
            bl3Loader::Info info;
            if (bl3Loader::identify(candle, target.canId, 0, &info) == bl3Loader::Identify_E::BL3)
                ok = bl3Loader::eraseConfig(candle, target.canId);
            else
                ok = bl2Loader::eraseConfig(candle, target.canId);
        }
        if (candle != nullptr)
            detachCandle(candle);

        if (!ok)
        {
            log.error("Force-erase failed!");
            return false;
        }
        log.success("Force-erase complete for MD @ %d", target.canId);
        return true;
    }
}  // namespace mab
