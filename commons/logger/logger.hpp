#pragma once
#include <string>
#include <cstdint>

#ifndef _WIN32

#define RED      "\x1b[38;5;196m"
#define GREEN    "\x1b[38;5;46m"
#define BLUE     "\x1b[38;5;33m"
#define YELLOW   "\x1b[38;5;226m"
#define ORANGE   "\x1b[38;5;202m"
#define GREY     "\x1b[38;5;240m"
#define RESETCLR "\x1b[0m"

#else

#define RED
#define GREEN
#define BLUE
#define YELLOW
#define ORANGE
#define GREY
#define RESETCLR

#endif

struct logger
{
    enum class LogLevel_E : uint8_t
    {
        DEBUG  = 0,
        INFO   = 10,
        WARN   = 20,
        ERROR  = 30,
        SILENT = 255
    };
    LogLevel_E  level = LogLevel_E::INFO;
    std::string tag   = "";

    void info(const char* msg, ...);

    /**
     * @brief Print a message without any prefix or newline
     */
    void info_raw(const char* msg, ...);
    void success(const char* msg, ...);
    void debug(const char* msg, ...);
    void warn(const char* msg, ...);
    void error(const char* msg, ...);
};