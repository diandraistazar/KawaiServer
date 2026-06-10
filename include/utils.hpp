#pragma once

#include <string>

namespace Utils {
	enum Converto { IP4_ADDR_STR, PORT_STR };
	
	int converto(int to, int data, char *dest);
	size_t read_file(char *filepath, std::string &dest);
	char *get_mimetype(char *filepath);
}
