#pragma once
#include <cstdlib>

#include "mini/ini.h"

#include "mab_types.hpp"
#include "logger.hpp"
#include "web_file.hpp"

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

        static inline std::string typeToStr(CurlError_E error)
        {
            switch (error)
            {
                case CurlError_E::UNKNOWN_ERROR:
                    return "UNKNOWN_ERROR";
                case CurlError_E::OK:
                    return "OK";
                case CurlError_E::FILE_WRITE_ERROR:
                    return "FILE_WRITE_ERROR";
                case CurlError_E::FILE_READ_ERROR:
                    return "FILE_READ_ERROR";
                case CurlError_E::ADDRESS_NOT_FOUND:
                    return "ADDRESS_NOT_FOUND";
                case CurlError_E::SYSTEM_CALL_ERROR:
                    return "SYSTEM_CALL_ERROR";
                case CurlError_E::UNRECOGNISED_FILETYPE:
                    return "UNRECOGNISED_FILETYPE";
                default:
                    return "INVALID_ENUM_VALUE";
            }
        }

        CurlHandler();

        std::pair<CurlError_E, WebFile_S> downloadFile(const std::string_view id);
        std::string                       getOutputFilePath();
        bool downloadAPIFile(std::string_view url, const std::filesystem::path& outputPath);
        std::optional<std::string> fetchUrl(std::string_view url);
        void                       setFallbackMetadata(const mINI::INIFile& fallbackMetadata);
        std::filesystem::path      createTemporaryPath(std::string_view url);

      private:
        Logger m_logger = Logger(Logger::ProgramLayer_E::LAYER_2, "CurlHandler");

        std::optional<mINI::INIFile> m_fallbackMetadata;
        mINI::INIStructure           m_addressLutStructure;
        std::string                  fullOutputPath;

        static inline std::string constructCurlCmd(std::string_view filename,
                                                   std::string_view baseUrl,
                                                   std::string_view outputPath)
        {
            std::stringstream ret;
            ret << "curl --fail -L -o " << outputPath << " " << baseUrl << filename;
            return ret.str();
        }
    };
}  // namespace mab