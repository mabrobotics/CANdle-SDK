#include <filesystem>
#include "curl_handler.hpp"
#include "utilities.hpp"

#include <curl/curl.h>

namespace mab
{
    CurlHandler::CurlHandler()
    {
    }

    void CurlHandler::setFallbackMetadata(const mINI::INIFile& fallbackMetadata)
    {
        m_fallbackMetadata = std::move(fallbackMetadata);
    }

    static size_t curlCallback(char* ptr, size_t size, size_t nmemb, std::string* out)
    {
        out->append(ptr, size * nmemb);
        return size * nmemb;
    }

    static size_t writeToFileCallback(char* ptr, size_t size, size_t nmemb, FILE* file)
    {
        return fwrite(ptr, size, nmemb, file);
    }

    std::filesystem::path CurlHandler::createTemporaryPath(std::string_view url)
    {
        std::string           urlToStr(url);
        std::filesystem::path tmpDirectory        = std::filesystem::temp_directory_path();
        std::string           iniFileDownloadName = urlToStr.substr(urlToStr.find_last_of('/') + 1);
        std::filesystem::path outputPath          = tmpDirectory / iniFileDownloadName;

        return outputPath;
    }

    std::optional<std::string> CurlHandler::fetchUrl(std::string_view url)
    {
        CURL*       curl = curl_easy_init();
        std::string resBody;
        std::string urlToStr(url);

        curl_easy_setopt(curl, CURLOPT_URL, urlToStr.c_str());
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

    bool CurlHandler::downloadAPIFile(std::string_view url, const std::filesystem::path& outputPath)
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
        std::string urlToStr(url);

        curl_easy_setopt(curl, CURLOPT_URL, urlToStr.c_str());
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

    std::string CurlHandler::getOutputFilePath()
    {
        std::string outputPath = fullOutputPath;
        return outputPath;
    }

    std::pair<CurlHandler::CurlError_E, WebFile_S> CurlHandler::downloadFile(
        const std::string_view id)
    {
        m_logger.info("Downloading file [ %s ]", id.data());
        WebFile_S webFile;
        webFile.m_path = std::filesystem::temp_directory_path();

        const mINI::INIFile* file = nullptr;
        if (m_fallbackMetadata.has_value())
        {
            file = &m_fallbackMetadata.value();
        }
        else
        {
            m_logger.error("Error with .ini file!");
            return std::make_pair(CurlError_E::FILE_READ_ERROR, webFile);
        }

        // If failed, fall back to the local LUT file
        if (file == nullptr || !file->read(m_addressLutStructure))
        {
            m_logger.error("Failed to read the LUT file");
            return std::make_pair(CurlError_E::FILE_READ_ERROR, webFile);
        }

        // DEBUG PRINT LUT
        for (const auto& section : m_addressLutStructure)
        {
            m_logger.debug("Name: %s", section.first.c_str());
            for (const auto& pair : section.second)
            {
                m_logger.debug("  %s = %s", pair.first.c_str(), pair.second.c_str());
            }
        }

        std::string typeString = m_addressLutStructure[id.data()]["type"];
        auto        type       = WebFile_S::strToType(typeString);
        if (type == WebFile_S::Type_E::UNKNOWN)
        {
            m_logger.error("Could not recognise filetype or no such ID in metainfo file!");
            return std::make_pair(CurlError_E::UNRECOGNISED_FILETYPE, webFile);
        }
        webFile.m_type = type;

        // Look for the address and filename in the LUT structure
        std::string baseUrl       = m_addressLutStructure[id.data()]["base_url"];
        std::string baseUrlMirror = m_addressLutStructure[id.data()]["base_url_mirror"];
        std::string filename      = m_addressLutStructure[id.data()]["filename"];
        // For multiarch entries
        if (filename.empty())
        {
            if (type == WebFile_S::Type_E::MD_FLASHER)
            {
                constexpr sysArch_E arch           = getSysArch();
                std::string         filename_field = "filename_";

                if constexpr (arch == sysArch_E::ARM64)
                    filename_field += "arm64";
                else if constexpr (arch == sysArch_E::ARMHF)
                    filename_field += "armhf";
                else if constexpr (arch == sysArch_E::X86_64)
                    filename_field += "x86_64";
                else
                    m_logger.warn("No architecture specific filename found");
                filename = m_addressLutStructure[id.data()][filename_field];
            }
        }
        if (!baseUrl.empty() && !filename.empty())
        {
            m_logger.info("Found URL [ %s ] for file [ %s ]", baseUrl.c_str(), filename.data());
            fullOutputPath      = (webFile.m_path / filename).string();
            std::string command = constructCurlCmd(filename, baseUrl, fullOutputPath);
            bool        result  = executeCommand(command);
            if (result)
            {
                m_logger.warn("Failed to download file [ %s ] from URL [ %s ]",
                              filename.data(),
                              (baseUrl + filename).c_str());
                m_logger.warn("Trying mirror...");
                command = constructCurlCmd(filename, baseUrlMirror, fullOutputPath);
                result  = executeCommand(command);

                if (result)
                {
                    m_logger.error("Failed to download file [ %s ] from URL [ %s ]",
                                   filename.data(),
                                   (baseUrlMirror + filename).c_str());
                    return std::make_pair(CurlError_E::SYSTEM_CALL_ERROR, webFile);
                }
            }
            m_logger.success("Successfully downloaded file [ %s ]", id.data());
            webFile.m_path.append(filename);
            return std::make_pair(CurlError_E::OK, webFile);
        }

        m_logger.error("Could not find URL for file [ %s ] in LUT", id.data());
        return std::make_pair(CurlError_E::ADDRESS_NOT_FOUND, webFile);
    }
}  // namespace mab