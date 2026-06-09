#pragma once

#include <string>
#include <vector>

namespace Http {
	enum { HTTP_REQUEST, HTTP_RESPONSE };

	struct Header {
		std::string key, value;
	};

	struct request_msg {
		int mode = -1; // Must be HTTP_REQUEST
		std::string method, version, path;
		std::vector<struct Header> headers;
	};

	struct response_msg {
		int mode = -1; // Must be HTTP_RESPONSE
		std::string version, status_code, status_desc;
		std::vector<struct Header> headers;
	};

	struct msg {
    	int mode = -1; // either HTTP_REQUEST or HTTP_RESPONSE
    	char data[sizeof(std::string) * 3] = {0};
		std::vector<struct Header> headers;
	};

	int create_http_header_str(struct Http::msg &http, std::string &dest);
	int parse_http_header(std::string &message, struct Http::msg &http);
	void push_token(struct Http::msg &http, char *key, char *value);
	void push_token(struct Http::msg &http, char *key, long long value);
};
