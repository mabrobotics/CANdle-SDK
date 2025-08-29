#include <filesystem>
#include "curl_handler.hpp"
#include "utilities.hpp"

namespace mab
{
    CurlHandler::CurlHandler(const mINI::INIFile fallbackAddressLut)
        : m_fallbackAddressLut(fallbackAddressLut)
    {
    }

    std::pair<CurlHandler::CurlError_E, std::filesystem::path> CurlHandler::downloadFile(
        const std::string_view name)
    {
        m_log.info("Downloading file [ %s ]", name.data());
        std::filesystem::path pathDownloadedFile = std::filesystem::current_path();

        const mINI::INIFile* file;
        // Try to get the latest LUT from the server
        CurlError_E result = getLatestLut();
        if (result != CurlError_E::OK)
        {
            m_log.warn(
                "Could not get the latest LUT from the MAB servers, falling back to local LUT");
            file = &m_fallbackAddressLut;
        }

        // If failed, fall back to the local LUT file
        if (!file->read(m_addressLutStructure) || file == nullptr)
        {
            m_log.error("Failed to read the LUT file");
            return std::make_pair(CurlError_E::FILE_READ_ERROR, pathDownloadedFile);
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

        // Look for the address and filename in the LUT structure
        std::string baseUrl  = m_addressLutStructure[name.data()]["base_url"];
        std::string filename = m_addressLutStructure[name.data()]["filename"];
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
                return std::make_pair(CurlError_E::SYSTEM_CALL_ERROR, pathDownloadedFile);
            }
            m_log.success("Successfully downloaded file [ %s ]", name.data());
            return std::make_pair(CurlError_E::OK, pathDownloadedFile.append(filename));
        }

        m_log.error("Could not find URL for file [ %s ] in LUT", name.data());
        return std::make_pair(CurlError_E::ADDRESS_NOT_FOUND, pathDownloadedFile);
    }

    CurlHandler::CurlError_E CurlHandler::getLatestLut()
    {
        // Todo: implement fetching lut when ready
        return CurlError_E::ADDRESS_NOT_FOUND;
    }
}  // namespace mab