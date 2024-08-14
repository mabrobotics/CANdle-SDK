#pragma once
#include <string>
#include <cstdint>
#include <atomic>
#include <optional>
#include <mutex>

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

    LogLevel_E  m_level = LogLevel_E::INFO;
    std::string m_tag   = "";

    void progress(double percentage);

    void ui(const char* msg, ...);
    void info(const char* msg, ...);
    void success(const char* msg, ...);
    void debug(const char* msg, ...);
    void warn(const char* msg, ...);
    void error(const char* msg, ...);

  private:
    inline static std::mutex g_m_printfLock;
    void                     print(FILE* steam, const char* header, const char* msg, va_list args);
};