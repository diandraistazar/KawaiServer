#pragma once

#include <string>
#include <unordered_map>

namespace Utils {
	enum Converto { IP4_ADDR_STR, PORT_STR };
	
	static std::unordered_map<std::string, std::string> mime_types = {
		{ "txt", "text/plain" },
		{ "html", "text/html" },
		{ "htm", "text/html" },
		{ "css", "text/css" },
		{ "js", "text/javascript" },
		{ "php", "text/php" },
		{ "jpeg", "image/jpeg" },
		{ "jpg", "image/jpeg" },
		{ "mp3", "audio/mpeg" },
		{ "mp4", "video/mp4" },
	};

	double get_time();
	int converto(int to, int data, char *dest);
	ssize_t read_file(std::string &filepath, std::string &dest);
	ssize_t get_filesize(std::string &filepath);
	std::string get_filext(std::string &filepath);
	std::string get_mimetype(std::string &filepath);
}
