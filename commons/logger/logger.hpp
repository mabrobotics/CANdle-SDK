#pragma once
#include <string>
#include <cstdint>
#include <mutex>
#include <array>
#include <optional>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iostream>

#ifndef _WIN32

#define RED      "\x1b[38;5;196m"
#define GREEN    "\x1b[38;5;46m"
#define BLUE     "\x1b[38;5;33m"
#define YELLOW   "\x1b[38;5;226m"
#define ORANGE   "\x1b[38;5;202m"
#define GREY     "\x1b[38;5;240m"
#define RED      "\x1b[38;5;196m"
#define GREEN    "\x1b[38;5;46m"
#define BLUE     "\x1b[38;5;33m"
#define YELLOW   "\x1b[38;5;226m"
#define ORANGE   "\x1b[38;5;202m"
#define GREY     "\x1b[38;5;240m"
#define RESETCLR "\x1b[0m"
#define NEW_LINE "\n"

#else

#define RED      ""
#define GREEN    ""
#define BLUE     ""
#define YELLOW   ""
#define ORANGE   ""
#define GREY     ""
#define RESETCLR ""
#define NEW_LINE "\r\n"

#endif

using std::cout, std::endl;

class Logger
{
    using preferredClock_t          = std::chrono::high_resolution_clock;
    using preferredClockTimepoint_t = std::chrono::time_point<preferredClock_t>;

  public:
    enum class LogLevel_E : uint8_t
    {
        DEBUG  = 0,
        INFO   = 10,
        WARN   = 20,
        ERROR_ = 30,  // WINDOWS does not tollerate ERROR word in code
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
        TOP     = 0,
        LAYER_2 = 1,
        BOTTOM  = 2
    };
    enum class MessageType_E : uint8_t
    {
        INFO = 0,
        SUCCESS,
        DEBUG,
        WARN,
        ERROR_  // WINDOWS does not tollerate ERROR word in code
    };
    Logger() = default;
    /// @brief constructor with all the necessary parameters
    /// @param programLayer layer on which the logger instance is living (lower layers are less
    /// verbose)
    /// @param tag header on messages
    Logger(ProgramLayer_E programLayer, std::string tag);
    Logger(const Logger& logger);
    ~Logger() = default;

    static constexpr std::array<std::array<LogLevel_E, 3>, 5> g_m_verbosityTable{
        // TOP, LAYER_2, BOTTOM
        {{{LogLevel_E::INFO, LogLevel_E::WARN, LogLevel_E::ERROR_}},        // DEFAULT
         {{LogLevel_E::INFO, LogLevel_E::INFO, LogLevel_E::WARN}},          // V1
         {{LogLevel_E::DEBUG, LogLevel_E::INFO, LogLevel_E::INFO}},         // V2
         {{LogLevel_E::DEBUG, LogLevel_E::DEBUG, LogLevel_E::DEBUG}},       // V3
         {{LogLevel_E::SILENT, LogLevel_E::SILENT, LogLevel_E::SILENT}}}};  // SILENT

    /// @brief abstraction layer of the logger, influences how verbose the module will be. TOP is
    /// the most verbose
    ProgramLayer_E m_layer = ProgramLayer_E::TOP;
    /// @brief header informing where the information came from
    std::string m_tag = "";
    /// @brief verbosity of the whole program, global
    inline static std::optional<Verbosity_E> g_m_verbosity;
    /// @brief by assigning this variable, custom LogLevel will be set. It will override regular
    /// layer/verbosity levels.
    std::optional<LogLevel_E> m_optionalLevel;

    /// @brief Log redirect function, can be used to stream to files
    /// @param path_ path to file
    /// @return operation successful on true
    [[nodiscard]] static bool setStream(const char* path_);

    /// @brief special logger function to display progress bar
    void progress(double percentage) const;

    // standard logger functions with formatting, printf syntax
    void info(const char* msg, ...) const;
    void success(const char* msg, ...) const;
    void debug(const char* msg, ...) const;
    void warn(const char* msg, ...) const;
    void error(const char* msg, ...) const;

    /// @brief
    /// @return current log level of this instance
    LogLevel_E getCurrentLevel() const;

    /// @brief overload for << operator fort writing to std output, prints on newline characters
    /// @tparam T type to stream, like const char *
    /// @param value value to stream
    /// @return instance of logger
    template <typename T>
    Logger& operator<<(const T& value)
    {
        if (getCurrentLevel() == LogLevel_E::SILENT)
            return *this;

        std::stringstream buffer;
        buffer << value;

        constexpr char termination = *NEW_LINE;
        // Process buffer contents character by character
        for (auto& character : buffer.str())
        {
            if (character == termination)
            {
                info(m_internalStrBuffer.str().c_str());
                m_internalStrBuffer.str("");
            }
            else
            {
                m_internalStrBuffer << character;
            }
        }
        return *this;
    }
    /// @brief special overload case for stream manipulators (like std::endl), prints on newline
    /// characters
    /// @param manip stream manipulator
    /// @return instance of the logger
    Logger& operator<<(std::ostream& (*manip)(std::ostream&))
    {
        if (getCurrentLevel() == LogLevel_E::SILENT)
            return *this;

        // Apply the manipulator to the internal stream
        manip(m_internalStrBuffer);

        // Check if the manipulator inserted a newline
        constexpr char newlineChar = *NEW_LINE;
        auto           newlinePos  = m_internalStrBuffer.str().find(newlineChar);

        if (newlinePos != std::string::npos)
        {
            std::string temp = m_internalStrBuffer.str();
            m_internalStrBuffer.str("");
            // Remove all newline characters from the string
            temp.erase(std::remove(temp.begin(), temp.end(), newlineChar), temp.end());
            info(temp.c_str());
        }

        return *this;
    }

  private:
    inline static std::mutex g_m_printfLock;  // global print lock
    inline static std::optional<FILE*>
        g_m_streamOverride;  // if set all streams will be redirected here

    void        printLogLine(FILE* stream, const char* header, const char* msg, va_list args) const;
    void        printLogLine(FILE* stream, const char* header, const char* msg) const;
    void        printLog(FILE* stream, const char* header, const char* msg, va_list args) const;
    void        printLog(FILE* stream, const char* header, const char* msg) const;
    std::string generateHeader(Logger::MessageType_E messageType) const noexcept;

    std::stringstream m_internalStrBuffer;

    static inline bool printSpecials()
    {
        return !g_m_streamOverride.has_value();
    }
};
