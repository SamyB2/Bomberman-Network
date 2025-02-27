#include "board.h"
#include "chat.h"
#include "format.h"
#include "view.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// Error codes
enum {
  SUCCESS,
  ERROR_SOCKET_CREATION,
  ERROR_INVALID_ADDRESS,
  ERROR_CONNECTION_FAILED,
  ERROR_SENDING_MESSAGE,
  ERROR_READING_RESPONSE,
  ERROR_INCOMPLETE_RESPONSE,
  ERROR_SOCKET_OPTIONS,
  ERROR_SYSTEM
};

// Error messages
const char *ERROR_MESSAGES[] = {"No error",
                                "Socket creation failed",
                                "Invalid address/ Address not supported",
                                "Connection failed",
                                "Error sending message",
                                "Error reading response",
                                "Incomplete response",
                                "Error setting socket options",
                                "System error"};

#define BUFFER_SIZE 1024
#define BUFFER_SIZE_UDP 2048
#define MAX_ATTEMPTS 5
int connect_to_tcp(const char *server_url, uint16_t server_port,
                   int *socket_fd);
int join_game_rq(int sockfd, int gamemode);
int join_multicast_group(const char *multicast_ip, uint16_t multicast_port,
                         int *sockfd_, struct sockaddr_in6 *addr_);
int get_game_info(int sockfd, int *id, int *eq, uint16_t *port_udp,
                  uint16_t *port_mdiff, char *adrm_diff);
int send_header_rq(int sockfd, uint16_t header_id);
int create_socket_udp(uint16_t portudp, int *sock_udp,
                      struct sockaddr_in6 *addr_udp_);
int join_game(int sock_tcp, Server_info *serv_info, Header_info *header_info);
void byte_to_board(char *cells, Board *board);
void extract_changed_cells(char *buffer, Board *board);
int update_chat_check(int sock_tcp, char *buffer, int *alive, Chat *chat,
                      int *id_winner, int *eq_winner);
int update_board_check(int sock_mdff, struct sockaddr_in6 *addr_mdff,
                       Board *board, uint16_t* num_rq_rd, int get_dim);
