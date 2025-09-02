#include "flasher.hpp"

namespace mab
{
    Flasher::Error_E Flasher::flash(canId_t canId, bool recovery)
    {
        m_log.warn("This is a legacy method of installing firmware.");
        m_log.warn("It only works on linux, and the flashed device");
        m_log.warn("should be the only device present in the CAN network.");
        bool        err = true;
        std::string cmd = "chmod a+x ";

        // Validity checks
        if (m_path.empty())
        {
            m_log.error("No path to the file!");
            return Error_E::NO_FILE;
        }
        else if (m_type != Type_E::CANDLE_FLASHER && m_type != Type_E::MD_FLASHER)
        {
            m_log.error("Invalid file type: %s", typeToStr(m_type).c_str());
            return Error_E::EXECUTION_ERROR;
        }

        // Permissions
        cmd += m_path;
        err = executeCommand(cmd);
        if (err)
        {
            m_log.error("Error setting permissions!");
            return Error_E::EXECUTION_ERROR;
        }

        // Flashing
        cmd = m_path;
        if (m_type == Type_E::MD_FLASHER)
        {
            cmd += " --id ";
            cmd += std::to_string(canId);
            cmd += " --baud 1M ";
            if (recovery)
                cmd += "--wait";
        }
        err = executeCommand(cmd);
        if (err)
        {
            m_log.error("Error executing flasher device!");
            return Error_E::EXECUTION_ERROR;
        }
        return Error_E::OK;
    }

}  // namespace mab