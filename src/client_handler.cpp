#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "client_handler.h"
#include "redis_command.h"

ClientHandler::ClientHandler(int client_fd) : client_fd(client_fd) {}

void ClientHandler::handle_client() {
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
			handle_ping();
		} else if (request_command.get_command_type() == "ECHO") {
			handle_echo(request_command.get_command_args().front());
		} else if (request_command.get_command_type() == "SET") {
			handle_set(request_command.get_command_args()[0],
					   request_command.get_command_args()[1]);
		} else if (request_command.get_command_type() == "GET") {
			handle_get(request_command.get_command_args()[0]);
		}
	}
}

void ClientHandler::handle_ping() {
	const char *response_to_ping = "+PONG\r\n";
	if (send(client_fd, response_to_ping, strlen(response_to_ping), 0) == -1) {
		std::cerr << "send message failed\n";
		exit(1);
	}
	std::cout << "Sent PONG\n";
}

void ClientHandler::handle_echo(const std::string &message) {
	std::string response_to_echo_str = "+" + message + "\r\n";
	const char *response_to_echo = response_to_echo_str.c_str();
	if (send(client_fd, response_to_echo, strlen(response_to_echo), 0) == -1) {
		std::cerr << "send message failed\n";
		exit(1);
	}
	std::cout << "Sent ECHO\n";
}

void ClientHandler::handle_set(const std::string &key,
							   const std::string &value) {
	if (data.find(key) != data.end()) {
		data[key] = value;
	} else {
		data.insert({key, value});
	}

	// Send the response
	std::string response_to_set_str = "+OK\r\n";
	const char *response_to_set = response_to_set_str.c_str();
	if (send(client_fd, response_to_set, strlen(response_to_set), 0) == -1) {
		std::cerr << "send message failed\n";
		exit(1);
	}
	std::cout << "Sent OK\n";
}

void ClientHandler::handle_get(const std::string &key) {
	std::string response_to_get_str;
	if (data.find(key) == data.end()) {
		response_to_get_str = "$-1\r\n";
	} else {
		std::string value = data[key];
		response_to_get_str =
			"$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
	}
	const char *response_to_get = response_to_get_str.c_str();
	if (send(client_fd, response_to_get, strlen(response_to_get), 0) == -1) {
		std::cerr << "send message failed\n";
		exit(1);
	}
	std::cout << "Sent GET response\n";
}
