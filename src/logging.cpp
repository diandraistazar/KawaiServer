#include <time.h>
#include <stdarg.h>
#include <iostream>
#include "../include/logging.hpp"

void Logging::log(int level, const char *filename, int line, const char *format, ...) {
	struct tm *tm_time = nullptr;
	FILE *stream = nullptr;
	char time_buffer[32];
	size_t time_size = 0;
	time_t seconds = 0;
	va_list list;
	
	if(level <= LOG_WARN)
		stream = stdout;
	else
		stream = stderr;
	
	va_start(list, format);
	
	seconds = time(nullptr);
	tm_time = localtime(&seconds);
	time_size = strftime(time_buffer, sizeof(time_buffer), "%m-%d-%y %I:%M:%S %p", tm_time);

	std::fwrite(time_buffer, 1, time_size, stream); // Print the timestamp first
	std::fprintf(stream, " [%s] ", level_strings[level]); // Then, print the level label, e.g TRACE, ERROR
	std::vfprintf(stream, format, list);
	
	va_end(list);
	std::fwrite("\n", 1, 1, stream);
}
