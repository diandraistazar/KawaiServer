#include <string.h>
#include "../include/webserver.hpp"

void fatal(const char *at_line) {
	std::fprintf(stderr, "%s: [%s] %s\n", at_line, strerrorname_np(errno), strerrordesc_np(errno));

	exit(1);
}

int main(int argc, char *argv[]) {
	if(argc < 3) {
		std::printf("Usage: %s host service\n", argv[0]);
		return 1;
	}	

	Server server;
	if(server.get_addr(argv[1], argv[2], 0, AF_INET, SOCK_STREAM, 0) == -1) {
		fatal("server.get_addr()");
		return 1;
	}
	
	if(server.create_sock() == -1) {
		fatal("server.create_sock()");
		return 1;
	}
	
	int yes = 1;
	if(server.set_sockopt(SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		fatal("server.set_sockopt()");
		return 1;
	}
	
	if(server.bind_sock() == -1) {
		fatal("server.bind_sock()");
		return 1;
	}

	if(server.listen_sock(1) == -1) {
		fatal("server.listen_sock()");
		return 1;
	}

	server.start();
	return 0;
}
