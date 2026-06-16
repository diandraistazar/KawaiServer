#pragma once

#include <string>
#include <vector>
#include <array>

namespace Utils {
	enum Converto { IP4_ADDR_STR, PORT_STR };
	
	enum { PLAIN, HTML, CSS, JAVASCRIPT, PHP, MIME_TYPE_COUNT };
	static std::array<const char*, MIME_TYPE_COUNT> mime_types = {
		"text/plain", "text/html", "text/css", "text/javascript", "text/php"
	};
	static std::array<const char*, MIME_TYPE_COUNT> ext_types = {
		"txt", "html", "css", "js", "php"
	};

	double get_time();
	std::string lowercase(char *source);
	int converto(int to, int data, char *dest);
	size_t read_file(char *filepath, std::string &dest);
	size_t list_dir(char *fullpath, std::vector<char*> &dest);
	size_t get_filesize(char *filepath);
	char *get_filext(char *filepath);
	const char *get_mimetype(char *filepath);
}
