#include <logger.hpp>
#include <stdarg.h>
#include <cmath>
#include <mutex>
#include <cstring>

Logger ::Logger(const Logger& logger_)
{
    m_layer = logger_.m_layer;
    m_tag   = logger_.m_tag;
}

bool Logger ::setStream(const char* path_)
{
    if (Logger::g_m_streamOverride.has_value())
    {
        fclose(Logger::g_m_streamOverride.value());
        Logger::g_m_streamOverride.reset();
    }

    FILE* fileRaw = fopen(path_, "w");
    if (fileRaw == nullptr)
    {
        return false;
    }

    Logger::g_m_streamOverride = fileRaw;
    return true;
}

/* progress bar */
#define PBSTR   "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

void Logger::progress(double percentage)
{
    if (getCurrentLevel() == LogLevel_E::SILENT)
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

void Logger::ui(const char* msg, ...)
{
    if (getCurrentLevel() == LogLevel_E::SILENT)
        return;

    va_list args;
    va_start(args, msg);
    Logger::printLog(stdout, "", msg, args);
    va_end(args);
}

void Logger::info(const char* msg, ...)
{
    if (getCurrentLevel() > LogLevel_E::INFO)
        return;

    const std::string header("[" BLUE "INFO" RESETCLR "][" + m_tag + "] ");

    va_list args;
    va_start(args, msg);
    Logger::printLog(stdout, header.c_str(), msg, args);
    va_end(args);
}

void Logger::success(const char* msg, ...)
{
    if (getCurrentLevel() > LogLevel_E::INFO)
        return;

    const std::string header("[" GREEN " OK " RESETCLR "][" + m_tag + "] ");

    va_list args;
    va_start(args, msg);
    Logger::printLog(stdout, header.c_str(), msg, args);
    va_end(args);
}

void Logger::debug(const char* msg, ...)
{
    if (getCurrentLevel() > LogLevel_E::DEBUG)
        return;

    const std::string header("[" ORANGE "DEBUG" RESETCLR "][" + m_tag + "] ");

    va_list args;
    va_start(args, msg);
    Logger::printLog(stdout, header.c_str(), msg, args);
    va_end(args);
}

void Logger::warn(const char* msg, ...)
{
    if (getCurrentLevel() > LogLevel_E::WARN)
        return;

    const std::string header("[" YELLOW "WARNING" RESETCLR "][" + m_tag + "] ");

    va_list args;
    va_start(args, msg);
    Logger::printLog(stderr, header.c_str(), msg, args);
    va_end(args);
}

void Logger::error(const char* msg, ...)
{
    if (getCurrentLevel() > LogLevel_E::ERROR)
        return;

    const std::string header("[" RED "ERROR" RESETCLR "][" + m_tag + "] ");

    va_list args;
    va_start(args, msg);
    Logger::printLog(stderr, header.c_str(), msg, args);
    va_end(args);
}

Logger::LogLevel_E Logger::getCurrentLevel()
{
    if (m_optionalLevel.has_value())
        return m_optionalLevel.value();
    else
        return g_m_verbosityTable[static_cast<uint8_t>(
            g_m_verbosity.value_or(Logger::Verbosity_E::DEFAULT))][static_cast<uint8_t>(m_layer)];
}

void Logger::printLog(FILE* stream, const char* header, const char* msg, va_list args)
{
    std::lock_guard<std::mutex> lock(g_m_printfLock);

    fprintf(g_m_streamOverride.value_or(stream), header, this->m_tag.c_str());
    vfprintf(g_m_streamOverride.value_or(stream), msg, args);
    fprintf(g_m_streamOverride.value_or(stream), NEW_LINE);
}