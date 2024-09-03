#pragma once
#include <string>
#include <cstdint>
#include <mutex>
#include <iostream>
#include <fstream>
#include <memory>
#include <array>
#include <optional>
#include <functional>
#include <sstream>
#include <algorithm>

#ifndef _WIN32

#define RED      "\x1b[38;5;196m"
#define GREEN    "\x1b[38;5;46m"
#define BLUE     "\x1b[38;5;33m"
#define YELLOW   "\x1b[38;5;226m"
#define ORANGE   "\x1b[38;5;202m"
#define GREY     "\x1b[38;5;240m"
#define RESETCLR "\x1b[0m"
#define NEW_LINE "\n"

#else

#define RED
#define GREEN
#define BLUE
#define YELLOW
#define ORANGE
#define GREY
#define RESETCLR
#define NEW_LINE "\r\n"

#endif

class Logger
{
  public:
    Logger() = default;
    Logger(const Logger& logger);

    enum class LogLevel_E : uint8_t
    {
        DEBUG  = 0,
        INFO   = 10,
        WARN   = 20,
        ERROR  = 30,
        SILENT = 255
    };
    enum class Verbosity_E : uint8_t
    {
        DEFAULT     = 0,
        VERBOSITY_1 = 1,
        VERBOSITY_2 = 2,
        VERBOSITY_3 = 3,
        SILENT      = 4  // this parameter must always be last
    };
    enum class ProgramLayer_E : uint8_t
    {
        TOP    = 0,
        MIDDLE = 1,
        BOTTOM = 2
    };

    static constexpr std::array<std::array<LogLevel_E, 3>, 5> g_m_verbosityTable{
        // TOP, MIDDLE, BOTTOM
        {{{LogLevel_E::INFO, LogLevel_E::WARN, LogLevel_E::ERROR}},         // DEFAULT
         {{LogLevel_E::INFO, LogLevel_E::INFO, LogLevel_E::WARN}},          // V1
         {{LogLevel_E::DEBUG, LogLevel_E::INFO, LogLevel_E::INFO}},         // V2
         {{LogLevel_E::DEBUG, LogLevel_E::DEBUG, LogLevel_E::DEBUG}},       // V3
         {{LogLevel_E::SILENT, LogLevel_E::SILENT, LogLevel_E::SILENT}}}};  // SILENT

    /// @brief abstraction layer of the logger, influences how verbose the module will be. TOP is
    /// the most verbose
    ProgramLayer_E m_layer = ProgramLayer_E::TOP;
    /// @brief header to display where the information came from if using standard logger functions
    std::string m_tag = "";
    /// @brief verbosity of the whole program. Entered via CLI.
    inline static std::optional<Verbosity_E> g_m_verbosity;
    /// @brief by assigning this variable, custom LogLevel will be set. It will override regular
    /// layer/verbosity level.
    std::optional<LogLevel_E> m_optionalLevel;

    [[nodiscard]] static bool setStream(const char* path_);

    /// @brief special logger function to display progress bar
    void progress(double percentage);
    /// @brief special logger function to display ui info without header
    void ui(const char* msg, ...);

    // standard logger functions with formatting
    void info(const char* msg, ...);
    void success(const char* msg, ...);
    void debug(const char* msg, ...);
    void warn(const char* msg, ...);
    void error(const char* msg, ...);

    /// @brief helper function
    /// @return current log level
    LogLevel_E getCurrentLevel();

    /// @brief overload for << operator fort writing to std output
    /// @tparam T type to stream
    /// @param value value to stream
    /// @return instance of logger
    template <typename T>
    Logger& operator<<(const T& value)
    {
        if (getCurrentLevel() == LogLevel_E::SILENT)
            return *this;

        std::stringstream buffer;
        buffer << value;

        // Process buffer contents character by character
        for (auto& character : buffer.str())
        {
            constexpr char termination = '\n';
            if (character == termination)
            {
                info(m_str.str().c_str());
                m_str.str("");
            }
            else
            {
                m_str << character;
            }
        }
        return *this;
    }
    /// @brief special overload case for stream manipulators (like std::endl)
    /// @param manip stream manipulator
    /// @return instance of the logger
    Logger& operator<<(std::ostream& (*manip)(std::ostream&))
    {
        if (getCurrentLevel() == LogLevel_E::SILENT)
            return *this;

        // Invoke the manipulator on the internal stream
        manip(m_str);

        // Check if the manipulator inserted a newline, if so flush the buffer
        if (m_str.str().find('\n') != std::string::npos)
        {
            info(m_str.str().c_str());
            m_str.str("");
        }

        return *this;
    }

  private:
    /// @brief global mutex for stream access
    inline static std::mutex g_m_printfLock;

    void printLog(FILE* stream, const char* header, const char* msg, va_list args);

    inline static std::optional<FILE*> g_m_streamOverride;

    std::stringstream m_str;
};
