#include "../include/server.h"
#include "../include/udp.h"
#include "../include/format.h"


void send_udp_server_info(int new_socket) {
  // Create UDP server
  int udp_server_port = 54321; // Choose an appropriate port
  char *udp_multicast_group =
      "ff02::1"; // Choose an appropriate multicast group

  // Convert udp_server_port to string
  char udp_server_port_str[6];
  sprintf(udp_server_port_str, "%d", udp_server_port);

  // Start UDP server in a new thread
  pthread_t udp_server_thread;
  char *argv[] = {udp_multicast_group, udp_server_port_str, NULL};
  if (pthread_create(&udp_server_thread, NULL, start_udp_server, argv) != 0) {
    perror("could not create UDP server thread");
    return;
  }
  pthread_detach(udp_server_thread);

  // Send UDP server info to the client
  char udp_server_info[256];
  sprintf(udp_server_info, "%s %d", udp_multicast_group, udp_server_port);
  if (send(new_socket, udp_server_info, strlen(udp_server_info), 0) <
      (ssize_t)strlen(udp_server_info)) {
    perror("send failed");
  } else {
    printf("UDP server info sent\n");
  }
}

void handle_client_message(int new_socket) {
  char buffer[BUFFER_SIZE] = {0};
  if (read(new_socket, buffer, BUFFER_SIZE) < 0) {
    perror("read failed");
  } else {
    printf("Client message: %s\n", buffer);
    send_udp_server_info(new_socket);
  }
}

void *handle_client(void *arg) {
  int new_socket = *((int *)arg);
  free(arg);
  char buffer[BUFFER_SIZE] = {0};

  size_t valread = 0;
  size_t toread = sizeof(uint16_t);

  while(valread < toread) {
    ssize_t bytes_read = read(new_socket, buffer + valread, toread - valread);
    if (bytes_read < 0) {
      perror("read failed");
      close(new_socket);
      return NULL;
    }
    valread += (size_t)bytes_read;
  }

  uint16_t header_id;
  memcpy(&header_id, buffer, sizeof(uint16_t));
  header_id = ntohs(header_id);
  int code_req, id, eq;
  header_id_decode(header_id, &code_req, &id, &eq);
  printf("Client request: code=%d, id=%d, eq=%d\n", code_req, id, eq);

  char *addr = malloc(sizeof(uint32_t) * 4);
  memcpy(addr, "ff02::1", strlen("ff02::1") + 1);
  char *msg = serv_start_request(3, 0, 0, 54321, 8081, addr);
  free(addr);
  size_t msg_len = sizeof(uint16_t) * 3 + 4 * sizeof(uint32_t);
  size_t sent = 0;

  while (sent < msg_len) {
    ssize_t bytes_sent = send(new_socket, msg + sent, msg_len - sent, 0);
    if (bytes_sent < 0) {
      perror("send failed");
      free(msg);
      close(new_socket);
      return NULL;
    }
    sent += (size_t)bytes_sent;
  }
  pthread_t udp_thread;
  if (pthread_create(&udp_thread, NULL, server_udp, NULL) != 0) {
    perror("could not create UDP server thread");
    return NULL;
  }
  close(new_socket);
  return NULL;
}

void *server_udp(void *arg) {
  int * ff = (int *)arg;
  ff += 1;
  char *ip = "ff02::1";
  uint16_t port = 8081;
  printf("Starting server on %s : %d\n", ip, port);
  
  int sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
  struct sockaddr_in6 addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(port);
  addr.sin6_scope_id = 0;
  inet_pton(AF_INET6, ip, &(addr.sin6_addr));

  Board boardobj;
  Board *board = &boardobj;
  setup_board(board);
  gen_map(board);


  char* rqst = all_board_request(11, 2, 1, 34, board);
  size_t rqst_len = sizeof(uint16_t) * 2 + sizeof(uint8_t) * ((long unsigned int)(board->h * board->w + 2));
  size_t sent = 0;

  printf("Board sent = %ld bytes\n", rqst_len);

  while (sent < rqst_len) {
    ssize_t bytes_sent = sendto(sockfd, rqst + sent, rqst_len - sent, 0, (struct sockaddr *)&addr, sizeof(addr));
    if (bytes_sent < 0) {
      perror("sendto failed");
      free(rqst);
      close(sockfd);
      return NULL;
    }
    sent += (size_t)bytes_sent;
  }
  printf("Board sent total = %ld bytes\n", sent);
  sleep(3);
  set_grid(board, 0, 0, 0);
  set_grid(board, 1, 0, 1);
  set_grid(board, 2, 0, 0);
  set_grid(board, 3, 0, 1);
  set_grid(board, 4, 0, 2);
  free(rqst);
  rqst = change_board_request(12, 2, 1, 36, board);
  rqst_len = 2 * sizeof(uint16_t) + sizeof(uint8_t) * ((size_t)(board->changed_cells_count * 3 + 1));
  printf("Board sent2 = %ld bytes\n", rqst_len);
  sent = 0;
  while (sent < rqst_len) {
    ssize_t bytes_sent = sendto(sockfd, rqst + sent, rqst_len - sent, 0, (struct sockaddr *)&addr, sizeof(addr));
    if (bytes_sent < 0) {
      perror("sendto failed");
      free(rqst);
      close(sockfd);
      return NULL;
    }
    sent += (size_t)bytes_sent;
  }
  printf("Board sent total2 = %ld bytes\n", sent);

  free(rqst);
  close(sockfd);
  return NULL;
}

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

void accept_connections(int server_fd, struct sockaddr_in6 *address) {
  int addrlen = sizeof(*address);
  printf("Server is ready to accept connections\n");
  while (1) {
    int *new_socket = malloc(sizeof(int));
    if ((*new_socket = accept(server_fd, (struct sockaddr *)address,
                              (socklen_t *)&addrlen)) < 0) {
      perror("accept failed");
      free(new_socket);
      continue;
    }

    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, handle_client, (void *)new_socket) !=
        0) {
      perror("could not create thread");
      free(new_socket);
      continue;
    }
    pthread_detach(thread_id);
  }
}

int main() {
  int server_fd = create_server_socket();
  set_socket_options(server_fd);

  struct sockaddr_in6 address;
  memset(&address, 0, sizeof(address));
  address.sin6_family = AF_INET6;
  address.sin6_addr = in6addr_any;
  address.sin6_port = htons(PORT);

  bind_and_listen(server_fd, &address);
  accept_connections(server_fd, &address);

  close(server_fd);
  return 0;
}