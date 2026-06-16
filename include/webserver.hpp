#pragma once

#include <netdb.h>
#include <sys/un.h>
#include "http.hpp"

#define LOCAL_PATH "/tmp/kawaiserver-php-local"
#define LOCAL_PATH_LEN sizeof(LOCAL_PATH)

struct Client {
	struct sockaddr_in client_sock;
	char ip_addr[16] = {0}, port[16] = {0};
	char hostname[128] = {0}, service[16] = {0};
	int client_fd;
	
	~Client();
	void close();
};

struct Server {
	struct sockaddr_in server_sock;
	struct sockaddr_un php_sock;
	char *root_path = nullptr;
	char ip_addr[16] = {0}, port[16] = {0};
	int server_fd, php_fd;
	bool is_continue = true;
	
	~Server();
	void close();
	int initialize();
	void run();
	int create_sockets(char *host, char *service);
	int set_socket_options();
	int bind_sockets();
	int listen_sockets();
	
private:
	static void terminate(int signal);
	
	size_t recv_data(int fd, std::string &dest);
	size_t send_data(int fd, std::string &buffer);
	size_t send_file(int fd, std::string &filepath);
	void handling_client(struct Client &client);
	int accept_client(class Client &client);

	void handle_get(struct Http::request_msg &_request, struct Client &client);
};
