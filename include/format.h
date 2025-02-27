#ifndef FORMAT_H
#define FORMAT_H

#include "board.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct header_info {
  int code_req;
  int id;
  int eq;
} Header_info;

typedef struct server_info {
  int sock_mdff;
  uint16_t port_mdff;
  struct sockaddr_in6 addr_mdff;
  int sock_udp;
  uint16_t port_udp;
  struct sockaddr_in6 addr_udp;
  int sock_tcp[4];
} Server_info;

u_int16_t header_id_request(int code_req, int id, int eq);
void header_id_decode(uint16_t req, int *code_Req, int *id, int *eq);
uint32_t action_request(int code_req, int id, int eq, int num, int action);
void action_id_decode(uint16_t req, int *num, int *action);
char *tchat_request(int code_req, int id, int eq, size_t size, char *msg);
char *serv_start_request(int code_req, int id, int eq, uint16_t port_udp,
                         uint16_t port_mdiff, char *adrm_diff);
char *all_board_request(int code_req, int id, int eq, int num, Board *board);
char *change_board_request(int code_req, int id, int eq, int num, Board *board);
ssize_t read_message(int sockfd, struct sockaddr_in6 *addr, char *buffer,
                     size_t size_mess, int protocol);
ssize_t send_meessage(int sockfd, struct sockaddr_in6 *addr, char *buffer,
                      size_t size_mess, int protocol);

#endif