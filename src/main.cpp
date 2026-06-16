#include "../include/logging.hpp"
#include "../include/webserver.hpp"
#include "../include/utils.hpp"

Logging logging;
Server server;

int main(int argc, char *argv[]) {
	double start, end;

	start = Utils::get_time();

	if(argc < 4) {
		std::printf("usage: %s host service root\n", argv[0]);
		return 1;
	}

	logging.info("server setup is running...");

	server.root_path = argv[3]; // Set the root path
	
	if(server.initialize() < 0) {
		logging.error("server.initialize() returns non-zero");
		return 1;
	}

	if(server.create_sockets(argv[1], argv[2]) < 0) {
		logging.error("server.create_socket() returns non-zero");
		return 1;
	}
	
	logging.info("server socket created successfully");

	if(server.set_socket_options() < 0) {
		logging.error("server.set_socket_option() returns non-zero");
		return 1;
	}
	
	logging.info("socket options set successfully");
	
	if(server.bind_sockets() < 0) {
		logging.error("server.bind_socket() returns non-zero");
		return 1;
	}
	
	logging.info("socket bounded to %s:%s successfully", server.ip_addr, server.port);

	if(server.listen_sockets() < 0) {
		logging.error("server.listen_socket() returns non-zero");
		return 1;
	}

	logging.info("socket now is listening connections");

	server.run();
	logging.info("server terminated");
	
	end = Utils::get_time();
	logging.info("program was running for about %.2f seconds", end - start);
	return 0;
}
