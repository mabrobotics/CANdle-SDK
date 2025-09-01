#pragma once
#include <cstdlib>

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
            SYSTEM_CALL_ERROR
        };

        CurlHandler(const mINI::INIFile fallbackAddressLut);

        std::pair<CurlError_E, std::filesystem::path> downloadFile(const std::string_view id);

      private:
        Logger m_log = Logger(Logger::ProgramLayer_E::LAYER_2, "CurlHandler");

        const mINI::INIFile m_fallbackAddressLut;
        mINI::INIStructure  m_addressLutStructure;

        CurlError_E getLatestLut();
    };
}  // namespace mab