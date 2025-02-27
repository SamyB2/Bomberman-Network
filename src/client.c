#include "../include/client.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int connect_to_tcp(const char *server_url, uint16_t server_port, int *sockfd) {
  struct addrinfo hints, *res;
  char port_str[6];

  sprintf(port_str, "%u", server_port);

  // Prepare hints structure
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // Resolve the server URL
  int err = getaddrinfo(server_url, port_str, &hints, &res);
  if (err != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    return ERROR_INVALID_ADDRESS;
  }

  // Create socket
  *sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (*sockfd == -1) {
    perror("Socket creation failed");
    freeaddrinfo(res);
    return ERROR_SOCKET_CREATION;
  }

  // Connect to the server
  int attempts = 0;
  while (connect(*sockfd, res->ai_addr, res->ai_addrlen) < 0) {
    if (++attempts == MAX_ATTEMPTS) {
      perror("Connection failed");
      freeaddrinfo(res);
      close(*sockfd);
      return ERROR_CONNECTION_FAILED;
    }
    sleep(5);
  }

  freeaddrinfo(res);
  return SUCCESS;
}

int create_socket_udp(uint16_t portudp, int *sock_udp,
                      struct sockaddr_in6 *addr_udp_) {
  struct sockaddr_in6 addr_udp;
  int sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("Socket creation failed");
    return ERROR_SOCKET_CREATION;
  }

  // Enable address reuse
  int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) <
      0) {
    perror("setsockopt failed");
    return ERROR_SOCKET_CREATION;
  }

  // Set up the sockaddr_in6 structure
  memset(&addr_udp, 0, sizeof(addr_udp));
  addr_udp.sin6_family = AF_INET6;
  addr_udp.sin6_addr = in6addr_any;
  addr_udp.sin6_port = htons(portudp);

  // Bind the socket to the port
  if (bind(sockfd, (struct sockaddr *)&addr_udp, sizeof(addr_udp)) < 0) {
    perror("Bind failed");
    close(sockfd);
    return ERROR_SOCKET_CREATION;
  }

  *sock_udp = sockfd;
  memcpy(addr_udp_, &addr_udp, sizeof(addr_udp));
  return SUCCESS;
}

int join_game_rq(int sockfd, int gamemode) {
  uint16_t header_id = header_id_request((gamemode == 1) ? 1 : 2, 0, 0);
  return send_header_rq(sockfd, header_id);
}

int get_game_info(int sockfd, int *id, int *eq, uint16_t *port_udp,
                  uint16_t *port_mdiff, char *adrm_diff) {
  char buffer[BUFFER_SIZE];
  ssize_t rd = 0;
  uint32_t adrm_diff_[4] = {0};
  size_t data_sz = sizeof(uint16_t) * 3 + 4 * sizeof(uint32_t);
  rd = read_message(sockfd, NULL, buffer, data_sz, 0);

  if (rd < 0) {
    perror("Error reading response");
    return ERROR_READING_RESPONSE;
  }

  if (rd < (ssize_t)data_sz) {
    return ERROR_READING_RESPONSE;
  }

  // Decode message
  uint16_t header_id;
  int code_req;
  memcpy(&header_id, buffer, sizeof(uint16_t));
  header_id = ntohs(header_id);
  header_id_decode(header_id, &code_req, id, eq);
  memcpy(port_udp, buffer + sizeof(uint16_t), sizeof(uint16_t));
  *port_udp = ntohs(*port_udp);
  memcpy(port_mdiff, buffer + sizeof(uint16_t) * 2, sizeof(uint16_t));
  *port_mdiff = ntohs(*port_mdiff);
  memcpy(adrm_diff_, buffer + sizeof(uint16_t) * 3, 4 * sizeof(uint32_t));
  for (int i = 0; i < 4; i++) {
    adrm_diff_[i] = ntohl(adrm_diff_[i]);
  }
  memcpy(adrm_diff, adrm_diff_, 4 * sizeof(uint32_t));
  return SUCCESS;
}

