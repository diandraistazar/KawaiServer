#include "../include/logging.hpp"
#include "../include/webserver.hpp"
#include "../include/utils.hpp"

Server server;

int main(int argc, char *argv[]) {
	double start, end;

	start = Utils::get_time();

	if(argc < 4) {
		std::printf("usage: %s host service root\n", argv[0]);
		return 1;
	}

	Logging::level = Logging::LOG_TRACE;
	server.root_path = argv[3]; // Set the root path
	
	Logging::info("server setup is running...");
	
	if(server.initialize() < 0) {
		Logging::error("server.initialize() returns non-zero");
		return 1;
	}

	if(server.create_sockets(argv[1], argv[2]) < 0) {
		Logging::error("server.create_socket() returns non-zero");
		return 1;
	}
	
	Logging::info("server socket created successfully");

	if(server.set_socket_options() < 0) {
		Logging::error("server.set_socket_option() returns non-zero");
		return 1;
	}
	
	Logging::info("socket options set successfully");
	
	if(server.bind_sockets() < 0) {
		Logging::error("server.bind_socket() returns non-zero");
		return 1;
	}
	
	Logging::info("socket bounded to %s:%s successfully", server.ip_addr, server.port);

	if(server.listen_sockets() < 0) {
		Logging::error("server.listen_socket() returns non-zero");
		return 1;
	}

	Logging::info("socket now is listening connections");

	server.run();
	Logging::info("server terminated");
	
	end = Utils::get_time();
	Logging::info("program was running for about %.2f seconds", end - start);
	return 0;
}
