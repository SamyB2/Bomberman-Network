#include "udp.h"

#include <netinet/in.h>

#define BUFFER_SIZE 1024

int create_server_socket(void);
void set_socket_options(int server_fd);
void bind_and_listen(int server_fd, struct sockaddr_in6 *address);
int create_server(__uint16_t port);
void send_multicast_info(int socket, int code_req, int id, int eq,
                         uint16_t udp_port, uint16_t mdiff_port);