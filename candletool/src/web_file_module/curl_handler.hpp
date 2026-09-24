#pragma once
#include <cstdlib>
#include <filesystem>
#include <string>
#include <string_view>

#include "mini/ini.h"

#include "mab_types.hpp"
#include "logger.hpp"

namespace mab
{
    class CurlHandler
    {
      public:
        enum class CurlError_E : u8
        {
            UNKNOWN_ERROR,
            OK,
            FILE_WRITE_ERROR,
            FILE_READ_ERROR,
            ADDRESS_NOT_FOUND,
            SYSTEM_CALL_ERROR,
            UNRECOGNISED_FILETYPE
        };

        // Firmware server, main server
        // (https://mabrobotics.github.io/MD80-x-CANdle-Documentation/) has the same layout
        static constexpr const char* FW_SERVER_ROOT =
            "https://mabrobotics.github.io/mab-documentation-devel-deploy/_static/firmware/";
        static constexpr const char* FW_INDEX_FILE = "api_download.ini";

        /// @brief download a single file from url to outputPath
        static CurlError_E download(std::string_view url, const std::filesystem::path& outputPath);

        /// @brief download firmware index from the server and parse it
        static bool loadIndex(mINI::INIStructure& index);

        /// @brief get key of the first index entry named <prefix><version> or
        /// <prefix><version>_<suffix>, version "latest" matches <prefix>..._latest
        /// @param prefix lowercase, index section names are lowercased by mINI
        static std::string findIndexEntry(const mINI::INIStructure& index,
                                          const std::string&        prefix,
                                          const std::string&        version,
                                          const char*               key);
    };
}  // namespace mab
