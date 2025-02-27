#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void send_udp_server_info(int new_socket);
void handle_client_message(int new_socket);
void *handle_client(void *arg);
int create_server_socket(void);
void set_socket_options(int server_fd);
void bind_and_listen(int server_fd, struct sockaddr_in6 *address);
void accept_connections(int server_fd, struct sockaddr_in6 *address);
void *server_udp(void *arg);