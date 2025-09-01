#pragma once

#include "web_file.hpp"
#include "mab_types.hpp"
#include "utilities.hpp"
#include "logger.hpp"

namespace mab
{
    class Flasher : public WebFile_S
    {
      public:
        enum class Error_E
        {
            UNKNOWN,
            OK,
            NO_FILE,
            EXECUTION_ERROR
        };

        Flasher(const WebFile_S& webFile) : WebFile_S(webFile)
        {
        }

        Error_E flash(canId_t canId, bool recovery);

      private:
        Logger m_log = Logger(Logger::ProgramLayer_E::LAYER_2, "FLASHER");
    };
}  // namespace mab