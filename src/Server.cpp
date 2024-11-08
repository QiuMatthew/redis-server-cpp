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

class RedisCommand {
   public:
	RedisCommand(std::string raw_command) {
		// Parse the raw command
		int argc =
			std::stoi(raw_command.substr(1, raw_command.find("\r\n") - 1));
		raw_command = raw_command.substr(raw_command.find("\r\n") + 2);
		// get the command
		int length =
			std::stoi(raw_command.substr(1, raw_command.find("\r\n") - 1));
		raw_command = raw_command.substr(raw_command.find("\r\n") + 2);
		command_type = raw_command.substr(0, raw_command.find("\r\n"));
		for (char &c : command_type) {
			c = toupper(c);
		}
		raw_command = raw_command.substr(raw_command.find("\r\n") + 2);
		std::cout << "Command: " << command_type << std::endl;
		// get the arguments
		for (int i = 0; i < argc - 1; i++) {
			int length =
				std::stoi(raw_command.substr(1, raw_command.find("\r\n") - 1));
			raw_command = raw_command.substr(raw_command.find("\r\n") + 2);
			arguments.push_back(raw_command.substr(0, length));
			raw_command = raw_command.substr(raw_command.find("\r\n") + 2);
		}
	}

	std::string get_command() { return command_type; }
	std::vector<std::string> get_arguments() { return arguments; }

   private:
	std::string command_type;
	std::vector<std::string> arguments;
};

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

void handle_client(int client_fd) {
	std::unordered_map<std::string, std::string> dict = {};
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
		if (request_command.get_command() == "PING") {
			// Handle PING command
			const char *response_to_ping = "+PONG\r\n";
			if (send(client_fd, response_to_ping, strlen(response_to_ping),
					 0) == -1) {
				std::cerr << "send message failed\n";
				exit(1);
			}
			std::cout << "Sent PONG\n";
		} else if (request_command.get_command() == "ECHO") {
			// Handle ECHO command
			std::string response_to_echo_str =
				"+" + request_command.get_arguments()[0] + "\r\n";
			const char *response_to_echo = response_to_echo_str.c_str();
			if (send(client_fd, response_to_echo, strlen(response_to_echo),
					 0) == -1) {
				std::cerr << "send message failed\n";
				exit(1);
			}
			std::cout << "Sent ECHO\n";
		} else if (request_command.get_command() == "SET") {
			// Handle SET command
			// Add the key-value pair to the dictionary
			if (dict.find(request_command.get_arguments()[0]) == dict.end()) {
				dict.insert({request_command.get_arguments()[0],
							 request_command.get_arguments()[1]});
			} else {
				dict[request_command.get_arguments()[0]] =
					request_command.get_arguments()[1];
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
		} else if (request_command.get_command() == "GET") {
			// Handle GET command
			// Get the value from the dictionary
			std::string response_to_get_str;
			if (dict.find(request_command.get_arguments()[0]) == dict.end()) {
				response_to_get_str = "$-1\r\n";
			} else {
				std::string value = dict[request_command.get_arguments()[0]];
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

int main(int argc, char **argv) {
	// Flush after every std::cout / std::cerr
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	// create a socket
	int server_fd = create_socket();

	// set reuse address option
	set_socket_options(server_fd);

	// set up server address
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(6379);

	// bind to port
	bind_socket(server_fd, server_addr);
	std::cout << "Socket bind to port 6379\n";

	// start listening
	int connection_backlog = 5;
	listen_socket(server_fd, connection_backlog);
	std::cout << "Waiting for a client to connect...\n";

	// set up client address
	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);

	// receive and send messages
	while (true) {
		// establish connection with a client
		int client_fd = accept_socket(server_fd, client_addr, client_addr_len);
		std::cout << "Client connected\n";

		// handle client
		std::thread client_thread(handle_client, client_fd);
		client_thread.detach();
	}

	close(server_fd);

	return 0;
}
