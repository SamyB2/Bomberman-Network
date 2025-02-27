#include "format.h"
#include "board.h"
#include "chat.h"
#include "view.h"

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>

typedef struct header_info {
  int code_req;
  int id;
  int eq;
} Header_info;

typedef struct server_info {
  int sock_mdff;
  struct sockaddr_in6 addr_mdff;
  int sock_udp;
  struct sockaddr_in6 addr_udp;
} Server_info;

// Error codes
enum {
  ERROR_NONE,
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
int connect_to_tcp(const char *server_url, uint16_t server_port, int* socket_fd);
int join_game_rq(int sockfd, int gamemode);
int join_multicast_group(const char *multicast_ip, uint16_t multicast_port,
                                     int* sockfd_, struct sockaddr_in6* addr_);
int get_game_info(int sockfd, int* id, int* eq, uint16_t* port_udp,
                       uint16_t* port_mdiff, char* adrm_diff);
int send_header_rq (int sockfd, uint16_t header_id);
int create_socket_udp(uint16_t portudp, int* sock_udp, struct sockaddr_in6* addr_udp_);
int join_game(int sock_tcp, Server_info* serv_info, Header_info* header_info);
int read_message(int sock_udp, struct sockaddr_in6* addr, char* buffer, size_t size, size_t size_mess)
ssize_t send_meessage(int sockfd, struct sockaddr_in6* addr, char* buffer,
                                  size_t size_mess, int protocol);
void byte_to_board (char* cells, Board* board);
void extract_changed_cells(char* buffer, Board* board);
