#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string_view>
#include "candletool_cli.hpp"
#include "json.h"
#include "picosha2.h"

#ifdef _WIN32
#include <windows.h>
// excluded from windows.h by WIN32_LEAN_AND_MEAN
#include <shellapi.h>
#else
#include <unistd.h>
#endif

namespace mab
{
    // json.h lookups; missing keys and wrong types yield nullptr / empty string
    static json_value_s* jsonMember(json_value_s* object, std::string_view key)
    {
        json_object_s* obj = object ? json_value_as_object(object) : nullptr;
        if (obj == nullptr)
            return nullptr;

        for (json_object_element_s* element = obj->start; element; element = element->next)
        {
            if (std::string_view(element->name->string, element->name->string_size) == key)
                return element->value;
        }
        return nullptr;
    }

    static std::string jsonString(json_value_s* object, std::string_view key)
    {
        json_value_s*  value = jsonMember(object, key);
        json_string_s* str   = value ? json_value_as_string(value) : nullptr;
        return str ? std::string(str->string, str->string_size) : std::string{};
    }

    static json_array_s* jsonArray(json_value_s* object, std::string_view key)
    {
        json_value_s* value = jsonMember(object, key);
        return value ? json_value_as_array(value) : nullptr;
    }

    std::optional<CandletoolVersion> parseVersion(const std::string& tag)
    {
        const char* s = tag.c_str();
        if (*s == 'v' || *s == 'V')
            s++;

        CandletoolVersion version;
        if (std::sscanf(s, "%d.%d.%d", &version.major, &version.minor, &version.patch) != 3)
            return std::nullopt;
        return version;
    }

    // Lowercase hex, the same form GitHub uses in an asset's "digest" field
    static std::optional<std::string> sha256Hex(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
            return std::nullopt;
        return picosha2::hash256_hex_string(std::istreambuf_iterator<char>(file),
                                            std::istreambuf_iterator<char>());
    }

#ifndef _WIN32
    // Removes the private download directory, and the package in it, on every exit path
    struct ScopedDirectory
    {
        std::filesystem::path path;

        ~ScopedDirectory()
        {
            std::error_code ec;
            std::filesystem::remove_all(path, ec);
        }
    };
#endif

    // Both HTTP helpers use the curl executable, same as CurlHandler: it is a .deb dependency on
    // Linux and ships with Windows 10+, so nothing has to be linked on any platform.
    // No overall time limit for the package (slow links are fine), but give up on a connection
    // that can't be made in 10 s or a transfer that stalls completely for 30 s.
    bool CandletoolCli::downloadFile(const std::string&           url,
                                     const std::filesystem::path& outputPath)
    {
        std::string cmd = "curl --fail -L --connect-timeout 10 --speed-time 30 -o \"" +
                          outputPath.string() + "\" \"" + url + "\"";
        return !executeCommand(cmd);
    }