int join_multicast_group(const char *multicast_ip, uint16_t multicast_port,
                         int *sockfd_, struct sockaddr_in6 *addr_) {
  struct sockaddr_in6 addr;
  struct ipv6_mreq mreq;
  int sockfd;

  // Create socket
  if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
    perror("Socket creation failed");
    return -1;
  }

  // Enable address reuse
  int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) <
      0) {
    perror("setsockopt failed");
    return -1;
  }

  // Set up the sockaddr_in6 structure
  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_addr = in6addr_any;
  addr.sin6_port = htons(multicast_port);

  // Bind the socket to the port
  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("Bind failed");
    close(sockfd);
    return -1;
  }

  // Join the multicast group
  if (inet_pton(AF_INET6, multicast_ip, &(mreq.ipv6mr_multiaddr.s6_addr)) <=
      0) {
    perror("inet_pton failed");
    return -1;
  }
  mreq.ipv6mr_interface = 0;

  if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq)) <
      0) {
    perror("setsockopt(IPV6_JOIN_GROUP) failed");
    close(sockfd);
    return -1;
  }
  *sockfd_ = sockfd;
  memcpy(addr_, &addr, sizeof(addr));
  return SUCCESS;
}

// Function to send a header request to the server
int send_header_rq(int sockfd, uint16_t header_id) {
  char *joining_req = malloc(sizeof(uint16_t));
  memcpy(joining_req, &header_id, sizeof(uint16_t));

  // Send joining request
  ssize_t bytes_sent = send(sockfd, joining_req, sizeof(uint16_t), 0);
  while (bytes_sent < (ssize_t)sizeof(uint16_t)) {
    bytes_sent += send(sockfd, joining_req + bytes_sent,
                       sizeof(uint16_t) - (size_t)bytes_sent, 0);
  }
  free(joining_req);
  return SUCCESS;
}

// Function to send a message to the server when the client join the game
int join_game(int sock_tcp, Server_info *serv_info, Header_info *header_info) {
  int error_code;
  uint16_t port_udp = 0;
  uint16_t port_mdiff = 0;
  char *adrm_diff = malloc(4 * sizeof(uint32_t));
  do {
    error_code = get_game_info(sock_tcp, &(header_info->id), &(header_info->eq),
                               &port_udp, &port_mdiff, adrm_diff);
  } while (error_code != SUCCESS);
  
  printf("Game is starting\n");

  error_code = join_multicast_group(
      adrm_diff, port_mdiff, &(serv_info->sock_mdff), &(serv_info->addr_mdff));

  if (error_code != SUCCESS)
    return error_code;

  error_code = create_socket_udp(port_udp, &(serv_info->sock_udp),
                                 &(serv_info->addr_udp));
  if (error_code != SUCCESS)
    return error_code;

  return SUCCESS;
}

// Functio to incode de board 
void byte_to_board(char *cells, Board *board) {
  size_t index = 0;
  char cells_mapping[] = {PATH, WALL, D_WALL, 'A', 'B', 'C',
                          'D', 0, 1 , 2 , 3};

  for (int i = 0; i < board->w * board->h; i++) {
    uint8_t cell;
    memcpy(&cell, cells + index, sizeof(uint8_t));
    index += sizeof(uint8_t);

    if (cell < 11) {
      board->grid[i] = cells_mapping[cell];
    }
  }
}

// Function to extract the changed cells from the buffer
void extract_changed_cells(char *buffer, Board *board) {
  memcpy(&(board->changed_cells_count), buffer, sizeof(uint8_t));
  int nb_cells = (int)board->changed_cells_count;
  char *cells_data = buffer + sizeof(uint8_t);
  size_t len_cell = sizeof(uint8_t) * 3;
  for (int i = 0; i < nb_cells; ++i) {
    uint8_t x, y, value;
    memcpy(&x, cells_data + (size_t)i * len_cell, sizeof(uint8_t));
    memcpy(&y, cells_data + (size_t)i * len_cell + sizeof(uint8_t),
           sizeof(uint8_t));
    memcpy(&value, cells_data + (size_t)i * len_cell + 2 * sizeof(uint8_t),
           sizeof(uint8_t));
    set_grid_cell(board, (int)x, (int)y, (char)value);
  }
}
// return 1 if there is an update, 0 otherwise
int update_board_check(int sock_mdff, struct sockaddr_in6 *addr_mdff,
                       Board *board, uint16_t *num_rq_rd, int get_dim) {
  char buffer[BUFFER_SIZE];
  // reading message from multicast
  ssize_t rd = read_message(sock_mdff, addr_mdff, buffer, BUFFER_SIZE, 1);
  // check if we got header and number of message
  if (rd < (ssize_t)sizeof(uint16_t) * 2){
    return 0;
  }
  else rd -= (ssize_t)sizeof(uint16_t) * 2;
  // decode header and number of message
  uint16_t header_id;
  uint16_t num;
  Header_info hd_info;
  memcpy(&header_id, buffer, sizeof(uint16_t));
  header_id = ntohs(header_id);
  memcpy(&num, buffer + sizeof(uint16_t), sizeof(uint16_t));
  num = ntohs(num);
  // get the dimension of the board
  if (get_dim) {
    uint8_t w, h;
    memcpy(&h, buffer + sizeof(uint16_t) * 2, sizeof(uint8_t));
    memcpy(&w, buffer + sizeof(uint16_t) * 2 + sizeof(uint8_t),
           sizeof(uint8_t));
    board->w = (int)w;
    board->h = (int)h;
  }
  if (num < 5 && *num_rq_rd >= 0xffff - 5) {
    *num_rq_rd = 0;
  }
  // check if we got the latest message
  if (num >= *num_rq_rd) {
    *num_rq_rd = num % 0xffff;
    // decode header
    header_id_decode(header_id, &hd_info.code_req, &hd_info.id, &hd_info.eq);
    // the message is the full grid
    if (hd_info.code_req == 11 &&
        rd >= (ssize_t)(sizeof(uint16_t) +
                        sizeof(uint8_t) * (size_t)(board->w * board->h)))
      byte_to_board(buffer + sizeof(uint16_t) * 3, board);
    else if (hd_info.code_req == 12 && rd >= (ssize_t)sizeof(uint8_t))
      extract_changed_cells(buffer + sizeof(uint16_t) * 2, board);
    else
      return 0;
    }else return 0;
  return 1;
}

