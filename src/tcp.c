#include "../include/tcp.h"
#include "../include/format.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


int create_server_socket() {
  int server_fd;
  if ((server_fd = socket(AF_INET6, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }
  return server_fd;
}

void set_socket_options(int server_fd) {
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }

  int no = 0;
  if (setsockopt(server_fd, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no,
                 sizeof(no)) < 0) {
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }
}

// bind and listen to the server
void bind_and_listen(int server_fd, struct sockaddr_in6 *address) {
  if (bind(server_fd, (struct sockaddr *)address, sizeof(*address)) < 0) {
    perror("bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 3) < 0) {
    perror("listen failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }
}

int create_server(__uint16_t port) {
  int server_fd = create_server_socket();
  set_socket_options(server_fd);

  struct sockaddr_in6 address;
  memset(&address, 0, sizeof(address));
  address.sin6_family = AF_INET6;
  address.sin6_addr = in6addr_any;
  address.sin6_port = htons(port);

  bind_and_listen(server_fd, &address);

  return server_fd;
}

// send multicast info to the client
void send_multicast_info(int socket, int code_req, int id, int eq, uint16_t udp_port,
                         uint16_t mdiff_port) {
  char *multicast_info =
      serv_start_request(code_req, id, eq, udp_port, mdiff_port, MULTICAST_IP);
  size_t msg_len = sizeof(uint16_t) * 3 + 4 * sizeof(uint32_t);
  size_t sent = 0;

  while (sent < msg_len) {
    ssize_t bytes_sent = send(socket, multicast_info + sent, msg_len - sent, 0);
    if (bytes_sent < 0) {
      perror("send failed");
      free(multicast_info);
    }
    sent += (size_t)bytes_sent;
  }
  free(multicast_info);
}