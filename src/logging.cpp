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
	
	if(level < this->level)
		return;

	if(level <= LOG_WARN)
		stream = stdout;
	else
		stream = stderr;
	
	va_start(list, format);
	
	seconds = time(nullptr);
	tm_time = localtime(&seconds);
	time_size = strftime(time_buffer, sizeof(time_buffer), "%b %d %H:%M:%S", tm_time);
	
	std::fprintf(stream, "%s %s%s:%d [%s] \x1b[0m",
		time_buffer, this->use_color ? this->level_colors[level] : "",
		filename, line, this->level_strings[level]);
	std::vfprintf(stream, format, list);

	va_end(list);
	std::fwrite("\n", 1, 1, stream);
}
