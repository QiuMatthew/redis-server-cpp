#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class ClientHandler {
   public:
	ClientHandler(int client_fd);
	void handle_client();

   private:
	int client_fd;
	std::unordered_map<std::string, std::string> data;
	std::unordered_map<std::string, std::string> expiration;

	void handle_ping();
	void handle_echo(const std::string& message);
	void handle_set(const std::string& key, const std::string& value,
					const std::vector<std::string>& optional_args = {});
	void handle_get(const std::string& key);
};
