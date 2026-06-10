#include "../include/logging.hpp"
#include "../include/webserver.hpp"

Logging logging;

int main(int argc, char *argv[]) {
	if(argc < 3) {
		std::printf("usage: %s host service\n", argv[0]);
		return 1;
	}

	logging.info("setup server is starting...");

	Server server;
	if(server.create_sock(argv[1], argv[2], 0, AF_INET, SOCK_STREAM, 0) == -1) {
		logging.error("server.create_sock() returns -1");
		return 1;
	}
	
	logging.info("server socket created successfully");

	int yes = 1;
	if(server.set_sockopt(SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		logging.error("server.set_sockopt() returns -1");
		return 1;
	}
	
	logging.info("socket options set successfully");
	
	if(server.bind_sock() == -1) {
		logging.error("server.bind_sock() returns -1");
		return 1;
	}
	
	logging.info("socket bounded to %s:%s successfully", server.ip_addr, server.port);

	if(server.listen_sock(1) == -1) {
		logging.error("server.listen_sock() returns -1");
		return 1;
	}

	logging.info("socket now is listening connections");
	logging.info("server opened HTTP service");

	server.start();
	logging.info("server terminated");
	return 0;
}
