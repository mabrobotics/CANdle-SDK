#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string_view>
#include "candletool_cli.hpp"
#include "json.h"

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

    // Both HTTP helpers use the curl executable, same as CurlHandler: it is a .deb dependency on
    // Linux and ships with Windows 10+, so nothing has to be linked on any platform.
    bool CandletoolCli::downloadFile(const std::string&           url,
                                     const std::filesystem::path& outputPath)
    {
        std::string cmd = "curl --fail -L -o \"" + outputPath.string() + "\" \"" + url + "\"";
        return !executeCommand(cmd);
    }

    bool CandletoolCli::installPackage(const std::filesystem::path& path)
    {
#ifdef __linux__
        std::string cmd = "sudo dpkg -i " + path.string();
#elif _WIN32
        std::string cmd = "\"" + path.string() + "\"";
#else
        m_logger.error("Auto-install is not supported on this OS.");
        return false;
#endif

        int result = std::system(cmd.c_str());
        return result == 0;
    }

    std::optional<std::string> CandletoolCli::fetchUrl(const std::string& url)
    {
        std::string cmd = "curl --fail -sSL \"" + url + "\"";
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

                m_logger.info("Do you want to download and install it? [y/N]");
                std::string answer;
                std::getline(std::cin, answer);
                if (answer != "y" && answer != "Y")
                {
                    return;
                }

                // download phase
                // Assets are named candletool-<version>-<tag>-<platform>.<ext>, next to mdgui and
                // other platforms' packages. The tag letter differs between builds, so match only
                // the package name and this build's own platform suffix.
                constexpr std::string_view packagePrefix = "candletool-";
                constexpr std::string_view packageSuffix = CANDLETOOL_PACKAGE_SUFFIX;

                std::string   downloadUrl;
                json_array_s* assets = jsonArray(release.get(), "assets");
                for (auto* asset = assets ? assets->start : nullptr; asset; asset = asset->next)
                {
                    std::string name = jsonString(asset->value, "name");
                    if (name.starts_with(packagePrefix) && name.ends_with(packageSuffix))
                    {
                        downloadUrl = jsonString(asset->value, "browser_download_url");
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

                std::filesystem::path tmpDirectory = std::filesystem::temp_directory_path();
                std::string fileName = downloadUrl.substr(downloadUrl.find_last_of('/') + 1);
                std::filesystem::path outputDirectory = tmpDirectory / fileName;

                if (!downloadFile(downloadUrl, outputDirectory))
                {
                    m_logger.error("Failed to download update file.");
                    return;
                }

                m_logger.info("Downloaded to: %s", outputDirectory.string().c_str());

                // install phase
                if (!installPackage(outputDirectory))
                {
                    m_logger.error("Installation failed.");
                    return;
                }

                m_logger.info("Update installed successfully.");

                std::error_code ec;
                std::filesystem::remove(outputDirectory, ec);
                if (ec)
                {
                    m_logger.error("Failed to remove temporary file: %s", ec.message().c_str());
                }
            });
    }

}  // namespace mab
