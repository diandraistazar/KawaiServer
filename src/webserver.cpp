#include <unistd.h>
#include <cstring>
#include <filesystem>
#include "../include/logging.hpp"
#include "../include/webserver.hpp"
#include "../include/utils.hpp"

#define PUSH(a, b) Http::push_token((struct Http::msg&) response, (char *) a, b);

extern Logging logging;

// Client
Client::~Client() {
	Client::close();
}

void Client::close() {
	::close(this->fd); // Close in unistd.h header, not client's close method
}

// Webserver
Server::~Server() {
	this->close();
}

void Server::close() {
	::close(this->fd); // Close in unistd.h header, not server's close method
}

int Server::create_sock(char *host, char *service, int flags, int family, int socktype, int protocol) {
	struct addrinfo *addr, hints = {
		.ai_flags = flags,
		.ai_family = family,
		.ai_socktype = socktype,
		.ai_protocol = protocol,
	};

	int return_value = getaddrinfo(host, service, &hints, &addr);
	if(addr == nullptr)
		return -1;

	this->fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if(this->fd == -1)
		return -1;
		
	struct sockaddr_in *sock_in = (struct sockaddr_in*) addr->ai_addr;
	Utils::converto(Utils::IP4_ADDR_STR, sock_in->sin_addr.s_addr, this->ip_addr);
	Utils::converto(Utils::PORT_STR, sock_in->sin_port, this->port);
	
	std::memcpy(&this->addr, addr, sizeof(this->addr));
	freeaddrinfo(addr);	
	return 0;
}

int Server::set_sockopt(int opt, int values, void *data, size_t data_size) {
	if(setsockopt(this->fd, opt, values, data, data_size) == -1)
		return -1;

	return 0;
}

int Server::bind_sock() {
	struct sockaddr_in *sock_in = (struct sockaddr_in*) this->addr.ai_addr;

	if(bind(this->fd, (struct sockaddr*) sock_in, sizeof(*sock_in)) == -1)
		return -1;

	return 0;
}

int Server::listen_sock(int backlog) {
	if(listen(this->fd, backlog) == -1)
		return -1;

	return 0;
}

void Server::start() {
	while(true) {
		struct Client client;

		if(this->accept_client(client) == -1) {
			logging.error("Failed to accept a client");
			continue;
		}
		
		this->handling_client(client);
	}
}

// Private Server methods
size_t Server::receive_data(struct Client &client, std::string &dest) {
	char buffer[1024] = {0};
	size_t bytes = 0;

	bytes = recv(client.fd, buffer, sizeof(buffer), 0);
	dest = buffer;

	return bytes;
}

size_t Server::send_data(struct Client &client, std::string &source) {
	int bytes = 1, size = source.size();
	char *data = source.data();

	while(bytes > 0 && bytes < size) {
		bytes = send(client.fd, data, size, 0);
	}

	return bytes;
}

void Server::handling_client(struct Client &client) {
	std::string request_data, complete_response;
	struct Http::request_msg request;

	if(this->receive_data(client, request_data) == -1) {
		logging.error("receive_data() returns -1");
		return;
	}
	
	logging.info("success to receive '%d' bytes request from '%s'", request_data.size(), client.ip_addr);

	if(Http::parse_http_header(request_data, (struct Http::msg&) request) == -1) {
		logging.error("Http::parse_http_header returns -1");
		return;
	}
	
	logging.info("success to parse the request and stored to request structure");

	logging.info(
		"'%s' (%s) with '%s' method, requests '%s' resource.",
		client.ip_addr, client.hostname, request.method.data(), request.path.data());

	if(request.method == "GET")
		this->handle_get(complete_response, request);

	if(this->send_data(client, complete_response) == -1) {
		logging.error("send_data() returns -1");
		return;
	}

	logging.info("success to send '%s' resource to '%s' (%s)", request.path.data(), client.ip_addr, client.hostname);
}

int Server::accept_client(class Client &client) {
	socklen_t size = sizeof(client.in_sock);

	client.fd = accept(this->fd, (struct sockaddr*) &client.in_sock, &size);
	if(client.fd == -1)
		return -1;

	getnameinfo((struct sockaddr*) &client.in_sock, sizeof(client.in_sock),
				client.hostname, sizeof(client.hostname),
				client.service, sizeof(client.service), 0);

	Utils::converto(Utils::IP4_ADDR_STR, client.in_sock.sin_addr.s_addr, client.ip_addr);
	Utils::converto(Utils::PORT_STR, client.in_sock.sin_port, client.port);
	return 0;
}

void Server::handle_get(std::string &dest, struct Http::request_msg &request) {
	struct Http::response_msg response;
	std::string body;

	response.mode = Http::HTTP_RESPONSE;
	response.version = "HTTP/1.0";
	
	if(!std::filesystem::exists(request.path.data())) { 
		logging.info("'%s' resource isn't exists. Instead, send 'not-found' resource", request.path.data());
		request.path = "Not-Found";

		body = "<h1><b>Not Found - 404</b></h1>";
		
		response.status_code = "404";
		response.status_desc = "Not Found";
		
		PUSH("Content-Type", "text/html");
		PUSH("Content-Length", body.size());
		goto combine;
	}

	logging.info("'%s' resource is exists. Processing", request.path.data());

	if(std::filesystem::exists(request.path)) {
		char *mime_type = Utils::get_mimetype((char *) request.path.data());
	
		response.status_code = "200";
		response.status_desc = "OK";

		if(Utils::read_file(request.path.data(), body) < 1) {
			body = "File length is '0'!";
			goto combine;
		}

		PUSH("Content-Type", mime_type);
		PUSH("Content-Length", body.size());
	}
	else if(std::filesystem::is_directory(request.path)) {
		response.status_code = "403";
		response.status_desc = "You accessed a directory, what?";

		body = "<h1></b>Umm, My server can't serve a directory, only files :3, but I will add it later..</b></h1>";
	}

combine:

	Http::create_http_header_str((struct Http::msg&) response, dest); // Append the HTTP header
	dest.append("\r\n"); // Then empty line to seperate HTTP header and Body 
	dest.append(body); // Finally, append the body
}
