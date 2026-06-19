#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <fstream>
#include <sstream>

#include "../include/utils.hpp"

/*
	The below functions used to as helper functions
*/

#define NSEC_PER_SECONDS 1000000000

double Utils::get_time() {
	static double offset = 0;
	struct timespec tm;

	if(offset == 0) {
		clock_gettime(CLOCK_REALTIME, &tm);
		offset = tm.tv_sec * NSEC_PER_SECONDS + tm.tv_nsec;
	}

	clock_gettime(CLOCK_REALTIME, &tm);
	return (double)(tm.tv_sec * NSEC_PER_SECONDS + tm.tv_nsec - offset) / (double)NSEC_PER_SECONDS;
}

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

ssize_t Utils::read_file(std::string &filepath, std::string &dest) {
	std::ifstream file(filepath);
	std::stringstream temp;

	temp << file.rdbuf();
	dest = temp.str();

	return temp.str().size();
}

ssize_t Utils::get_filesize(std::string &filepath) {
	struct stat buffer;

	if(stat(filepath.data(), &buffer) < 0)
		return -1;

	return buffer.st_size;
}

std::string Utils::get_filext(std::string &filepath) {
	ssize_t pos;

	pos = filepath.rfind(".");
	if(pos == std::string::npos)
		return "";

	return filepath.substr(pos + 1, filepath.size() - (pos + 1));
}

std::string Utils::get_mimetype(std::string &filepath) {
	std::string file_ext = Utils::get_filext(filepath);
	
	// Default the filepath doesn't have file extension 
	if(file_ext.empty())
		return Utils::mime_types["txt"];
	
	auto search = Utils::mime_types.find(file_ext);
	if(search != Utils::mime_types.end())
		return search->second;

	return "";
}
