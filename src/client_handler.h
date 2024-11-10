#pragma once

#include <string>
#include <unordered_map>

class ClientHandler {
   public:
	ClientHandler(int client_fd);
	void handle_client();

   private:
	int client_fd;
	std::unordered_map<std::string, std::string> data;

	void handle_ping();
	void handle_echo(const std::string& message);
	void handle_set(const std::string& key, const std::string& value);
	void handle_get(const std::string& key);
};
