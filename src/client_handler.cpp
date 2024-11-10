#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
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
			if (request_command.get_command_args().size() == 2) {
				handle_set(request_command.get_command_args()[0],
						   request_command.get_command_args()[1]);
			} else if (request_command.get_command_args().size() == 4) {
				handle_set(request_command.get_command_args()[0],
						   request_command.get_command_args()[1],
						   {request_command.get_command_args()[2],
							request_command.get_command_args()[3]});
			}
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

void ClientHandler::handle_set(const std::string &key, const std::string &value,
							   const std::vector<std::string> &optional_args) {
	// Flags for expiration
	bool has_expire = false;
	int expire_time = 0;

	// Parse the optional arguments
	if (optional_args.size() == 2) {
		const std::string &option = optional_args[0];
		const std::string &time_str = optional_args[1];

		if (option == "PX" || option == "px") {
			has_expire = true;

			// get current time in milliseconds since Jan 1, 1970
			auto now = std::chrono::system_clock::now();
			auto duration = now.time_since_epoch();
			auto duration_in_ms =
				std::chrono::duration_cast<std::chrono::milliseconds>(duration);
			int current_time = duration_in_ms.count();

			// get the expire time in milliseconds
			expire_time = current_time + std::stoi(time_str);
		} else {
			std::cerr << "Invalid SET command option: " << option << "\n";
			return;
		}
	}

	// Set the key-value pair
	if (data.find(key) != data.end()) {
		data[key] = value;
	} else {
		data.insert({key, value});
	}

	// Set the expire time
	if (has_expire) {
		if (expiration.find(key) != expiration.end()) {
			expiration[key] = std::to_string(expire_time);
		} else {
			expiration.insert({key, std::to_string(expire_time)});
		}
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
	int current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
						   std::chrono::system_clock::now().time_since_epoch())
						   .count();
	if (data.find(key) == data.end()) {
		response_to_get_str = "$-1\r\n";
	} else if (expiration.find(key) != expiration.end() &&
			   current_time > stoi(expiration[key])) {
		data.erase(key);
		expiration.erase(key);
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
