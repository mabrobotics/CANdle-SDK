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

    int compareCandletoolVersion(const CandletoolVersion* latest, const CandletoolVersion* current)
    {
        if (latest->major != current->major)
            return latest->major < current->major ? -1 : 1;
        if (latest->minor != current->minor)
            return latest->minor < current->minor ? -1 : 1;
        if (latest->patch != current->patch)
            return latest->patch < current->patch ? -1 : 1;
        return 0;
    }

    CandletoolVersion parseVersion(const char* s)
    {
        CandletoolVersion v = {0, 0, 0};

        if (!s || !*s)
            return v;
        if (s[0] == 'v' || s[0] == 'V')
            s++;
        std::sscanf(s, "%d.%d.%d", &v.major, &v.minor, &v.patch);

        return v;
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

                // check candle version
                const std::string currentVersion = CANDLETOOL_VERSION;

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
                std::string latestVersion = jsonString(release.get(), "tag_name");
                if (latestVersion.empty())
                {
                    m_logger.error("Failed to get response from GitHub.");
                    return;
                }

                CandletoolVersion latestV  = parseVersion(latestVersion.c_str());
                CandletoolVersion currentV = parseVersion(currentVersion.c_str());

                if (compareCandletoolVersion(&latestV, &currentV) == 0)
                {
                    m_logger.info(
                        ("candletool is already up to date (" + currentVersion + ").").c_str());
                    return;
                }

                m_logger.info(("Update available! Found version: " + latestVersion +
                               ". Current version installed is: " + currentVersion)
                                  .c_str());

                m_logger.info("Do you want to download and install it? [y/N]");
                std::string answer;
                std::getline(std::cin, answer);
                if (answer != "y" && answer != "Y")
                {
                    return;
                }

                // download phase
                std::string targetExtension;
#ifdef _WIN32
                targetExtension = ".exe";
#else
                targetExtension = "x86_64.deb";
#endif
                std::string   downloadUrl;
                json_array_s* assets = jsonArray(release.get(), "assets");
                for (auto* asset = assets ? assets->start : nullptr; asset; asset = asset->next)
                {
                    std::string name = jsonString(asset->value, "name");
                    if (name.size() > targetExtension.size() &&
                        name.compare(name.size() - targetExtension.size(),
                                     targetExtension.size(),
                                     targetExtension) == 0)
                    {
                        downloadUrl = jsonString(asset->value, "browser_download_url");
                        break;
                    }
                }
                if (downloadUrl.empty())
                {
                    m_logger.error("No matching release downloadable for this platform.");
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

                m_logger.info(("Downloaded to: " + outputDirectory.string()).c_str());

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
                    m_logger.error(("Failed to remove temporary file: " + ec.message()).c_str());
                }
            });
    }

}  // namespace mab
