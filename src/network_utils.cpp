#include <arpa/inet.h>

#include <cstdlib>
#include <iostream>

#include "network_utils.h"

int create_socket(void) {
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		std::cerr << "Failed to create server socket\n";
		exit(1);
	}
	return server_fd;
}

void set_socket_options(int server_fd) {
	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
		0) {
		std::cerr << "setsockopt failed\n";
		exit(1);
	}
	return;
}

void bind_socket(int server_fd, struct sockaddr_in server_addr) {
	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
		0) {
		std::cerr << "Failed to bind to port 6379\n";
		exit(1);
	}
	return;
}

void listen_socket(int server_fd, int connection_backlog) {
	if (listen(server_fd, connection_backlog) != 0) {
		std::cerr << "listen failed\n";
		exit(1);
	}
	return;
}

int accept_socket(int server_fd, struct sockaddr_in client_addr,
				  int client_addr_len) {
	int client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
						   (socklen_t *)&client_addr_len);
	if (client_fd < 0) {
		std::cerr << "accept failed\n";
		exit(1);
	}
	return client_fd;
}
