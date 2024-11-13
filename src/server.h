#pragma once
#include <netinet/in.h>

#include <unordered_map>

class Server {
   public:
	Server(int port, std::unordered_map<std::string, std::string> args);
	void start();
	void stop();

   private:
	int port;
	int server_fd;
	std::unordered_map<std::string, std::string> config;
	struct sockaddr_in server_addr;
	void accept_client();
};
