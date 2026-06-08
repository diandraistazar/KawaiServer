#pragma once

#include <string>
#include <cstring>
#include <vector>

struct http {
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

	static std::string create_http_header_str(struct http::msg &http) {
		std::string msg_temp;

		if(http.mode == http::HTTP_REQUEST) {
			struct http::request_msg *http_req = (struct http::request_msg*) &http;

			msg_temp.append(http_req->method);
			msg_temp.append(" ");
			msg_temp.append(http_req->path);
			msg_temp.append(" ");
			msg_temp.append(http_req->version);
			msg_temp += "\r\n";
		}
		else if(http.mode == http::HTTP_RESPONSE) {
			struct http::response_msg *http_res = (struct http::response_msg*) &http;

			msg_temp.append(http_res->version);
			msg_temp.append(" ");
			msg_temp.append(http_res->status_code);
			msg_temp.append(" ");
			msg_temp.append(http_res->status_desc);
			msg_temp += "\r\n";
		}

		for(int index = 0; index < http.headers.size(); index++) {
			msg_temp.append(http.headers[index].key);
			msg_temp.append(": ");
			msg_temp.append(http.headers[index].value);
			msg_temp.append("\r\n");
		}

		return msg_temp;
	} 

	static int parse_http_header(std::string message, struct http::msg &http) {
		std::string first, headers;

		size_t newline_pos = message.find("\n");
		first = message.substr(0, newline_pos);
		headers = message.substr(newline_pos + 1, message.size() - (newline_pos + 1));

    	// If the header is Response HTTP/
		if(first.substr(0, 6) == "HTTP/") {
        	struct http::response_msg *http_res = (struct http::response_msg*) &http;
        	size_t pos_last = 0, pos_now = 0;	
		
			http_res->mode = http::HTTP_RESPONSE;

			pos_now = first.find(" ");
			http_res->version = first.substr(pos_last, pos_now);
			
			pos_last = pos_now + 1;
			pos_now = first.rfind(" ");
			http_res->status_code = first.substr(pos_last, pos_now);

			pos_last = pos_now + 1;
			pos_now = first.rfind("\n");
			http_res->status_desc = first.substr(pos_last, pos_now);
    	}
    	// Else, the header is Request
		else if(first[0] != 0 && first[0] != ' ') {
        	struct http::request_msg *http_req = (struct http::request_msg*) &http;
			size_t pos_last = 0, pos_now = 0;

			http_req->mode = http::HTTP_REQUEST;
			
			pos_now = first.find(" ");
			http_req->method = first.substr(pos_last, pos_now - pos_last);
			
			pos_last = pos_now + 1;
			pos_now = first.rfind(" ");
			http_req->path = first.substr(pos_last, pos_now - pos_last);

			pos_last = pos_now + 1;
			pos_now = first.rfind("\n");
			http_req->version = first.substr(pos_last, pos_now);
    	}
    	// The message is invalid
    	else { 
        	return -1;
		}
	
		struct http::Header header;
		size_t seperator = 0, last_string = 0, first_string = 0;
		while(true) {
			first_string = last_string;
			seperator = headers.find_first_of(": ", first_string + 1);
			last_string = headers.find_first_of("\n", seperator + 2); // ': ' start after the seperator

			if(seperator == -1 || last_string == -1)
				break;

			header.key = headers.substr(first_string + 1, seperator);
			header.value = headers.substr(seperator + 2, last_string);
			http.headers.push_back(header);
		}

    	return 0;
	};
	
	static void push_token(struct http::msg &http, char *key, char *value) {
		struct http::Header header;
		if(key != nullptr)
			header.key = key;
		if(value != nullptr)
			header.value = value;

		http.headers.push_back(header);
	}

};
