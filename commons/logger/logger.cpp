#include <logger.hpp>
#include <stdarg.h>
#include <cmath>
#include <mutex>
#include <cstring>

Logger ::Logger()
{
    // first initialization of logger is a base point
    if (Logger::g_m_start == nullptr)
    {
        Logger::g_m_start =
            std::make_unique<Logger::preferredClockTimepoint_t>(Logger::preferredClock_t::now());
    }
}

Logger ::Logger(const Logger& logger_) : m_layer(logger_.m_layer), m_tag(logger_.m_tag)
{
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

void Logger::info(const char* msg, ...)
{
    if (getCurrentLevel() > LogLevel_E::INFO)
        return;

    const std::string header(generateHeader(MessageType_E::INFO).c_str());

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

std::string Logger ::generateHeader(Logger::MessageType_E messageType) const noexcept
{
    std::string header;
    using MT_E = Logger::MessageType_E;
    switch (messageType)
    {
        case MT_E::INFO:
            header = "[" BLUE "INFO" RESETCLR "]";
            break;
        case MT_E::DEBUG:
            header = "[" ORANGE "DEBUG" RESETCLR "]";
            break;
        case MT_E::SUCCESS:
            header = "[" GREEN "SUCCESS" RESETCLR "]";
            break;
        case MT_E::WARN:
            header = "[" BLUE "WARN" RESETCLR "]";
            break;
        case MT_E::ERROR:
            header = "[" BLUE "ERROR" RESETCLR "]";
            break;
        default:
            break;
    }
    header.append("[" + m_tag + "]");
    const uint32_t durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                    (preferredClock_t::now() - *g_m_start))
                                    .count();
    const uint32_t durationSec = durationMs / 1000u;
    const uint32_t durationMin = durationSec / 60u;
    const uint32_t durationHr  = durationMin / 60u;

    const std::string timestamp = "[" + std::to_string(durationHr) + ":" +
                                  std::to_string(durationMin) + ":" + std::to_string(durationSec) +
                                  "." + std::to_string(durationMs) + "] ";

    header.append(timestamp);

    return header;
}