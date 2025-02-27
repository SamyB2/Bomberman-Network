#include "bomb.h"
#include "deque.h"
#include "format.h"
#include "player.h"
#include "tcp.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define MAX_PLAYERS 4
#define MAX_CLIENTS 1024
#define TIMEOUT 5000

typedef struct {
  int socket;
  struct sockaddr_in6 addr;
} ClientInfo;

int check_connection_to_client(ClientInfo *client[]);
void process_queue(Deque *queue, pthread_mutex_t *queue_mutex,
                   pthread_mutex_t *count_mutex, int *count, int gamemode);
void *check_queues(void *arg);
void populate_clients(Deque *queue, ClientInfo *client[]);
void process_clients(ClientInfo *client[], int gamemode);
void push_clients_back_to_queue(Deque *queue, ClientInfo *client[]);
void *init_game(void *arg);
void game(Server_info *server_info, Board *board, Player *players,
          Deque *bombs);
ClientInfo *create_client_info(int client_socket);
void add_client_to_queue(int client_socket, int gamemode);
int read_client_gamemode(int socket);
void *init_client(void *arg);
void send_message_to_all(int sock_tcp[], char* rqst, size_t rqst_len);
void send_board(Server_info *server_info, Board *board, int turn, int diff_turn,
                uint16_t num_request_wr);
size_t get_all_actions(ssize_t rd, char *buff, Deque **players_actions);
void perform_actions(Deque **players_actions, Player *players, Board *board,
                     Deque *bombs);
int check_players_state(Player *players, Deque *bombs, Board *board,
                        Server_info *server_info, int *nb_alive, int *id_winner,
                        int *eq_winner, int game_mode);
void check_chat_update(Server_info *server_info, Player *players,
                       fd_set *readfds);
