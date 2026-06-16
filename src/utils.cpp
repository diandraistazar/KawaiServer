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

std::string Utils::lowercase(char *source) {
	std::string buffer = "";

	for(char *ptr = source; *ptr != 0; ptr++) {
		buffer.push_back(std::tolower(*ptr));
	}

	return buffer;
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

size_t Utils::read_file(char *filepath, std::string &dest) {
	std::ifstream file(filepath);
	std::stringstream temp;

	temp << file.rdbuf();
	dest = temp.str();

	return temp.str().size();
}

size_t Utils::list_dir(char *fullpath, std::vector<char*> &dest) {
	DIR *dir = opendir(fullpath);
	if(dir == nullptr)
		return -1;
	
	struct dirent *dirp = nullptr;
	size_t count = 0;
	while((dirp = readdir(dir)) != nullptr) {
		dest.push_back(dirp->d_name);
		count++;
	}

	closedir(dir);
	return count;
}

size_t Utils::get_filesize(char *filepath) {
	struct stat buffer;

	if(stat(filepath, &buffer) < 0)
		return -1;

	return buffer.st_size;
}

char *Utils::get_filext(char *filepath) {
	char *ptr = filepath;

	for(int i = std::strlen(ptr) - 1; i; i--) {
		if(ptr[i] == '.')
			return ptr + i + 1;
	}

	return nullptr;
}

const char *Utils::get_mimetype(char *filepath) {
	std::string file_ext = Utils::get_filext(filepath);
	if(file_ext.empty())
		return mime_types[Utils::PLAIN];

	for(int i = 0; i < Utils::MIME_TYPE_COUNT; i++)
		if(file_ext == ext_types[i])
			return mime_types[i];

	return nullptr;
}


