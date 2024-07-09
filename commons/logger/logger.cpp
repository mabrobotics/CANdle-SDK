#include <logger.hpp>
#include <stdarg.h>

void logger::info(const char* msg, ...)
{
    if (level > LogLevel_E::INFO)
        return;
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "[" BLUE "INFO" RESETCLR "][%s]", this->tag.c_str());
    vfprintf(stderr, msg, args);
    printf("\n");
    va_end(args);
}

void logger::info_raw(const char* msg, ...)
{
    if (level > LogLevel_E::INFO)
        return;
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
}

void logger::success(const char* msg, ...)
{
    if (level > LogLevel_E::INFO)
        return;
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "[" GREEN " OK " RESETCLR "][%s]", this->tag.c_str());
    vfprintf(stderr, msg, args);
    printf("\n");
    va_end(args);
}

void logger::debug(const char* msg, ...)
{
    if (level > LogLevel_E::DEBUG)
        return;
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "[" ORANGE "DBG " RESETCLR "][%s]", this->tag.c_str());
    vfprintf(stderr, msg, args);
    printf("\n");
    va_end(args);
}

void logger::warn(const char* msg, ...)
{
    if (level > LogLevel_E::WARN)
        return;
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "[" YELLOW "WARN" RESETCLR "][%s]", this->tag.c_str());
    vfprintf(stderr, msg, args);
    printf("\n");
    va_end(args);
}

void logger::error(const char* msg, ...)
{
    if (level > LogLevel_E::ERROR)
        return;
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "[" RED "ERR " RESETCLR "][%s]", this->tag.c_str());
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    va_end(args);
}
