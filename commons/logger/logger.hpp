#pragma once
#include <string>
#include <cstdint>
#include <mutex>
#include <iostream>
#include <memory>
#include <array>
#include <optional>

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

    Logger() = default;
    Logger(const Logger& logger);

    /// @brief header to display where the information came from if using standard logger functions
    std::string m_tag = "";
    /// @brief abstraction layer of the logger, influences how verbose the module will be. TOP is
    /// the most verbose
    ProgramLayer_E m_layer = ProgramLayer_E::TOP;
    /// @brief verbosity of the whole program. Entered via CLI.
    inline static std::optional<Verbosity_E> g_m_verbosity;
    /// @brief by assigning this variable, custom LogLevel will be set. It will override regular
    /// layer/verbosity level.
    std::optional<LogLevel_E> m_optionalLevel;

    template <typename T>
    [[nodiscard]] static bool setStream(T path_);

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
        std::lock_guard<std::mutex> lock(g_m_printfLock);
        std::cout << value;
        return *this;
    }
    /// @brief special overload case for stream manipulators (like std::endl)
    /// @param manip stream manipulator
    /// @return instance of the logger
    Logger& operator<<(std::ostream& (*manip)(std::ostream&))
    {
        if (getCurrentLevel() == LogLevel_E::SILENT)
            return *this;
        std::lock_guard<std::mutex> lock(g_m_printfLock);
        std::cout << manip;
        return *this;
    }

  private:
    /// @brief global mutex for stream access
    inline static std::mutex g_m_printfLock;

    void printLog(FILE* stream, const char* header, const char* msg, va_list args);

    inline static std::optional<FILE*> g_m_streamOverride;
};