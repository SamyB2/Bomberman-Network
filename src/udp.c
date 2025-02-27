#include "../include/udp.h"

int create_socket() {
  int sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }
  return sockfd;
}

struct sockaddr_in6 create_address(char *ip, int port) {
  struct sockaddr_in6 addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  inet_pton(AF_INET6, ip, &(addr.sin6_addr));
  addr.sin6_port = htons((short unsigned int)port);
  addr.sin6_scope_id = 0;
  return addr;
}

void send_message(int sockfd, struct sockaddr_in6 addr, char *message) {
  if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&addr,
             sizeof(addr)) < 0) {
    perror("Sending message failed");
    exit(EXIT_FAILURE);
  }
}

void start_server(char *ip, int port, char *message) {
  int sockfd = create_socket();
  struct sockaddr_in6 addr = create_address(ip, port);

  printf("Server started. Players can subscribe to multicast group %s on "
         "port %d\n",
         ip, port);

  for (int i = 0; i < 10; i++) {
    send_message(sockfd, addr, message);
    sleep(5);
  }

  close(sockfd);
}

void *start_udp_server(void *arg) {
  char **argv = (char **)arg;
  char *ip = argv[0];
  int port = atoi(argv[1]);
  printf("Starting server on %s:%d\n", ip, port);
  char *message = "Server Connection successful!";

  start_server(ip, port, message);

  exit(EXIT_SUCCESS);
}