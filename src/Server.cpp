#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

#include "client_handler.h"
#include "network_utils.h"
#include "server.h"

Server::Server(int port, std::unordered_map<std::string, std::string> args) {
	// store the config
	this->port = port;
	this->config = args;

	// create a socket
	this->server_fd = create_socket();

	// set reuse address option
	set_socket_options(this->server_fd);

	// set up server address
	this->server_addr.sin_family = AF_INET;
	this->server_addr.sin_addr.s_addr = INADDR_ANY;
	this->server_addr.sin_port = htons(port);

	// bind to port
	bind_socket(this->server_fd, this->server_addr);
	std::cout << "Socket bind to port " << port << std::endl;
}

void Server::start() {
	int connection_backlog = 5;
	// start listening
	listen_socket(this->server_fd, connection_backlog);
	std::cout << "Waiting for a client to connect...\n";
	while (true) {
		accept_client();
	}
}

void Server::accept_client() {
	// set up client address
	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);

	// establish connection with a client
	int client_fd =
		accept_socket(this->server_fd, client_addr, client_addr_len);
	std::cout << "Client connected\n";

	// handle client
	ClientHandler client_handler(client_fd, this->config);
	std::thread client_thread(&ClientHandler::handle_client, client_handler);
	client_thread.detach();
}

void Server::stop() { close(this->server_fd); }

int main(int argc, char **argv) {
	// Flush after every std::cout / std::cerr
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	// parse args, the program will be run as ./server --dir <path_to_directory>
	// --dbfilename <dbfilename>
	std::string dir = "";
	std::string dbfilename = "";
	for (int i = 1; i < argc; i++) {
		if (std::string(argv[i]) == "--dir") {
			dir = argv[i + 1];
		} else if (std::string(argv[i]) == "--dbfilename") {
			dbfilename = argv[i + 1];
		}
	}
	std::unordered_map<std::string, std::string> args = {
		{"dir", dir}, {"dbfilename", dbfilename}};

	Server server(6379, args);
	server.start();
	server.stop();

	return 0;
}
