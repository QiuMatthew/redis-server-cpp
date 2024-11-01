#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

std::vector<char> receive_message_until_delimiter(int client_fd,
												  char delimiter) {
	std::vector<char> message;
	while (true) {
		std::vector<char> buffer(1);
		ssize_t bytes_received = recv(client_fd, buffer.data(), 1, 0);
		if (bytes_received == -1) {
			std::cerr << "recv failed\n";
			exit(1);
		}
		if (bytes_received == 0) {
			std::cout << "Client disconnected\n";
			break;
		}
		if (buffer[0] == delimiter) {
			break;
		}
		message.push_back(buffer[0]);
	}
	return message;
}

int main(int argc, char **argv) {
	// Flush after every std::cout / std::cerr
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		std::cerr << "Failed to create server socket\n";
		return 1;
	}

	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
		0) {
		std::cerr << "setsockopt failed\n";
		return 1;
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(6379);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
		0) {
		std::cerr << "Failed to bind to port 6379\n";
		return 1;
	}

	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		std::cerr << "listen failed\n";
		return 1;
	}

	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);

	std::cout << "Waiting for a client to connect...\n";

	int client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
						   (socklen_t *)&client_addr_len);
	std::cout << "Client connected\n";

	while (true) {
		std::string request_msg_str =
			receive_message_until_delimiter(client_fd, '\n').data();
		std::cout << "Received message length: " << request_msg_str.length()
				  << std::endl;
		std::cout << "Received message: " << request_msg_str << std::endl;
		const char *response_to_ping = "+PONG\r\n";
		if (send(client_fd, (const void *)response_to_ping,
				 strlen(response_to_ping), 0) == -1) {
			std::cerr << "send message failed\n";
			return 1;
		}
	}

	close(server_fd);

	return 0;
}
