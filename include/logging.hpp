#pragma once

#include <array>

// __FILE__ points to the current file where the __FILE__ define does, in this case logging.hpp, but if you called it over main.cpp it will be 'main.cpp'
// __LINE__ points to where line you called the __LINE__ define. 
// __VA_ARGS__ used with '...' means, every arguments will be grouped as __VA_ARGS__

#define trace(...) log(Logging::LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define debug(...) log(Logging::LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define info(...) log(Logging::LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define warn(...) log(Logging::LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define error(...) log(Logging::LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define fatal(...) log(Logging::LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

struct Logging {
	enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };
	
	int level = LOG_TRACE, use_color = true; // Level of logging and should use color (either true or false)

	// For string level
	std::array<const char*, 6> level_strings = {
  		"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
	};
	
	// For level coloring
	std::array<const char*, 6> level_colors = {
  		"\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
	};
	
	void log(int level, const char *filename, int line, const char *format, ...);
};
