#include <logger.hpp>
#include <stdarg.h>

// void log_verbose(const char *msg, ...) {
//     if(verbose == 1) {
//         va_list args;
//         va_start(args, msg);
//         printf("[" GREY "#" RESETCLR "] ");
//         vfprintf(stderr, msg, args);
//         printf("\n");
//         va_end(args);
//     }
// }
void logger::info(const char* msg, ...)
{
	if (level > LogLevel_E::INFO)
		return;
	va_list args;
	va_start(args, msg);
	fprintf(stderr, "[%s][" BLUE "INFO" RESETCLR "] ", tag.c_str());
	vfprintf(stderr, msg, args);
	printf("\n");
	va_end(args);
}
void logger::success(const char* msg, ...)
{
	if (level > LogLevel_E::INFO)
		return;
	va_list args;
	va_start(args, msg);
	fprintf(stderr, "[%s][" GREEN " OK " RESETCLR "] ", tag.c_str());
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
	fprintf(stderr, "[%s][" ORANGE "DBG " RESETCLR "]", tag.c_str());
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
	fprintf(stderr, "[%s][" YELLOW "WARN" RESETCLR "] ", tag.c_str());
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
	fprintf(stderr, "[%s][" RED "ERR " RESETCLR "] ", tag.c_str());
	vfprintf(stderr, msg, args);
	fprintf(stderr, "\n");
	va_end(args);
}
