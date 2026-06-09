#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/socket.h>
#include <magic.h>

#include "../include/utils.hpp"

char *Utils::get_mimetype(char *filepath) {
	magic_t magic = magic_open(MAGIC_MIME_TYPE);
	magic_load(magic, nullptr);

	return (char *) magic_file(magic, filepath);
}

size_t Utils::read_file(char *filepath, std::string &dest) {
	std::ifstream file(filepath);
	std::stringstream temp;

	temp << file.rdbuf();
	dest = temp.str();

	return temp.str().size();
}
