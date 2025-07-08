#include <logger.hpp>
#include <stdarg.h>
#include <cmath>
#include <mutex>
#include <cstring>
#include <iomanip>

// PUBLICS

Logger ::Logger(ProgramLayer_E programLayer, std::string tag) : m_layer(programLayer), m_tag(tag)
{
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

void Logger::progress(double percentage) const
{
    if (getCurrentLevel() == LogLevel_E::SILENT)
        return;
    const uint8_t val  = (uint8_t)(percentage * 100);
    const uint8_t lpad = (uint8_t)(percentage * PBWIDTH);
    const uint8_t rpad = PBWIDTH - lpad;

    const char* progBarTemplate = "%3d%% [%.*s%*s]";
    char        progBar[512];
    snprintf(progBar, sizeof(progBar), progBarTemplate, val, lpad, PBSTR, rpad, "");

    printLog(stdout, "", "\r");
    printLog(stdout, generateHeader(MessageType_E::INFO).c_str(), progBar);
    if (fabs(percentage - 1.0) < 0.00001)
        printLogLine(stdout, "", "\r");
}

void Logger::info(const char* msg, ...) const
{
    if (getCurrentLevel() > LogLevel_E::INFO)
        return;

    const std::string header(generateHeader(MessageType_E::INFO).c_str());

    va_list args;
    va_start(args, msg);
    Logger::printLogLine(stdout, header.c_str(), msg, args);
    va_end(args);
}

void Logger::success(const char* msg, ...) const
{
    if (getCurrentLevel() > LogLevel_E::INFO)
        return;

    const std::string header(generateHeader(MessageType_E::SUCCESS).c_str());

    va_list args;
    va_start(args, msg);
    Logger::printLogLine(stdout, header.c_str(), msg, args);
    va_end(args);
}

void Logger::debug(const char* msg, ...) const
{
    if (getCurrentLevel() > LogLevel_E::DEBUG)
        return;

    const std::string header(generateHeader(MessageType_E::DEBUG).c_str());

    va_list args;
    va_start(args, msg);
    Logger::printLogLine(stdout, header.c_str(), msg, args);
    va_end(args);
}

void Logger::warn(const char* msg, ...) const
{
    if (getCurrentLevel() > LogLevel_E::WARN)
        return;

    const std::string header(generateHeader(MessageType_E::WARN).c_str());

    va_list args;
    va_start(args, msg);
    Logger::printLogLine(stderr, header.c_str(), msg, args);
    va_end(args);
}

void Logger::error(const char* msg, ...) const
{
    if (getCurrentLevel() > LogLevel_E::ERROR_)
        return;

    const std::string header(generateHeader(MessageType_E::ERROR_).c_str());

    va_list args;
    va_start(args, msg);
    Logger::printLogLine(stderr, header.c_str(), msg, args);
    va_end(args);
}

Logger::LogLevel_E Logger::getCurrentLevel() const
{
    if (m_optionalLevel.has_value())
        return m_optionalLevel.value();
    else
        return g_m_verbosityTable[static_cast<uint8_t>(
            g_m_verbosity.value_or(Logger::Verbosity_E::DEFAULT))][static_cast<uint8_t>(m_layer)];
}

// PRIVATES

void Logger::printLogLine(FILE* stream, const char* header, const char* msg, va_list args) const
{
    printLog(stream, header, msg, args);

    std::lock_guard<std::mutex> lock(g_m_printfLock);
    fprintf(g_m_streamOverride.value_or(stream), NEW_LINE);
}
void Logger::printLogLine(FILE* stream, const char* header, const char* msg) const
{
    printLog(stream, header, msg);

    std::lock_guard<std::mutex> lock(g_m_printfLock);
    fprintf(g_m_streamOverride.value_or(stream), NEW_LINE);
}

void Logger::printLog(FILE* stream, const char* header, const char* msg, va_list args) const
{
    std::lock_guard<std::mutex> lock(g_m_printfLock);

    fprintf(g_m_streamOverride.value_or(stream), header, this->m_tag.c_str());
    vfprintf(g_m_streamOverride.value_or(stream), msg, args);
    fflush(NULL);
}
void Logger::printLog(FILE* stream, const char* header, const char* msg) const
{
    std::lock_guard<std::mutex> lock(g_m_printfLock);

    fprintf(g_m_streamOverride.value_or(stream), header, this->m_tag.c_str());
    fprintf(g_m_streamOverride.value_or(stream), "%s", msg);
    fflush(NULL);
}

std::string Logger ::generateHeader(Logger::MessageType_E messageType) const noexcept
{
    std::string header;

    if (Logger::g_m_verbosity != Logger::Verbosity_E::DEFAULT &&
        Logger::g_m_verbosity != Logger::Verbosity_E::SILENT)
    {
        const uint32_t sec = std::chrono::duration_cast<std::chrono::seconds>(
                                 (preferredClock_t::now().time_since_epoch()))
                                 .count();
        const uint32_t nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                  (preferredClock_t::now().time_since_epoch()))
                                  .count() %
                              1'000'000'000;

        std::stringstream timestamp;
        timestamp << "[" << sec << "." << std::setw(9) << std::setfill('0') << nsec << "]";
        header.append(timestamp.str());
    }

    header.append("[" + m_tag + "]");

    const std::string orange   = Logger::printSpecials() ? ORANGE : "";
    const std::string green    = Logger::printSpecials() ? GREEN : "";
    const std::string yellow   = Logger::printSpecials() ? YELLOW : "";
    const std::string red      = Logger::printSpecials() ? RED : "";
    const std::string resetClr = Logger::printSpecials() ? RESETCLR : "";

    using MT_E = Logger::MessageType_E;
    switch (messageType)
    {
        case MT_E::INFO:
            header.append(" ");
            break;
        case MT_E::DEBUG:
            header.append("[" + orange + "DEBUG" + resetClr + "] ");
            break;
        case MT_E::SUCCESS:
            header.append("[" + green + "SUCCESS" + resetClr + "] ");
            break;
        case MT_E::WARN:
            header.append("[" + yellow + "WARN" + resetClr + "] ");
            break;
        case MT_E::ERROR_:
            header.append("[" + red + "ERROR" + resetClr + "] ");
            break;
        default:
            break;
    }

    return header;
}
