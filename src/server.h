#pragma once
#include <netinet/in.h>

class Server {
   public:
	Server(int port);
	void start();
	void stop();

   private:
	int port;
	int server_fd;
	struct sockaddr_in server_addr;
	void accept_client();
};
