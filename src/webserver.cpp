#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <algorithm>
#include <array>
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
	such as storing fd, strings of ip, port 
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
			logging.error("server can't accept client");
			continue;
		}
		
		this->handling_client(client);
	}
}

int Server::create_sockets(char *host, char *service) {
	// Setup server itself
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

	return 0;
}

int Server::listen_sockets() {
	if(listen(this->server_fd, 1) < 0)
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

ssize_t Server::read_data(int fd, std::string &dest) {
	ssize_t bytes = 0, recv_bytes = 0;
	char buffer[512]; // Hold 512 bytes
	
	while(true) {
		bytes = read(fd, buffer, sizeof(buffer));

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

ssize_t Server::recv_data(int fd, std::string &dest) {
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


ssize_t Server::send_data(int fd, std::string &buffer) {

#define MIN(a, b) std::min((size_t)(a), (size_t)(b))

	ssize_t bytes = 0, send_bytes = 0, buffer_size = 0;
	char *start_data = nullptr, *end_data = nullptr;

	buffer_size = buffer.size();
	start_data = buffer.data();
	end_data = start_data + buffer_size;

	while(send_bytes < buffer_size) {
		bytes = send(fd, start_data + send_bytes,
			MIN(64, end_data - (start_data + send_bytes)), 0);
		
		if(bytes > 0)
			send_bytes += bytes;
		else
			break;
	}

	if(bytes == -1)
		return -1;

	return send_bytes;
}

ssize_t Server::send_file(int fd, std::string &filepath) {
	ssize_t bytes = 0, send_bytes = 0, file_size = 0;
	int file_fd = 0;

	file_fd = open(filepath.data(), O_RDONLY);
	if(file_fd < 0)
		return -1;

	file_size = Utils::get_filesize(filepath);
	if(file_size < 0)
		return -1;

	while(send_bytes < file_size) {
		bytes = sendfile(fd, file_fd, 0, file_size); // but it takes too long

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
		logging.error("server couldn't receive or request size is 0 bytes");
		return;
	}

	logging.info("server received %d bytes from %s:%s", received_bytes, client.ip_addr, client.port);

	if(Http::parse_http_header(request, (struct Http::msg&) _request) < 0) {
		logging.error("server couldn't parse HTTP request");
		return;
	}
	
	logging.info("HTTP request is parsed");

	logging.info(
		"%s:%s %s, requests %s%s",
		client.ip_addr, client.port, _request.method.data(), this->root_path, _request.path.data());

	if(_request.method == "GET")
		this->handle_get(_request, client);
}

int Server::accept_client(class Client &client) {
	socklen_t size = sizeof(client.client_sock);

	client.client_fd = accept(this->server_fd, (struct sockaddr*) &client.client_sock, &size);
	if(client.client_fd < 0)
		return -1;

	//if(fcntl(client.client_fd, F_SETFL, O_NONBLOCK) < 0)
		//return -1;

	getnameinfo((struct sockaddr*) &client.client_sock, sizeof(client.client_sock),
				client.hostname, sizeof(client.hostname),
				client.service, sizeof(client.service), 0);

	Utils::converto(Utils::IP4_ADDR_STR, client.client_sock.sin_addr.s_addr, client.ip_addr);
	Utils::converto(Utils::PORT_STR, client.client_sock.sin_port, client.port);
	return 0;
}

int Server::php_service(std::string &php_file, std::string &dest) {

#define EXIT_PHP(a) r_value = a; goto php_service_exit;

	int pipe_fd[2], null_fd, status, r_value = 0; // [0] read end, [1] write end
	pid_t php_pid;
	struct timeval timeout = { .tv_sec = 3, .tv_usec = 0 }; // Exactly 3 seconds
	fd_set readfds, writefds;

	if(pipe(pipe_fd) < 0) {
		logging.error("server can't create pipes for PHP service");
		EXIT_PHP(-1);
	}

	if((null_fd = open("/dev/null", O_WRONLY)) < 0) {
		logging.error("server can't open /dev/null");
		EXIT_PHP(-1);
	}

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_SET(pipe_fd[0], &readfds);
	FD_SET(pipe_fd[1], &writefds);

	if(select(pipe_fd[1] + 1, &readfds, &writefds, nullptr, &timeout) < 1) { // Checking whetever created pipes are ready to be reading or writing
		logging.error("server received that select() returns either non-zero or zero");
		EXIT_PHP(-1);
	}

	//if(!FD_ISSET(pipe_fd[0], &readfds) || !FD_ISSET(pipe_fd[1], &writefds)) { // only pipe_fd[0] (read pipe) isn't present. hmm
		//logging.error("either a set of FD_ISSET() return false");
		//goto php_cleanup;
	//}

	if(fcntl(pipe_fd[0], F_SETFL, O_NONBLOCK) < 0) { // so that no blocking during reading
		logging.error("server can't modify read end fd");
		EXIT_PHP(-1);
	}

	php_pid = fork();
	if(php_pid < 0) {
		logging.error("server can't fork for PHP service");
		EXIT_PHP(-1);
	}

	else if(php_pid == 0) {
		if(dup2(pipe_fd[1], STDOUT_FILENO) < 0) {
			logging.error("server can't redirect write pipe fd to STDOUT_FILENO");
			exit(-1);
		}
		if(dup2(null_fd, STDERR_FILENO) < 0) {
			logging.error("server can't redirect STDERR fd to /dev/null");
			exit(-1);
		}

		execl("/usr/bin/php", "-f", php_file.data());
		exit(-1); // This line would be executed if execl() is failed to run php
	}
	
	waitpid(php_pid, &status, 0);

	if(!WIFEXITED(status) || (WEXITSTATUS(status) != 0)) {
		logging.error("PHP isn't returned normally");
		EXIT_PHP(-1);
	}

	if(read_data(pipe_fd[0], dest) < 0) {
		logging.error("server can't receive message from PHP service");
		EXIT_PHP(-1);
	}

php_service_exit:

	::close(pipe_fd[0]);
	::close(pipe_fd[1]);
	::close(null_fd);

	return r_value;
}

/*
	Those below functions used to handle client request,
	e.g GET, PUT, and so on. Currently only GET
*/

#define PUSH(a, b) Http::push_token((struct Http::msg&) _response, (char *) a, b);

void Server::handle_get(struct Http::request_msg &_request, struct Client &client) {
	struct Http::response_msg _response;
	std::string request_path, response, body;
	bool sendfile;
	// true -> Used when sending a file. It's directly sending file to client instead of loading into memory. It helps reduce time consuming and improve server latency and memory.
	// false -> Used when sending non-file (e.g hardcoded data). The data stored in memory, and need to send client.
	
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
		logging.info("%s is found", request_path.data());

		std::string mimetype;
		sendfile = true;

		mimetype = Utils::get_mimetype(request_path);
		if(mimetype.empty())
			mimetype = Utils::mime_types["txt"];

		else if(mimetype == Utils::mime_types["php"]) {
			if(Server::php_service(request_path, body) < 0)
				logging.error("something went wrong when server communicates with PHP");
			
			else {
				sendfile = false;
		
				PUSH("Content-Length", body.size());
				PUSH("Content-Type", Utils::mime_types["html"].data());
			}
		}
		else {
			PUSH("Content-Length", Utils::get_filesize(request_path));
			PUSH("Content-Type", mimetype.data());
		}

		_response.status_code = std::to_string(200);
		_response.status_desc = Http::status_messages[200];
		
		PUSH("Content-Location", request_path.data() + std::strlen(this->root_path));
	}
	else {
		logging.warn("%s isn't found", request_path.data());
		
		sendfile = false;
		
		body = "<h1>we couldn't found requested resource</h1>";

		_response.status_code = std::to_string(404);
		_response.status_desc = Http::status_messages[404];

		PUSH("Content-Type", (char *) "text/html");
		PUSH("Content-Length", body.size());
		
		request_path = this->root_path;
		request_path += "/not-found.html";
	}
	
	Http::create_http_header_str((struct Http::msg&) _response, response); // Append HTTP header
	response.append("\r\n"); // Then empty line to seperate HTTP header and Body 

	if(this->send_data(client.client_fd, response) < 0) {
		logging.error("failure to send %s:%s request", client.ip_addr, client.port);
		return;
	}
	
	if(sendfile)
		if(this->send_file(client.client_fd, request_path) < 0)
			logging.error("failure to send %s:%s %s request", client.ip_addr, client.port, request_path.data());
		else
			logging.info("success to send %s:%s %s request", client.ip_addr, client.port, request_path.data());

	else
		if(this->send_data(client.client_fd, body) < 0)
			logging.error("failure to send %s:%s %s request", client.ip_addr, client.port, request_path.data());
		else
			logging.info("success to send %s:%s %s request", client.ip_addr, client.port, request_path.data());
}
