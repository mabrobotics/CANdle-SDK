#include <filesystem>
#include "curl_handler.hpp"
#include "utilities.hpp"

namespace mab
{
    CurlHandler::CurlHandler(const mINI::INIFile fallbackMetadata)
        : m_fallbackMetadata(fallbackMetadata)
    {
    }

    std::pair<CurlHandler::CurlError_E, WebFile_S> CurlHandler::downloadFile(
        const std::string_view id)
    {
        m_log.info("Downloading file [ %s ]", id.data());
        WebFile_S webFile;
        webFile.m_path = std::filesystem::current_path();

        const mINI::INIFile* file;
        // Try to get the latest LUT from the server
        CurlError_E result = getLatestLut();
        if (result != CurlError_E::OK)
        {
            m_log.warn(
                "Could not get the latest LUT from the MAB servers, falling back to local LUT");
            file = &m_fallbackMetadata;
        }

        // If failed, fall back to the local LUT file
        if (file == nullptr || !file->read(m_addressLutStructure))
        {
            m_log.error("Failed to read the LUT file");
            return std::make_pair(CurlError_E::FILE_READ_ERROR, webFile);
        }

        // DEBUG PRINT LUT
        for (const auto& section : m_addressLutStructure)
        {
            m_log.debug("Name: %s", section.first.c_str());
            for (const auto& pair : section.second)
            {
                m_log.debug("  %s = %s", pair.first.c_str(), pair.second.c_str());
            }
        }

        std::string typeString = m_addressLutStructure[id.data()]["type"];
        auto        type       = WebFile_S::strToType(typeString);
        if (type == WebFile_S::Type_E::UNKNOWN)
        {
            m_log.error("Could not recognise filetype!");
            return std::make_pair(CurlError_E::UNRECOGNISED_FILETYPE, webFile);
        }
        webFile.m_type = type;

        // Look for the address and filename in the LUT structure
        std::string baseUrl  = m_addressLutStructure[id.data()]["base_url"];
        std::string filename = m_addressLutStructure[id.data()]["filename"];
        // For multiarch entries
        if (filename.empty())
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
                m_log.warn("No architecture specific filename found");
            filename = m_addressLutStructure[id.data()][filename_field];
        }
        if (!baseUrl.empty() && !filename.empty())
        {
            m_log.info("Found URL [ %s ] for file [ %s ]", baseUrl.c_str(), filename.data());
            std::string command = "curl -L -o " + filename + " " + baseUrl + filename;
            bool        result  = executeCommand(command);
            if (result)
            {
                m_log.error("Failed to download file [ %s ] from URL [ %s ]",
                            filename.data(),
                            (baseUrl + filename).c_str());
                return std::make_pair(CurlError_E::SYSTEM_CALL_ERROR, webFile);
            }
            m_log.success("Successfully downloaded file [ %s ]", id.data());
            webFile.m_path.append(filename);
            return std::make_pair(CurlError_E::OK, webFile);
        }

        m_log.error("Could not find URL for file [ %s ] in LUT", id.data());
        return std::make_pair(CurlError_E::ADDRESS_NOT_FOUND, webFile);
    }

    CurlHandler::CurlError_E CurlHandler::getLatestLut()
    {
        // Todo: implement fetching lut when ready
        return CurlError_E::ADDRESS_NOT_FOUND;
    }
}  // namespace mab