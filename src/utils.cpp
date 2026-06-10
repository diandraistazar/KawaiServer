#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <magic.h>
#include <fstream>
#include <sstream>

#include "../include/utils.hpp"

int Utils::converto(int to, int data, char *dest) {
	if(to == Utils::IP4_ADDR_STR) {
		struct in_addr a = { (in_addr_t) data };

		if(inet_ntop(AF_INET, &a, dest, (socklen_t) INET_ADDRSTRLEN) != dest)
			return -1;
	}
	else if(to == Utils::PORT_STR) {
		std::sprintf(dest, "%d", ntohs(data));
	}
	else
		return -1;

	return 0;
}

size_t Utils::read_file(char *filepath, std::string &dest) {
	std::ifstream file(filepath);
	std::stringstream temp;

	temp << file.rdbuf();
	dest = temp.str();

	return temp.str().size();
}

char *Utils::get_mimetype(char *filepath) {
	magic_t magic = magic_open(MAGIC_MIME_TYPE);
	magic_load(magic, nullptr);

	return (char *) magic_file(magic, filepath);
}


