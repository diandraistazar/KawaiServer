#include <cerrno>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <filesystem>
#include <array>
#include <algorithm>
#include "../include/logging.hpp"
#include "../include/webserver.hpp"
#include "../include/utils.hpp"

/*
	Some extern variabels
*/

extern Logging logging;
extern Server server;

/*
	The below functions aren't part of Server,
	they are seperated class for client handling,
	such as storing the fd, strings of ip, port 
*/

Client::~Client() {
	Client::close();
}

void Client::close() {
	::close(this->client_fd); // Close in unistd.h header, not client's close method
}

/*
	The below functions are public functions,
	so that they can be accessible from user program 
*/

Server::~Server() {
	this->close();
}

void Server::close() {
	// Close in unistd.h header, not server's close method
	
	::close(this->server_fd);
	::close(this->php_fd);

	remove(LOCAL_PATH); // After using the filesystem socket, make sure to delete it for future use 
}

int Server::initialize() {
	struct sigaction sig_action = {0};
	std::array<int, 2> signals = { SIGINT, SIGTERM };

	sig_action.sa_handler = Server::terminate;

	for(const int &signal : signals)
		if(sigaction(signal, &sig_action, nullptr) < 0)
			return -1;

	return 0;
}

void Server::run() {
	logging.info("server opened HTTP service");
	logging.info("root-path is %s", this->root_path);

	while(this->is_continue) {
		struct Client client;

		if(this->accept_client(client) < 0) {
			logging.error("failed to accept a client");
			continue;
		}
		
		this->handling_client(client);
	}
}

int Server::create_sockets(char *host, char *service) {
	// Setup the server itself
	struct addrinfo *addr, hints = {
		.ai_flags = 0,
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
		.ai_protocol = 0,
	};

	int return_value = getaddrinfo(host, service, &hints, &addr);
	if(addr == nullptr) {
		logging.error("getaddrinfo(): %s", gai_strerror(return_value));
		return -1;
	}

	this->server_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if(this->server_fd < 0)
		return -1;
		
	struct sockaddr_in *server_sock = (struct sockaddr_in*) addr->ai_addr;
	Utils::converto(Utils::IP4_ADDR_STR, server_sock->sin_addr.s_addr, this->ip_addr);
	Utils::converto(Utils::PORT_STR, server_sock->sin_port, this->port);
	
	std::memcpy(&this->server_sock, server_sock, sizeof(this->server_sock));
	freeaddrinfo(addr);	

	// Setup for IPC with PHP
	this->php_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if(this->php_fd < 0)
		return -1;

	this->php_sock.sun_family = AF_LOCAL;
	std::strncpy(this->php_sock.sun_path, LOCAL_PATH, LOCAL_PATH_LEN);
	return 0;
}

