#include "../include/format.h"

// Encode the header of the request
u_int16_t header_id_request(int code_req, int id, int eq) {
  u_int16_t res = 0;
  res = (u_int16_t)eq << 15 | id << 13 | code_req;
  return htons(res);
}

// Decode the header of the request
void header_id_decode(uint16_t req, int *code_req, int *id, int *eq) {
  *eq = req >> 15 & 0x1;
  *id = req >> 13 & 0x3;
  *code_req = req & 0x1FFF;
}

// Player send action to the server
uint32_t action_request(int code_req, int id, int eq, int num, int action) {
  uint16_t header_id = header_id_request(code_req, id, eq);
  uint16_t action_id = htons((uint16_t)action << 13 | num);
  return header_id | action_id << 16;
}

void action_id_decode(uint16_t req, int *num, int *action) {
  *num = req & 0x1FFF;
  *action = req >> 13 & 0x7;
}

// Player send chat message to the server
char *tchat_request(int code_req, int id, int eq, size_t size, char *msg) {
  size_t index = 0;
  uint16_t header_id = header_id_request(code_req, id, eq);
  uint8_t size_rq = (uint8_t)size;
  char *res = malloc(size*sizeof(char) + sizeof(uint16_t) + sizeof(uint8_t));
  memcpy(res, &header_id, sizeof(uint16_t));
  index += sizeof(uint16_t);
  memcpy(res + index, &size_rq, sizeof(uint8_t));
  index += sizeof(uint8_t);
  memcpy(res + index, msg, size*sizeof(char));
  return res;
}

// At the beginning of the game the serv send this message to the every player
char *serv_start_request(int code_req, int id, int eq, uint16_t port_udp,
                         uint16_t port_mdiff, char *adrm_diff) {
  uint16_t header_id = header_id_request(code_req, id, eq);
  uint16_t port_udp_ = htons(port_udp);
  uint16_t port_mdiff_ = htons(port_mdiff);

  uint32_t adrm_diff_[4] = {0};
  memcpy(adrm_diff_, adrm_diff, sizeof(uint32_t) * 4);
  for (int i = 0; i < 4; i++) {
    adrm_diff_[i] = htonl(adrm_diff_[i]);
  }

  char *res = malloc(sizeof(uint16_t) * 3 + 4 * sizeof(uint32_t));
  memcpy(res, &header_id, sizeof(uint16_t));
  memcpy(res + sizeof(uint16_t), &port_udp_, sizeof(uint16_t));
  memcpy(res + sizeof(uint16_t) * 2, &port_mdiff_, sizeof(uint16_t));
  memcpy(res + sizeof(uint16_t) * 3, adrm_diff_, 4 * sizeof(uint32_t));
  return res;
}

// Send the all board to the player
char *all_board_request(int code_req, int id, int eq, int num, Board *board) {
  uint16_t header_id = header_id_request(code_req, id, eq);
  uint16_t num_ = htons((uint16_t)num);
  uint8_t height = (uint8_t)board->h;
  uint8_t width = (uint8_t)board->w;
  size_t wr = 0;

  char *res = malloc(sizeof(uint16_t) * 2 +
                     sizeof(uint8_t) * ((size_t)(board->h * board->w + 2)));
  memcpy(res, &header_id, sizeof(uint16_t));
  wr += sizeof(uint16_t);
  memcpy(res + wr, &num_, sizeof(uint16_t));
  wr += sizeof(uint16_t);
  memcpy(res + wr, &height, sizeof(uint8_t));
  wr += sizeof(uint8_t);
  memcpy(res + wr, &width, sizeof(uint8_t));
  wr += sizeof(uint8_t);
  for (int i = 0; i < board->h * board->w; i++) {
    uint8_t cell = 0;
    switch (board->grid[i]) {
    case PATH:
      cell = 0;
      break;
    case WALL:
      cell = 1;
      break;
    case D_WALL:
      cell = 2;
      break;
    case 'A':
      cell = 3;
      break;
    case 'B':
      cell = 4;
      break;
    case 'C':
      cell = 5;
      break;
    case 'D':
      cell = 6;
      break;
    case 0:
      cell = 7;
      break;
    case 1:
      cell = 8;
      break;
    case 2:
      cell = 9;
      break;
    case 3:
      cell = 10;
      break;
    }
    memcpy(res + wr, &cell, sizeof(uint8_t));
    wr += sizeof(uint8_t);
  }
  return res;
}

