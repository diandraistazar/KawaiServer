#pragma once

#include <string>

namespace Utils {
	char *get_mimetype(char *filepath);
	size_t read_file(char *filepath, std::string &dest);
}