int Server::set_socket_options() {
	int yes = 1;

	if(setsockopt(this->server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
		return -1;

	return 0;
}

int Server::bind_sockets() {
	// Server socket
	if(bind(this->server_fd, (struct sockaddr*) &this->server_sock, sizeof(this->server_sock)) < 0)
		return -1;

	// PHP socket
	remove(LOCAL_PATH); // If the filesystem socket is exists, delete it. The server can't communicate to php if it's exists.
	if(bind(this->php_fd, (struct sockaddr*) &this->php_sock, sizeof(this->php_sock)) < 0)
		return -1;

	return 0;
}

int Server::listen_sockets() {
	if(listen(this->server_fd, 1) < 0)
		return -1;

	if(listen(this->php_fd, 1) < 0)
		return -1;

	return 0;
}

/*
	The below functions are private functions of Server,
	intended for server internal operations only
*/
void Server::terminate(int signal) {
	logging.info("server received a terminate signal (either SIGTERM or SIGINT). terminating");

	server.is_continue = false;
}

size_t Server::recv_data(int fd, std::string &dest) {
	ssize_t bytes = 0, recv_bytes = 0;
	char buffer[512]; // Hold 512 bytes
	
	while(true) {
		bytes = recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT);

		if(bytes > 0) {
			dest.append(buffer, 0, bytes);
			recv_bytes += bytes;
		}
		else
			break;
	}

	if(bytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
		return -1;

	return recv_bytes;
}

size_t Server::send_data(int fd, std::string &buffer) {
	size_t bytes = 0, send_bytes = 0, buffer_size = 0;
	char *start_data = nullptr, *end_data = nullptr;

	buffer_size = buffer.size();
	start_data = buffer.data();
	end_data = start_data + buffer_size;
	
	while(send_bytes < buffer_size) {
		start_data += send_bytes;

		bytes = send(fd, start_data, std::min((ssize_t)128, end_data - start_data), 0);

		if(bytes > 0)
			send_bytes += bytes;
		else
			break;
	}

	if(bytes == -1)
		return -1;

	return send_bytes;
}

size_t Server::send_file(int fd, std::string &filepath) {
	size_t bytes = 0, send_bytes = 0, file_size = 0;
	int file_fd = 0;

	file_fd = open(filepath.data(), O_RDONLY);
	if(file_fd < 0)
		return -1;

	file_size = Utils::get_filesize(filepath.data());
	if(file_size < 0)
		return -1;

	while(send_bytes < file_size) {
		bytes = sendfile(fd, file_fd, 0, file_size);
		
		if(bytes > 0)
			send_bytes += bytes;
		else
			break;
	}
	
	if(bytes == -1)
		return -1;

	::close(file_fd);
	return send_bytes;
}

void Server::handling_client(struct Client &client) {
	struct Http::request_msg _request;
	std::string request;
	ssize_t received_bytes = 0;

	received_bytes = this->recv_data(client.client_fd, request); 
	if(received_bytes <= 0){
		logging.error("recv_data() returns non-zero or zero");
		return;
	}

	logging.info("server received %d bytes request from %s:%s", received_bytes, client.ip_addr, client.port);

	if(Http::parse_http_header(request, (struct Http::msg&) _request) < 0) {
		logging.error("Http::parse_http_header returns non-zero");
		return;
	}
	
	logging.info("the request is parsed and stored in request structure");

	logging.info(
		"%s:%s with %s, requests %s%s resource",
		client.ip_addr, client.port, _request.method.data(), this->root_path, _request.path.data());

	if(_request.method == "GET")
		this->handle_get(_request, client);
}

int Server::accept_client(class Client &client) {
	socklen_t size = sizeof(client.client_sock);

	client.client_fd = accept(this->server_fd, (struct sockaddr*) &client.client_sock, &size);
	if(client.client_fd < 0)
		return -1;

	getnameinfo((struct sockaddr*) &client.client_sock, sizeof(client.client_sock),
				client.hostname, sizeof(client.hostname),
				client.service, sizeof(client.service), 0);

	Utils::converto(Utils::IP4_ADDR_STR, client.client_sock.sin_addr.s_addr, client.ip_addr);
	Utils::converto(Utils::PORT_STR, client.client_sock.sin_port, client.port);
	return 0;
}

/*
	Those below functions used to handle the client request,
	e.g GET, PUT, and so on. Currently only GET
*/

#define PUSH(a, b) Http::push_token((struct Http::msg&) _response, (char *) a, b);

void Server::handle_get(struct Http::request_msg &_request, struct Client &client) {
	struct Http::response_msg _response;
	std::string request_path, response, body;
	bool sendfile = false;
	// true -> Used when sending a file. It's directly sending the file to the client instead of loading into memory. It helps reduce time consuming and improve server latency and memory.
	// false -> Used when sending non-file (e.g hardcoded data). The data stored in memory, and need to send the client.
	
	_response.mode = Http::HTTP_RESPONSE;
	_response.version = "HTTP/1.0";
	
	request_path = this->root_path;
	request_path += _request.path; // '.' + '/home/diandra...'
		
	if(std::filesystem::is_directory(request_path)) {
		if(std::filesystem::exists(request_path + "/index.html"))
			request_path += "/index.html";
		else if(std::filesystem::exists(request_path + "/index.php"))
			request_path += "/index.php";
	}

	if(std::filesystem::is_regular_file(request_path)) { // This block will check two conditions, is exists and is regular file 
		char *mimetype;

		mimetype = (char *) Utils::get_mimetype(request_path.data());
		if(mimetype == nullptr)
			mimetype = (char *) Utils::mime_types[Utils::PLAIN];
			
		sendfile = true;

		_response.status_code = "200";
		_response.status_desc = "Found";
		
		PUSH("Content-Type", mimetype);
		PUSH("Content-Length", Utils::get_filesize(request_path.data()));
		PUSH("Content-Location", request_path.data() + std::strlen(this->root_path));
	}
	else {
		body = "<h1>we couldn't found the requested resource</h1>";

		_response.status_code = "404";
		_response.status_desc = "Not Found";

		PUSH("Content-Type", (char *) "text/html");
		PUSH("Content-Length", body.size());

		sendfile = false;
	}
	
	Http::create_http_header_str((struct Http::msg&) _response, response); // Append the HTTP header
	response.append("\r\n"); // Then empty line to seperate HTTP header and Body 

	if(this->send_data(client.client_fd, response) < 0) {
		logging.error("failure to send %s:%s request", client.ip_addr, client.port);
		return;
	}
	
	if(sendfile) {
		if(this->send_file(client.client_fd, request_path) < 0)
			logging.error("failure to send %s:%s request", client.ip_addr, client.port);
		else
			logging.info("success to send %s:%s request", client.ip_addr, client.port);
	}
	else {
		if(this->send_data(client.client_fd, body) < 0)
			logging.error("failure to send %s:%s request", client.ip_addr, client.port);
		else
			logging.info("success to send %s:%s request", client.ip_addr, client.port);
	}
}