// Function to update the chat with the message received from the server
int update_chat_check(int sock_tcp, char *buffer, int *alive, Chat *chat,
                      int *id_winner, int *eq_winner) {
  ssize_t rd = read_message(sock_tcp, NULL, buffer, BUFFER_SIZE, 0);
  if (rd < (ssize_t)sizeof(uint16_t))
    return 0;
  else
    rd -= (ssize_t)sizeof(uint16_t);

  // decode message
  uint16_t header_id;
  Header_info hd_info;
  memcpy(&header_id, buffer, sizeof(uint16_t));
  header_id = ntohs(header_id);
  header_id_decode(header_id, &hd_info.code_req, &hd_info.id, &hd_info.eq);

  char *dst[2] = {"[All] : ", "[TEAM] : "};
  size_t dst_len[2] = {8, 9};

  switch (hd_info.code_req) {
  case 13:
  case 14:
    if (rd < (ssize_t)(sizeof(uint8_t))) return 0;
    uint8_t size_mess;
    memcpy(&size_mess, buffer + sizeof(uint16_t), sizeof(uint8_t));
    char *message = calloc(size_mess + 10, sizeof(char));
    int index = (hd_info.code_req == 13) ? 0 : 1;
    memcpy(message, dst[index], dst_len[index]);
    memcpy(message + dst_len[index],
           buffer + sizeof(uint16_t) + sizeof(uint8_t), size_mess*sizeof(char));
    add_message(chat, message);
    break;

  case 15:
  case 16:
    *id_winner = hd_info.id;
    *eq_winner = hd_info.eq;
    // do things
    return 2;
  case 17:
    *alive = 0;
    return 0;

  default:
    return 0;
  }
  return 1;
}

