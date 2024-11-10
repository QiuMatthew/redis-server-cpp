#pragma once

#include <netinet/in.h>

int create_socket(void);
void set_socket_options(int server_fd);
void bind_socket(int server_fd, struct sockaddr_in server_addr);
void listen_socket(int server_fd, int connection_backlog);
int accept_socket(int server_fd, struct sockaddr_in client_addr,
				  int client_addr_len);
