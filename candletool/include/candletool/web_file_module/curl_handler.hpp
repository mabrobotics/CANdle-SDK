#pragma once
#include <cstdlib>

#include "mini/ini.h"

#include "candle/types/mab_types.hpp"
#include "candle/logger/logger.hpp"
#include "candletool/web_file_module/web_file.hpp"

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

        CurlHandler(const mINI::INIFile fallbackMetadata);

        std::pair<CurlError_E, WebFile_S> downloadFile(const std::string_view id);

      private:
        Logger m_log = Logger(Logger::ProgramLayer_E::LAYER_2, "CurlHandler");

        const mINI::INIFile m_fallbackMetadata;
        mINI::INIStructure  m_addressLutStructure;

        CurlError_E getLatestLut();

        static inline std::string constructCurlCmd(std::string_view filename,
                                                   std::string_view baseUrl)
        {
            std::stringstream ret;
            ret << "curl --fail -L -o " << filename << " " << baseUrl << filename;
            return ret.str();
        }
    };
}  // namespace mab