// send the board with the changed cells
char *change_board_request(int code_req, int id, int eq, int num,
                           Board *board) {
  uint16_t header_id = header_id_request(code_req, id, eq);
  uint16_t num_ = htons((uint16_t)num);
  uint8_t nb = (uint8_t)board->changed_cells_count;

  char *res =
      malloc(2 * sizeof(uint16_t) +
             sizeof(uint8_t) * ((size_t)(board->changed_cells_count * 3 + 1)));
  size_t wr = 0;
  memcpy(res + wr, &header_id, sizeof(uint16_t));
  wr += sizeof(uint16_t);
  memcpy(res + wr, &num_, sizeof(uint16_t));
  wr += sizeof(uint16_t);
  memcpy(res + wr, &nb, sizeof(uint8_t));
  wr += sizeof(uint8_t);
  memcpy(res + wr, board->changed_cells, board->changed_cells_size);
  return res;
}

// function to read the message from the socket
ssize_t read_message(int sockfd, struct sockaddr_in6 *addr, char *buffer,
                     size_t size_mess, int protocol) {
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 40;
  ssize_t total_bytes = 0;
  ssize_t rd = 0;

  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                 sizeof(timeout)) < 0)
    return -1;

  if (protocol) {
    socklen_t addr_len = sizeof(struct sockaddr_in6);
    rd = recvfrom(sockfd, buffer, size_mess, 0, (struct sockaddr *)addr,
                  &addr_len);
    total_bytes = rd;
    while (rd > 0 && total_bytes < (ssize_t)size_mess) {
      rd = recvfrom(sockfd, buffer + total_bytes,
                    size_mess - (size_t)total_bytes, 0, (struct sockaddr *)addr,
                    &addr_len);
      total_bytes += rd;
    }
  } else {
    rd = recv(sockfd, buffer, size_mess, 0);
    total_bytes = rd;
    while (rd > 0 && total_bytes < (ssize_t)size_mess) {
      rd = recv(sockfd, buffer + total_bytes, size_mess - (size_t)total_bytes,
                0);
      total_bytes += rd;
    }
  }

  timeout.tv_usec = 0;
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) <
      0) {
    return -1;
  }
  if (rd < 0)
    return total_bytes - rd;
  return total_bytes;
}

// function to send the message to the socket
ssize_t send_meessage(int sockfd, struct sockaddr_in6 *addr, char *buffer,
                      size_t size_mess, int protocol) {
  struct timeval timeout = {.tv_sec = 0, .tv_usec = 40};
  ssize_t bytes_sent = -1;
  ssize_t wr = 0;
  memset(buffer + size_mess, 0, size_mess - size_mess);

  if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                 sizeof(timeout)) < 0)
    return -1;

  if (protocol) {
    wr = sendto(sockfd, buffer, size_mess, 0, (struct sockaddr *)addr,
                sizeof(*addr));
    bytes_sent = wr;
    while (wr > 0 && bytes_sent < (ssize_t)size_mess) {
      wr = sendto(sockfd, buffer + bytes_sent, size_mess - (size_t)bytes_sent,
                  0, (struct sockaddr *)addr, sizeof(*addr));
      bytes_sent += wr;
    }
  } else {
    wr = send(sockfd, buffer, size_mess, 0);
    bytes_sent = wr;
    while (wr > 0 && bytes_sent < (ssize_t)size_mess) {
      wr = send(sockfd, buffer + bytes_sent, size_mess - (size_t)bytes_sent, 0);
      bytes_sent += wr;
    }
  }
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                 sizeof(timeout)) < 0)
    return -1;

  if (wr < 0)
    return bytes_sent - wr;
  return bytes_sent;
}
