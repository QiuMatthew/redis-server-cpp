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
#include <vector>

#include "client_handler.h"
#include "network_utils.h"
#include "redis_command.h"
#include "server.h"

void handle_client(int client_fd) {
	std::unordered_map<std::string, std::string> data = {};
	while (true) {
		// get the request message
		std::vector<char> request_msg(100);
		ssize_t bytes_received =
			recv(client_fd, request_msg.data(), request_msg.size(), 0);
		if (bytes_received == -1) {
			std::cerr << "recv failed\n";
			exit(1);
		}
		if (bytes_received == 0) {
			std::cout << "Client disconnected\n";
			return;
		}
		std::cout << "Received message length: " << bytes_received << std::endl;
		std::string request_msg_str(request_msg.begin(),
									request_msg.begin() + bytes_received);
		std::cout << "Received message: " << request_msg_str << std::endl;

		// Parse the request message
		RedisCommand request_command(request_msg_str);

		// Handle the request
		if (request_command.get_command_type() == "PING") {
			// Handle PING command
			const char *response_to_ping = "+PONG\r\n";
			if (send(client_fd, response_to_ping, strlen(response_to_ping),
					 0) == -1) {
				std::cerr << "send message failed\n";
				exit(1);
			}
			std::cout << "Sent PONG\n";
		} else if (request_command.get_command_type() == "ECHO") {
			// Handle ECHO command
			std::string response_to_echo_str =
				"+" + request_command.get_command_args()[0] + "\r\n";
			const char *response_to_echo = response_to_echo_str.c_str();
			if (send(client_fd, response_to_echo, strlen(response_to_echo),
					 0) == -1) {
				std::cerr << "send message failed\n";
				exit(1);
			}
			std::cout << "Sent ECHO\n";
		} else if (request_command.get_command_type() == "SET") {
			// Handle SET command
			// Add the key-value pair to the dictionary
			if (data.find(request_command.get_command_args()[0]) ==
				data.end()) {
				data.insert({request_command.get_command_args()[0],
							 request_command.get_command_args()[1]});
			} else {
				data[request_command.get_command_args()[0]] =
					request_command.get_command_args()[1];
			}

			// Send the response
			std::string response_to_set_str = "+OK\r\n";
			const char *response_to_set = response_to_set_str.c_str();
			if (send(client_fd, response_to_set, strlen(response_to_set), 0) ==
				-1) {
				std::cerr << "send message failed\n";
				exit(1);
			}
			std::cout << "Sent OK\n";
		} else if (request_command.get_command_type() == "GET") {
			// Handle GET command
			// Get the value from the dictionary
			std::string response_to_get_str;
			if (data.find(request_command.get_command_args()[0]) ==
				data.end()) {
				response_to_get_str = "$-1\r\n";
			} else {
				std::string value = data[request_command.get_command_args()[0]];
				response_to_get_str = "$" + std::to_string(value.size()) +
									  "\r\n" + value + "\r\n";
			}
			const char *response_to_get = response_to_get_str.c_str();
			std::cout << "Response to GET: " << response_to_get << std::endl;
			if (send(client_fd, response_to_get, strlen(response_to_get), 0) ==
				-1) {
				std::cerr << "send message failed\n";
				exit(1);
			}
			std::cout << "Sent GET response\n";
		}
	}
	return;
}

Server::Server(int port) {
	this->port = port;

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
	ClientHandler client_handler(client_fd);
	std::thread client_thread(&ClientHandler::handle_client, client_handler);
	client_thread.detach();
}

void Server::stop() { close(this->server_fd); }

int main(int argc, char **argv) {
	// Flush after every std::cout / std::cerr
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	Server server(6379);
	server.start();
	server.stop();

	return 0;
}