int main(int argc, char *argv[]) {
  if (argc < 4) {
    fprintf(stderr, "Usage: %s <url> <port> <gamemode>\n", argv[0]);
    return 1;
  }

  char *url = argv[1];
  uint16_t port = (uint16_t)atoi(argv[2]);
  int gamemode = atoi(argv[3]);

  if (gamemode != 1 && gamemode != 2) {
    fprintf(stderr, "Invalid gamemode\n");
    fprintf(stderr, "Usage : 1 for Free-For-All 2 for 2vs2\n");
    return 1;
  }

  // game utils
  Board *board = malloc(sizeof(Board));
  setup_board(board);
  Chat *chat = create_chat();

  // connecting to the server by TCP
  int sock_tcp;
  int error_code = connect_to_tcp(url, port, &sock_tcp);
  if (error_code != SUCCESS)
    goto error_message;
  Header_info header_info;
  Server_info serv_info;

  // sending a message to the server to join the game
  join_game_rq(sock_tcp, gamemode);
  // joining the game
  error_code = join_game(sock_tcp, &serv_info, &header_info);
  if (error_code != SUCCESS) {
    close(sock_tcp);
    goto error_message;
  }

  // send a message to the server to say that we are ready to play
  error_code = send_header_rq(
      sock_tcp, header_id_request((gamemode == 1) ? 3 : 4, header_info.id,
                                  header_info.eq));

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 40;

  int num_rq_wr = 0;
  uint16_t num_rq_rd = 0;
  int alive = 1;
  int id_winner = -1;
  int eq_winner = -1;
  setup_view();
  int ret = 0;
  // get the first full grid
  do { 
    ret = update_board_check(serv_info.sock_mdff, &serv_info.addr_mdff, board,
                     &num_rq_rd, 0);
    usleep(40);
  }while(!ret);
  refresh_board(board);
  refresh_line(chat->current_message, chat->prompt,
               board->h + 2 + chat->max_messages);

  while (1) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(serv_info.sock_mdff, &readfds);
    FD_SET(sock_tcp, &readfds);

    int max_fd =
        (serv_info.sock_mdff > sock_tcp) ? serv_info.sock_mdff : sock_tcp;
    int activity = select(max_fd + 1, &readfds, NULL, NULL, &timeout);
    if (activity < 0) {
      error_code = ERROR_SYSTEM;
      goto clean_space;
    }

    if (!activity)
      goto game_control;

    // get full grid or differential grid
    if (FD_ISSET(serv_info.sock_mdff, &readfds)) {
      if (update_board_check(serv_info.sock_mdff, &serv_info.addr_mdff, board,
                             &num_rq_rd, 0))
        refresh_board(board);
    }

    // get chat message
    if (FD_ISSET(sock_tcp, &readfds)) {
      char buffer[BUFFER_SIZE];
      int res = update_chat_check(sock_tcp, buffer, &alive, chat, &id_winner,
                                  &eq_winner);
      if (res == 2) {
        goto clean_space;
      }
      if (res)
        refresh_chat(chat, board->h + 2);
    }

    // get action from player
    ACTION action;
  game_control:
    action = control(chat, gamemode);
    if (action == QUIT) break;

    if (action == NONE) goto sleeep;

    if (action == REFRESH) {
      refresh_line(chat->current_message, chat->prompt,
                   board->h + 2 + chat->max_messages);
      goto sleeep;
    }

    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(serv_info.sock_udp, &writefds);
    FD_SET(sock_tcp, &writefds);

    max_fd = (serv_info.sock_udp > sock_tcp) ? serv_info.sock_udp : sock_tcp;
    activity = select(max_fd + 1, NULL, &writefds, NULL, &timeout);

    if (activity < 0) {
      error_code = ERROR_SYSTEM;
      goto clean_space;
    }

    if (!activity) goto sleeep;

    // send action request to server
    if (FD_ISSET(serv_info.sock_udp, &writefds) && action != SEND && alive) {
      // send action request to server
      uint32_t action_id =
          action_request(5, header_info.id, header_info.eq, num_rq_wr, action);
      char *action_req = malloc(sizeof(uint32_t));
      memcpy(action_req, &action_id, sizeof(uint32_t));
      send_meessage(serv_info.sock_udp, &(serv_info.addr_udp), action_req,
                    sizeof(uint32_t), 1);
      num_rq_wr = (num_rq_wr + 1) % 0x1fff;
      free(action_req);
      continue;
    }

    // send chat message to server
    if (FD_ISSET(sock_tcp, &writefds)) {
      // send chat message to server
      int destination = (chat->destination == 0) ? 7 : 8;
      char *chat_req = tchat_request(
          destination, header_info.id, header_info.eq,
          chat->current_message->cursor, chat->current_message->data);
      size_t total_bytes = sizeof(uint8_t) + sizeof(uint16_t) + chat->current_message->cursor*sizeof(char);
      ssize_t wr = send_meessage(sock_tcp, NULL, chat_req, total_bytes, 0);
      if (wr < 0) goto clean_space;
      clear_line(chat->current_message);
      refresh_line(chat->current_message, chat->prompt,
                   board->h + 2 + chat->max_messages);
      refresh_line(chat->current_message, chat->prompt,
                   board->h + 2 + chat->max_messages);
      free(chat_req);
    }
  sleeep:
    usleep(40);
  }


clean_space:
  printf("winner is %d from team %d\n", id_winner, eq_winner);
  printf("Game Over\n");
  free_board(board);
  free_chat(chat);
  close(sock_tcp);
  close(serv_info.sock_mdff);
  close(serv_info.sock_udp);
  curs_set(1);
  endwin();
error_message:
  fprintf(stderr, "Error: %s\n", ERROR_MESSAGES[error_code]);
  return error_code;
}