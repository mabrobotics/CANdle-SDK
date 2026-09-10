#include <filesystem>
#include "candletool_cli.hpp"
#include "curl/curl.h"
#include "nlohmann/json.hpp"

namespace mab
{
    static size_t curlCallback(char* ptr, size_t size, size_t nmemb, std::string* out)
    {
        out->append(ptr, size * nmemb);
        return size * nmemb;
    }

    static size_t writeToFileCallback(char* ptr, size_t size, size_t nmemb, FILE* file)
    {
        return fwrite(ptr, size, nmemb, file);
    }

    CandletoolVersion parseVersion(std::string s)
    {
        if (!s.empty() && s[0] == 'v')
            s.erase(0, 1);

        CandletoolVersion v{};
        std::sscanf(s.c_str(), "%d.%d.%d", &v.major, &v.minor, &v.patch);
        return v;
    }

    bool CandletoolCli::downloadFile(const std::string&           url,
                                     const std::filesystem::path& outputPath)
    {
        FILE* file = std::fopen(outputPath.string().c_str(), "wb");
        if (!file)
        {
            m_logger.error("Failed to open output file for writing.");
            return false;
        }

        CURL* curl = curl_easy_init();
        if (!curl)
        {
            std::fclose(file);
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "candletool");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        CURLcode res = curl_easy_perform(curl);

        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        curl_easy_cleanup(curl);
        std::fclose(file);

        return (res == CURLE_OK && httpCode == 200);
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
        CURL* curl = curl_easy_init();

        std::string resBody;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resBody);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "candletool");

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            curl_easy_cleanup(curl);
            return std::nullopt;
        }

        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);

        if (httpCode != 200)
            return std::nullopt;

        return resBody;
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
                    m_logger.error("Failed to reach GitHub API.");
                    return;
                }
                nlohmann::json json = nlohmann::json::parse(*body, nullptr, false);
                if (json.is_discarded())
                {
                    m_logger.error("Failed to parse release info.");
                    return;
                }
                std::string latestVersion = json.value("tag_name", std::string{});
                if (latestVersion.empty())
                {
                    m_logger.error("Failed to get response from GitHub.");
                    return;
                }

                CandletoolVersion latestV  = parseVersion(latestVersion);
                CandletoolVersion currentV = parseVersion(currentVersion);

                if (latestV <= currentV)
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
                std::string downloadUrl;
                for (const auto& asset : json["assets"])
                {
                    std::string name = asset.value("name", std::string{});
                    if (name.size() > targetExtension.size() &&
                        name.compare(name.size() - targetExtension.size(),
                                     targetExtension.size(),
                                     targetExtension) == 0)
                    {
                        downloadUrl = asset.value("browser_download_url", std::string{});
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
