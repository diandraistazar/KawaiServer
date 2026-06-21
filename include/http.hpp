#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace Http {
	enum { HTTP_REQUEST, HTTP_RESPONSE };

	static std::unordered_map<int, std::string> status_messages = {
		{ 200, "OK" },
		{ 404, "Not Found" }
	};

	struct request_msg {
		int mode; // Must be HTTP_REQUEST
		std::string method, version, path;
		std::unordered_map<std::string, std::string> queries, headers;
	};

	struct response_msg {
		int mode; // Must be HTTP_RESPONSE
		std::string version, status_code, status_desc;
		char padding[sizeof(std::unordered_map<std::string, std::string>)];
		std::unordered_map<std::string, std::string> headers;
	};

	struct msg {
    	int mode; // either HTTP_REQUEST or HTTP_RESPONSE
    	char padding[sizeof(std::string) * 3 + sizeof(std::unordered_map<std::string, std::string>)];
		std::unordered_map<std::string, std::string> headers;
	};

	int create_http_header_str(struct Http::msg &http, std::string &dest);
	int parse_http_header(std::string &message, struct Http::msg &http);
	void push_token(struct Http::msg &http, char *key, char *value);
	void push_token(struct Http::msg &http, char *key, long long value);
};
