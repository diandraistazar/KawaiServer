// C libraries
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

// C++ libraries
#include <fstream>
#include <filesystem>
#include <iostream>

// My own libraries
#include "../include/netlinux-utils.h"
#include "../include/mimetypes.h"
#include "../include/http.hpp"

void fatal(const char *at_line) {
	std::fprintf(stderr, "%s: [%s] %s\n", at_line, strerrorname_np(errno), strerrordesc_np(errno));

	exit(EXIT_FAILURE);
}

void handling_connection(int clientfd, struct sockaddr_in &client) {
	char request[1024] = {0}, hostname[128], service[32];
	struct http::request_msg http_req;
	struct http::response_msg http_res;
	std::string response;

	if(recv(clientfd, request, sizeof(request), 0) == -1)
		return;
	
	if(http::parse_http_header(request, (struct http::msg&) http_req) == -1)
		return;

	getnameinfo((struct sockaddr*) &client, sizeof(client), hostname, sizeof(hostname), service, sizeof(service), 0);

	std::printf(
		"####################"
		"\n%s"
		"Host: %s (%s)  Service: %s\n",
		request, inet_ntoa(client.sin_addr), hostname, service
	);
	
#define PUSH(a, b) http::push_token((struct http::msg&) http_res, (char *) a, (char *) b);
	
	std::stringstream file_buffer, file_size_str;

	http_res.mode = http::HTTP_RESPONSE;
	http_res.version = "HTTP/1.1";
	PUSH("Server", "RentGirlFriend OS");

	if(!std::filesystem::exists(http_req.path)) {
		file_buffer << "<h1><b>Not Found - 404</b></h1>";
		file_size_str << file_buffer.str().size();
		
		response.insert(0, file_buffer.str());	

		http_res.status_code = "404";
		http_res.status_desc = "Not Found";
		
		PUSH("Content-Type", "text/html");
		PUSH("Content-Length", file_size_str.str().c_str());
		goto send_data;
	}

	printf("[SUC] Request '%s' is exists.\n", http_req.path.c_str());

	if(std::filesystem::is_regular_file(http_req.path)) {
		std::ifstream read_request_file(http_req.path);
		file_buffer << read_request_file.rdbuf();
		read_request_file.close();
		
		file_size_str << std::filesystem::file_size(http_req.path);
		response.insert(0, file_buffer.str());
		
		http_res.status_code = "200";
		http_res.status_desc = "OK";
		
		char *mime_type = (char *) get_mimetype(http_req.path.c_str());
		if(mime_type == nullptr)
			mime_type = (char *) "text/plain";

		PUSH("Content-Type", mime_type);
		PUSH("Content-Length", file_size_str.str().c_str());
	}
	else if(std::filesystem::is_directory(http_req.path)) {
		file_buffer << "<h1><b>Bro you accessed a directory, what did you do this?</b></h1>";
		file_size_str << file_buffer.str().size();
		
		response.insert(0, file_buffer.str());	

		http_res.status_code = "403";
		http_res.status_desc = "Forbidden";
		
		PUSH("Content-Type", "text/html");
		PUSH("Content-Length", file_size_str.str().c_str());
	}
	
send_data:

	response.insert(0, "\r\n");
	response.insert(0, http::create_http_header_str((struct http::msg&) http_res));

	if(send_data(clientfd, (void *)response.c_str(), response.size(), 0) == -1)
		return;

	printf("[SUC] The request data have send to '%s'.\n", inet_ntoa(client.sin_addr));
}

int main(int argc, char *argv[]) {
	if(argc < 3) {
		std::printf("Usage: %s host service\n", argv[0]);
		return 1;
	}	

	// Get localhost information like family, flags, socktype protocols, and so on
	// I'm too lazy to setup everything from scratch, why don't we use the faster one?
	struct addrinfo hints, *addr;

	hints.ai_flags = 0;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	std::printf("-----------------------------------\n");

	int ret = getaddrinfo(argv[1], argv[2], &hints, &addr);
	std::printf("getaddrinfo status: %s\n\n", gai_strerror(ret));
	if(addr == NULL)
		return 1;
	
	// Creating a socket and store the file desc returned by socket() into sockfd
	struct sockaddr_in *sock = (struct sockaddr_in*)addr->ai_addr;
	int sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if(sockfd == -1)
		fatal("socket()");

	char port[6], ip_addr[16], family[16], socktype[16], protocol[16];
	ADDRSOCK_STR(FAMILY, addr->ai_family, family);
	ADDRSOCK_STR(SOCKTYPE, addr->ai_socktype, socktype);
	ADDRSOCK_STR(PROTOCOL, addr->ai_protocol, protocol);
	
	freeaddrinfo(addr);
	
	// I want to see the created socket's information in readable format
	std::printf(
		"SUCCESSFULLY to create a socket:\n"
		"Family    : %s\n"
		"Socktype  : %s\n"
		"Protocol  : %s\n\n",
		family, socktype, protocol
	);

	// I am going to set options to control socket behavior. I use SOL_SOCKET, which means socket-level.
	int yes = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		fatal("setsockopt()");

	// Hmm, the created socket currently isn't bind any addresss, so that I am going to bind it with my localhost IPv4
	if(bind(sockfd, (struct sockaddr*)sock, sizeof(struct sockaddr)))
		fatal("bind()");

	ADDRSOCK_STR(_FAMILY, sock->sin_family, family);
	ADDRSOCK_STR(_PORT, sock->sin_port, port);
	ADDRSOCK_STR(_ADDR, sock->sin_addr.s_addr, ip_addr);

	std::printf(
		"SUCCESSFULLY to bind the socket:\n"
		"sin_family = %s\n"
		"sin_port = %s\n"
		"sin_addr.s_addr = %s\n",
		family, port, ip_addr
	);
	
	if(listen(sockfd, 1) == -1)
		fatal("listen()");

	std::printf("-----------------------------------\n");
	std::printf("Listening...\n");
	do {
		struct sockaddr_in client;
		socklen_t client_size = sizeof(client);

		int clientfd = accept(sockfd, (struct sockaddr*) &client, &client_size);
		if(clientfd  == -1)
			fatal("accept()");
		
		handling_connection(clientfd, client);

		close(clientfd);

	} while(1);

	close(sockfd);
	return EXIT_SUCCESS;
}
