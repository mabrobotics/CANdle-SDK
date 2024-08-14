#include <logger.hpp>
#include <stdarg.h>
#include <cmath>
#include <mutex>

void Logger::ui(const char* msg, ...)
{
    if (m_level == LogLevel_E::SILENT)
        return;

    va_list args;
    va_start(args, msg);
    Logger::print(stderr, "", msg, args);
    va_end(args);
}

void Logger::info(const char* msg, ...)
{
    if (m_level > LogLevel_E::INFO)
        return;

    const std::string header("[" BLUE "INFO" RESETCLR "][" + m_tag + "] ");

    va_list args;
    va_start(args, msg);
    Logger::print(stderr, header.c_str(), msg, args);
    va_end(args);
}

/* progress bar */
#define PBSTR   "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

void Logger::progress(double percentage)
{
    if (m_level == LogLevel_E::SILENT)
        return;
    uint16_t val  = (uint16_t)(percentage * 100);
    uint16_t lpad = (uint16_t)(percentage * PBWIDTH);
    uint16_t rpad = PBWIDTH - lpad;

    std::lock_guard<std::mutex> lock(g_m_printfLock);

    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    if (fabs(percentage - 1.0) < 0.00001)
        printf("\r\n");
    fflush(stdout);
}

void Logger::success(const char* msg, ...)
{
    if (m_level > LogLevel_E::INFO)
        return;

    const std::string header("[" GREEN " OK " RESETCLR "][" + m_tag + "] ");

    va_list args;
    va_start(args, msg);
    Logger::print(stderr, header.c_str(), msg, args);
    va_end(args);
}

void Logger::debug(const char* msg, ...)
{
    if (m_level > LogLevel_E::DEBUG)
        return;

    const std::string header("[" ORANGE "DEBUG" RESETCLR "][" + m_tag + "] ");

    va_list args;
    va_start(args, msg);
    Logger::print(stderr, header.c_str(), msg, args);
    va_end(args);
}

void Logger::warn(const char* msg, ...)
{
    if (m_level > LogLevel_E::WARN)
        return;

    const std::string header("[" YELLOW "WARNING" RESETCLR "][" + m_tag + "] ");

    va_list args;
    va_start(args, msg);
    Logger::print(stderr, header.c_str(), msg, args);
    va_end(args);
}

void Logger::error(const char* msg, ...)
{
    if (m_level > LogLevel_E::ERROR)
        return;

    const std::string header("[" RED "ERROR" RESETCLR "][" + m_tag + "] ");

    va_list args;
    va_start(args, msg);
    Logger::print(stderr, header.c_str(), msg, args);
    va_end(args);
}

void Logger::print(FILE* steam, const char* header, const char* msg, va_list args)
{
    std::lock_guard<std::mutex> lock(g_m_printfLock);
    fprintf(steam, header, this->m_tag.c_str());
    vfprintf(steam, msg, args);
    fprintf(steam, NEW_LINE);
}
