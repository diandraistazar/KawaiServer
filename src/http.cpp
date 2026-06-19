#include "../include/http.hpp"

int Http::create_http_header_str(struct Http::msg &http, std::string &dest) {
	if(http.mode == Http::HTTP_REQUEST) {
		struct Http::request_msg *request = (struct Http::request_msg*) &http;

		dest.append(request->method);
		dest.append(" ");
		dest.append(request->path);
		dest.append(" ");
		dest.append(request->version);
		dest += "\r\n";
	}
	else if(http.mode == Http::HTTP_RESPONSE) {
		struct Http::response_msg *response = (struct Http::response_msg*) &http;

		dest.append(response->version);
		dest.append(" ");
		dest.append(response->status_code);
		dest.append(" ");
		dest.append(response->status_desc);
		dest += "\r\n";
	}
	else
		return -1;

	for(const struct Http::Header &header : http.headers) {
		dest.append(header.key);
		dest.append(": ");
		dest.append(header.value);
		dest.append("\r\n");
	}

	return 0;
} 

int Http::parse_http_header(std::string &message, struct Http::msg &http) {
	std::string first, headers;

	size_t newline_pos = message.find("\n");
	first = message.substr(0, newline_pos);
	headers = message.substr(newline_pos + 1, message.size() - (newline_pos + 1));

    // If the header is Response HTTP/
	if(first.substr(0, 6) == "HTTP/") {
        struct Http::response_msg *response = (struct Http::response_msg*) &http;
        size_t pos_last = 0, pos_now = 0;	
		
		response->mode = Http::HTTP_RESPONSE;

		pos_now = first.find(" ");
		response->version = first.substr(pos_last, pos_now);
			
		pos_last = pos_now + 1;
		pos_now = first.rfind(" ");
		response->status_code = first.substr(pos_last, pos_now);

		pos_last = pos_now + 1;
		pos_now = first.rfind("\n");
		response->status_desc = first.substr(pos_last, pos_now);
    }
    // Else, the header is Request
	else if(first[0] != 0 && first[0] != ' ') {
        struct Http::request_msg *request = (struct Http::request_msg*) &http;
		size_t pos_last = 0, pos_now = 0;

		request->mode = Http::HTTP_REQUEST;
			
		pos_now = first.find(" ");
		request->method = first.substr(pos_last, pos_now - pos_last);
		
		// I need to add query handling
		// https:://google.com/index.html?key=value&key=value
		// ? sign is the beginning of the queries
		// key=value is the query in key-value pair
		// & is seperator between queries
		pos_last = pos_now + 1;
		pos_now = first.rfind(" ");
		request->path = first.substr(pos_last, pos_now - pos_last);

		pos_last = pos_now + 1;
		pos_now = first.rfind("\n");
		request->version = first.substr(pos_last, pos_now);
    }
    // The message is invalid
    else { 
        return -1;
	}
	
	struct Http::Header header;
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

// For handle both string key and value
void Http::push_token(struct Http::msg &http, char *key, char *value) {
	struct Http::Header header;
	if(key != nullptr)
		header.key = key;
	if(value != nullptr)
		header.value = value;

	http.headers.push_back(header);
}
// For handle string key and integer value
void Http::push_token(struct Http::msg &http, char *key, long long value) {
	struct Http::Header header;
	if(key != nullptr)
		header.key = key;
	header.value = std::to_string(value);

	http.headers.push_back(header);
}
