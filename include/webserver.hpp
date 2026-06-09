#pragma once

#include <netdb.h>
#include "http.hpp"

struct Client {
	struct sockaddr_in in_sock;
	char ip_addr[16], port[16];
	char hostname[128], service[128];
	int fd;
	
	~Client();
	void close();
};

struct Server {
	struct addrinfo addr;
	char ip_addr[16], port[16];
	int fd;
	
	~Server();
	void close();
	int get_addr(char *host, char *service, int flags, int family, int socktype, int protocol);
	int create_sock();
	int set_sockopt(int opt, int values, void *data, size_t data_size);
	int bind_sock();
	int listen_sock(int backlog);
	void start();
	
private:
	size_t receive_data(struct Client &client, std::string &dest);
	size_t send_data(struct Client &client, std::string &source);
	void handling_client(struct Client &client);
	int accept_client(class Client &client);

	void handle_get(std::string &dest, struct Http::request_msg &request);
};
