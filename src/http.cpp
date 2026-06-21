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

	for(auto &pair : http.headers) {
		dest.append(pair.first);
		dest.append(": ");
		dest.append(pair.second);
		dest.append("\r\n");
	}

	return 0;
} 

int Http::parse_http_header(std::string &message, struct Http::msg &http) {
	std::string first, headers;

	size_t newline_pos = message.find("\n");
	first = message.substr(0, newline_pos);
	headers = message.substr(newline_pos + 1, message.size() - newline_pos + 1);

    // If the header is Response HTTP/
	if(first.substr(0, 6) == "HTTP/") {
        struct Http::response_msg *response = (struct Http::response_msg*) &http;
        size_t pos_last = 0, pos_now = 0;	
		
		response->mode = Http::HTTP_RESPONSE;

		if((pos_now = first.find(" ")) == std::string::npos)
			return -1;

		response->version = first.substr(pos_last, pos_now - pos_last);
			
		pos_last = pos_now + 1;
		if((pos_now = first.rfind(" ")) == std::string::npos)
			return -1;

		response->status_code = first.substr(pos_last, pos_now - pos_last);

		pos_last = pos_now + 1;
		response->status_desc = first.substr(pos_last, first.size() - pos_last);
    }
	else if(first[0] != 0 && first[0] != ' ') { // Else, the header is Request
        struct Http::request_msg *request = (struct Http::request_msg*) &http;
		size_t pos_last = 0, pos_now = 0;

		request->mode = Http::HTTP_REQUEST;
			
		if((pos_now = first.find(" ")) == std::string::npos)
			return -1;

		request->method = first.substr(pos_last, pos_now - pos_last);
		
		// I need to add query handling
		// https:://google.com/index.html?key=value&key=value
		// ? sign is the beginning of the queries
		// key=value is the query in key-value pair
		// & is seperator between queries
		pos_last = pos_now + 1;
		if((pos_now = first.rfind(" ")) == std::string::npos)
			return -1;

		request->path = first.substr(pos_last, pos_now - pos_last);
		
		pos_last = pos_now + 1;
		request->version = first.substr(pos_last, first.size() - pos_last);
		
		// Query handling
		pos_last = request->path.find("?");
		if(pos_last != std::string::npos) {
			std::string queries, query, key, value;
			size_t seperator;
			bool looping = true;

			queries = request->path.substr(pos_last + 1, request->path.size() - (pos_last + 1));
			request->path = request->path.substr(0, pos_last);
			
			pos_last = -1;
			while(looping) {
				pos_now = pos_last + 1;
				pos_last = queries.find_first_of("&", pos_now);
	
				if(pos_last == std::string::npos)
					looping = false;

				query = queries.substr(pos_now,
					(pos_last == std::string::npos ? queries.size() : pos_last)
					- pos_now
				);

				if((seperator = query.find("=")) == std::string::npos)
					continue;
				
				key = query.substr(0, seperator);
				value = query.substr(seperator + 1, query.size() - (seperator + 1));
				request->queries[key] = value;
			}
		}
    } else
		return -1;
	
	std::string key, value, line;
	size_t first_string = 0, last_string = -1, seperator = 0;
	while(true) {
		first_string = last_string + 1;
		last_string = headers.find_first_of("\n", first_string); // start after \n

		if(last_string == std::string::npos)
			break;
		
		line = headers.substr(first_string, last_string - first_string); // get the current processed header line
		
		if((seperator = line.find_first_of(": ", 0)) == std::string::npos)
			break;

		key = line.substr(0, seperator);
		value = line.substr(seperator + 2, last_string - (seperator + 2));
		http.headers[key] = value;
	}

    return 0;
};

// For handle both string key and value
void Http::push_token(struct Http::msg &http, char *key, char *value) {
	if(key != nullptr)
		http.headers[key] = value;
}
// For handle string key and integer value
void Http::push_token(struct Http::msg &http, char *key, long long value) {
	if(key != nullptr)
		http.headers[key] = std::to_string(value);
}