    bool CandletoolCli::installPackage(const std::filesystem::path& path)
    {
#ifdef __linux__
        // apt-get, unlike dpkg -i, also installs dependencies a new version adds. sudo is skipped
        // when already root, e.g. in containers that have no sudo.
        std::string cmd = geteuid() == 0 ? "" : "sudo ";
        cmd += m_assumeYes ? "apt-get install -y " : "apt-get install ";
        cmd += "\"" + path.string() + "\"";
        return !executeCommand(cmd);
#elif _WIN32
        // Only launches the installer, without waiting: it has to replace this candletool.exe,
        // which Windows keeps locked until the process exits. "runas" shows the UAC prompt.
        HINSTANCE result =
            ShellExecuteW(nullptr, L"runas", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        INT_PTR code = reinterpret_cast<INT_PTR>(result);
        if (code <= 32)
        {
            m_logger.error("Failed to launch the installer. Error code: %d",
                           static_cast<int>(code));
            return false;
        }
        return true;
#else
        m_logger.error("Auto-install is not supported on this OS.");
        return false;
#endif
    }

    std::optional<std::string> CandletoolCli::fetchUrl(const std::string& url)
    {
        std::string cmd = "curl --fail -sSL --connect-timeout 10 --max-time 30 \"" + url + "\"";
#ifdef _WIN32
        FILE* pipe = _popen(cmd.c_str(), "r");
#else
        FILE* pipe = popen(cmd.c_str(), "r");
#endif
        if (!pipe)
            return std::nullopt;

        std::string body;
        char        buffer[4096];
        size_t      bytesRead;
        while ((bytesRead = std::fread(buffer, 1, sizeof(buffer), pipe)) > 0)
            body.append(buffer, bytesRead);

#ifdef _WIN32
        int status = _pclose(pipe);
#else
        int status = pclose(pipe);
#endif
        // --fail makes curl exit non-zero on HTTP errors, so a zero status means a 2xx response
        if (status != 0)
            return std::nullopt;

        return body;
    }

    CandletoolCli::CandletoolCli(CLI::App* rootCli, CANdleToolCtx_S ctx)
    {
        // candletool update
        auto* updateCandletool =
            rootCli->add_subcommand("update", "Automatically update candletool to newest version.");
        updateCandletool->add_flag(
            "-y,--yes", m_assumeYes, "Install without asking for confirmation (for scripts).");

        updateCandletool->callback(
            [this, ctx]()
            {
                m_logger.info("Performing candletool software update.");

                constexpr CandletoolVersion currentVersion{
                    CANDLETOOL_VMAJOR, CANDLETOOL_VMINOR, CANDLETOOL_VREVISION};

                std::optional<std::string> body = fetchUrl(repoUrl);
                if (!body)
                {
                    m_logger.error(
                        "Failed to reach GitHub API. Make sure curl is installed and the "
                        "network is available.");
                    return;
                }
                // json_parse returns a single malloc'd block holding the whole tree
                std::unique_ptr<json_value_s, decltype(&std::free)> release(
                    json_parse(body->data(), body->size()), &std::free);
                if (!release)
                {
                    m_logger.error("Failed to parse release info.");
                    return;
                }
                std::string latestTag     = jsonString(release.get(), "tag_name");
                auto        latestVersion = parseVersion(latestTag);
                if (!latestVersion)
                {
                    m_logger.error("Unexpected release tag \"%s\" from GitHub.", latestTag.c_str());
                    return;
                }

                // Builds newer than the latest release (e.g. from devel) are not downgraded
                if (*latestVersion <= currentVersion)
                {
                    m_logger.info("candletool %s is up to date (latest release: %s).",
                                  CANDLESDK_VERSION,
                                  latestTag.c_str());
                    return;
                }

                m_logger.info(
                    "Update available! Found version: %s. Current version installed is: %s",
                    latestTag.c_str(),
                    CANDLESDK_VERSION);

                if (!m_assumeYes)
                {
                    // Plain stdout rather than the logger, so the question still shows with -s
                    std::cout << "Download and install candletool " << latestTag << "? [y/N] "
                              << std::flush;
                    std::string answer;
                    std::getline(std::cin, answer);
                    if (answer != "y" && answer != "Y")
                    {
                        return;
                    }
                }

                // download phase
                // Assets are named candletool-<version>-<tag>-<platform>.<ext>, next to mdgui and
                // other platforms' packages. The tag letter differs between builds, so match only
                // the package name and this build's own platform suffix.
                constexpr std::string_view packagePrefix = "candletool-";
                constexpr std::string_view packageSuffix = CANDLETOOL_PACKAGE_SUFFIX;

                std::string   downloadUrl;
                std::string   digest;  // "sha256:<hex>", empty if GitHub has none for the asset
                json_array_s* assets = jsonArray(release.get(), "assets");
                for (auto* asset = assets ? assets->start : nullptr; asset; asset = asset->next)
                {
                    std::string name = jsonString(asset->value, "name");
                    if (name.starts_with(packagePrefix) && name.ends_with(packageSuffix))
                    {
                        downloadUrl = jsonString(asset->value, "browser_download_url");
                        digest      = jsonString(asset->value, "digest");
                        break;
                    }
                }
                if (downloadUrl.empty())
                {
                    m_logger.error("Release %s has no candletool-*%s package for this platform.",
                                   latestTag.c_str(),
                                   CANDLETOOL_PACKAGE_SUFFIX);
                    return;
                }

                std::error_code       ec;
                std::filesystem::path tempRoot = std::filesystem::temp_directory_path(ec);
                if (ec)
                {
                    m_logger.error("No usable temporary directory: %s", ec.message().c_str());
                    return;
                }
#ifdef _WIN32
                // %TEMP% is already private to the user, and the installer still needs its file
                // after candletool exits, so it is downloaded there and not cleaned up
                std::filesystem::path downloadDirectory = tempRoot;
#else
                // The package is installed as root, so it goes to a fresh 0700 directory with a
                // random name instead of a predictable path in the shared /tmp: no other user can
                // pre-create or swap it between download and install.
                std::string directoryTemplate = (tempRoot / "candletool-XXXXXX").string();
                if (mkdtemp(directoryTemplate.data()) == nullptr)
                {
                    m_logger.error("Failed to create a temporary download directory.");
                    return;
                }
                std::filesystem::path downloadDirectory = directoryTemplate;
                ScopedDirectory       downloadCleanup{downloadDirectory};

                // apt-get reads local packages as its unprivileged _apt user. Let others enter the
                // directory (not list or write it) so it can, instead of printing a notice about
                // falling back to root; the package itself stays writable only by us.
                namespace fs = std::filesystem;
                fs::permissions(downloadDirectory,
                                fs::perms::owner_all | fs::perms::group_exec |
                                    fs::perms::others_exec,
                                ec);
#endif
                std::string fileName = downloadUrl.substr(downloadUrl.find_last_of('/') + 1);
                std::filesystem::path packagePath = downloadDirectory / fileName;

                if (!downloadFile(downloadUrl, packagePath))
                {
                    m_logger.error("Failed to download update file.");
                    return;
                }

                m_logger.info("Downloaded to: %s", packagePath.string().c_str());

                constexpr std::string_view sha256Prefix = "sha256:";
                if (digest.starts_with(sha256Prefix))
                {
                    std::optional<std::string> actual = sha256Hex(packagePath);
                    if (!actual || *actual != digest.substr(sha256Prefix.size()))
                    {
                        m_logger.error(
                            "SHA-256 of %s does not match the release, not installing it.",
                            fileName.c_str());
                        return;
                    }
                    m_logger.info("SHA-256 checksum verified.");
                }
                else
                {
                    m_logger.warn("Release has no SHA-256 checksum for %s, installing unverified.",
                                  fileName.c_str());
                }
#ifndef _WIN32
                // Readable for apt-get's _apt user regardless of umask (see directory above)
                fs::permissions(packagePath,
                                fs::perms::owner_read | fs::perms::owner_write |
                                    fs::perms::group_read | fs::perms::others_read,
                                ec);
#endif

                // install phase
                if (!installPackage(packagePath))
                {
                    m_logger.error("Installation failed.");
                    return;
                }

#ifdef _WIN32
                // The installer is still running and needs its file, so it is left in the temp
                // directory; candletool exits right after this so the installer can replace it.
                m_logger.info("Installer launched. Finish the update in the installer window.");
#else
                m_logger.info("Update installed successfully.");
#endif
            });
    }

}  // namespace mab
