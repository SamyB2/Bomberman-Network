#include "../include/server.h"

Deque *ffa, *team;

// Mutexes for game modes
pthread_mutex_t ffa_count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t team_count_mutex = PTHREAD_MUTEX_INITIALIZER;

// Mutexes for queues
pthread_mutex_t ffa_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t team_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t port_mutex = PTHREAD_MUTEX_INITIALIZER;

// Counters for game modes
int ffa_count = 0;
int team_count = 0;

uint16_t udp_var = 5000;

int check_connection_to_client(ClientInfo *client[]) {
  char buffer[1];
  int count = 0;
  for (int i = 0; i < MAX_PLAYERS; i++) {
    ssize_t result = recv(client[i]->socket, buffer, sizeof(buffer),
                          MSG_PEEK | MSG_DONTWAIT);
    if (result == 0) {
      close(client[i]->socket);
      client[i] = NULL;
    } else {
      count++;
    }
  }
  return count;
}

// Process the queue of clients and create the structure for the different client
void process_queue(Deque *queue, pthread_mutex_t *queue_mutex,
                   pthread_mutex_t *count_mutex, int *count, int gamemode) {
  pthread_mutex_lock(queue_mutex);
  pthread_mutex_lock(count_mutex);

  if (*count < MAX_PLAYERS) {
    pthread_mutex_unlock(count_mutex);
    pthread_mutex_unlock(queue_mutex);
    return;
  }

  ClientInfo *client[MAX_PLAYERS];
  populate_clients(queue, client);

  int check = check_connection_to_client(client);

  if (check < MAX_PLAYERS) {
    push_clients_back_to_queue(queue, client);
  } else {
    process_clients(client, gamemode);
    *count -= 4;
  }

  pthread_mutex_unlock(count_mutex);
  pthread_mutex_unlock(queue_mutex);
}

// function to populate the client array
void populate_clients(Deque *queue, ClientInfo *client[]) {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    Node *nd = pop_front(queue);
    client[i] = (ClientInfo *)nd->data;
    free(nd);
  }
}

void push_clients_back_to_queue(Deque *queue, ClientInfo *client[]) {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (client[i] != NULL) {
      push_front(queue, client[i]);
    }
  }
}

void *check_queues(void *arg) {
  (void)arg;
  while (1) {
    process_queue(ffa, &ffa_queue_mutex, &ffa_count_mutex, &ffa_count, 1);
    process_queue(team, &team_queue_mutex, &team_count_mutex, &team_count, 2);
    sleep(5);
  }

  pthread_exit(NULL);
}

ClientInfo *create_client_info(int client_socket) {
  // Allocate and initialize client info
  ClientInfo *client_info = malloc(sizeof(ClientInfo));
  if (!client_info) {
    perror("Failed to allocate memory for client info");
    close(client_socket);
    pthread_exit(NULL);
  }
  client_info->socket = client_socket;

  return client_info;
}

// function qui ajoute un client à la queue
void add_client_to_queue(int client_socket, int gamemode) {
  pthread_mutex_t *count_mutex, *queue_mutex;
  Deque *queue;
  int *count;

  // Create client info
  ClientInfo *client_info = create_client_info(client_socket);
  client_info->socket = client_socket;

  if (gamemode == 1) {
    queue_mutex = &ffa_queue_mutex;
    count_mutex = &ffa_count_mutex;
    count = &ffa_count;
    queue = ffa;
  } else {
    queue_mutex = &team_queue_mutex;
    count_mutex = &team_count_mutex;
    count = &team_count;
    queue = team;
  }

  pthread_mutex_lock(queue_mutex);
  pthread_mutex_lock(count_mutex);

  push_back(queue, client_info);
  (*count)++;

  pthread_mutex_unlock(count_mutex);
  pthread_mutex_unlock(queue_mutex);
  printf("Client connected.\n");
}

// function who read the gamemode of the client
int read_client_gamemode(int socket) {
  char buff[BUFFER_SIZE];
  uint16_t header_id;
  Header_info header_info;

  ssize_t rd = read_message(socket, NULL, buff, sizeof(uint16_t), 0);

  if (rd < 0) {
    perror("read failed");
    close(socket);
    return -1;
  }
  memcpy(&header_id, buff, sizeof(uint16_t));
  header_id = ntohs(header_id);
  header_id_decode(header_id, &header_info.code_req, &header_info.id,
                   &header_info.eq);
  return header_info.code_req;
}

void *init_client(void *arg) {
  int socket = *(int *)arg;

  // Read client gamemode
  int rcv_gamemode = read_client_gamemode(socket);
  if (rcv_gamemode < 0) {
    pthread_exit(NULL);
  }

  add_client_to_queue(socket, rcv_gamemode);

  pthread_exit(NULL);
}

void send_message_to_all(int sock_tcp[], char* rqst, size_t rqst_len) {
  ssize_t wr = 0;
  for (int i = 0; i < 4; i++) {
    wr = send_meessage(sock_tcp[i], NULL, rqst, rqst_len, 0);
    if (!wr) continue;
  }
}

// function who se board to the client
void send_board(Server_info *server_info, Board *board, int turn, int diff_turn,
                uint16_t num_request_wr) {
  ssize_t wr = 0;
  char *rqst;
  size_t rqst_len = 0;
  if (turn % diff_turn == 0) {
    rqst = all_board_request(11, 0, 0, num_request_wr, board);
    rqst_len = sizeof(uint16_t) * 2 +
               sizeof(uint8_t) * ((size_t)(board->h * board->w + 2));
    wr = send_meessage(server_info->sock_mdff, &server_info->addr_mdff, rqst,
                       rqst_len, 1);
    if (wr < 0) {
      perror("write failed");
      close(server_info->sock_mdff);
    }
    free(rqst);
  } else {
    rqst = change_board_request(12, 0, 0, num_request_wr, board);
    rqst_len = 2 * sizeof(uint16_t) +
               sizeof(uint8_t) * ((size_t)(board->changed_cells_count * 3 + 1));
    wr = send_meessage(server_info->sock_mdff, &server_info->addr_mdff, rqst,
                       rqst_len, 1);
    if (wr < 0) {
      perror("write failed");
      close(server_info->sock_mdff);
    }
    free(rqst);
  }
}

// function to get all actions from the buffer
size_t get_all_actions(ssize_t rd, char *buff, Deque **players_actions) {
  size_t index = 0;
  while (rd >= (ssize_t)(sizeof(uint32_t))) {
    uint16_t header_id;
    uint16_t *action = malloc(sizeof(uint16_t));
    memcpy(&header_id, buff + index, sizeof(uint16_t));
    index += sizeof(uint16_t);
    memcpy(action, buff + index, sizeof(uint16_t));
    index += sizeof(uint16_t);
    header_id = ntohs(header_id);
    *action = ntohs(*action);
    Header_info header_info;
    header_id_decode(header_id, &header_info.code_req, &header_info.id,
                     &header_info.eq);
    push_front(players_actions[header_info.id], action);
    rd -= (ssize_t)sizeof(uint32_t);
  }
  return index;
}

// function to perform all actions
void perform_actions(Deque **players_actions, Player *players, Board *board,
                     Deque *bombs) {
  for (int i = 0; i < 4; i++) {
    Node *node;
    uint16_t *last_action ;
    int max_num = -1;
    int todo = -1;
    while (players_actions[i]->start) {
      node = pop_front(players_actions[i]);
      last_action = (uint16_t *)node->data;
      int num;
      int act;
      action_id_decode(*last_action, &num, &act);
      free(last_action);
      if (num > max_num) {
        max_num = num;
        todo = act;
      }
      free(node);
    }
    switch (todo) {
    case -1: // no action
      break;
    case 4:
      drop_bomb(players + i, board, bombs,
                init_bomb(copy_pos(players[i].pos), 1, 100));
      break;

    default:
      move_player(players + i, todo, board);
      break;
    }
  }
}

// function check the state of the players
int check_players_state(Player *players, Deque *bombs, Board *board,
                        Server_info *server_info, int *nb_alive, int *id_winner,
                        int *eq_winner, int game_mode) {
  char *rqst;
  int *players_hit = reduce_timer(bombs, board, players, 20);
  if (!players_hit) return 0;
  for (int i = 0; i < 4; i++) {
    if (players_hit[i]) {
      uint16_t header_id = header_id_request(17, 0, 0);
      header_id = htons(header_id);
      rqst = malloc(sizeof(uint16_t));
      memcpy(rqst, &header_id, sizeof(uint16_t));
      send_meessage(server_info->sock_tcp[i], NULL, rqst, sizeof(uint16_t), 0);
      free(rqst);
      players[i].alive = 0;
      *nb_alive -= 1;
    }
  }

  if (*nb_alive == 1 || *nb_alive == 2) {
    int players_id[2];
    int index = 0;
    for (int i = 0; i < 4; i++) {
      if (players[i].alive) {
        players_id[index] = i;
        index++;
      }
    }
    if (*nb_alive == 1) {
      *id_winner = players_id[0];
      *eq_winner = players[*id_winner].eq;
    } else {
      if (players[players_id[0]].eq == players[players_id[1]].eq &&
          game_mode == 2) {
        *id_winner = players_id[0];
        *eq_winner = players[players_id[0]].eq;
      } else
        return 0;
    }
    uint16_t header_id = header_id_request(*nb_alive + 14, *id_winner, *eq_winner);
    rqst = malloc(sizeof(uint16_t));
    memcpy(rqst, &header_id, sizeof(uint16_t));
    send_message_to_all(server_info->sock_tcp, rqst, sizeof(uint16_t));
    free(rqst);
    return 1;
  } else return 0;

}

// function to check the chat update
void check_chat_update(Server_info *server_info, Player *players, fd_set* readfds) {
  ssize_t flow;
  char* rqst = NULL;
  for (int i = 0; i < 4; i++) {
    char buff[BUFFER_SIZE];
    if (FD_ISSET(server_info->sock_tcp[i], readfds)) {
      // recevoir les messages du joueurs
      flow = read_message(server_info->sock_tcp[i], NULL, buff,
                        BUFFER_SIZE, 0);
      if (flow <= 0) continue;
      
      uint16_t header_id;
      Header_info header_info;
      uint8_t len;
      size_t rqst_len ;
      // get header
      memcpy(&header_id, buff, sizeof(uint16_t));
      header_id = ntohs(header_id);
      header_id_decode(header_id, &header_info.code_req, &header_info.id, &header_info.eq);
      // get len
      memcpy(&len, buff + sizeof(uint16_t), sizeof(uint8_t));
      char *message = malloc(sizeof(char) * len);
      memcpy(message, buff + sizeof(uint16_t) + sizeof(uint8_t),
             sizeof(char) * len);
      rqst_len = sizeof(uint16_t) + sizeof(uint8_t) + sizeof(char) * len;
      if (header_info.code_req == 7) {
        rqst = tchat_request(13, header_info.id, header_info.eq, len, message);
        send_message_to_all(server_info->sock_tcp, rqst, rqst_len);        
      } else {
        // regarder quel est le destinataire et lui envoyer
        for (int k = 0; k < 4; k++) {
          // regarder quel est le destinataire et lui envoyer
          if (players[k].eq == header_info.eq) {
            rqst = tchat_request(14, header_info.id, header_info.eq, len, message);
            flow = send_meessage(server_info->sock_tcp[k], NULL, rqst, rqst_len, 0);
          }
        }
      }
      free(rqst);
      free(message);
    }
  }
}

// Function to send multicast information to the clients
void process_clients(ClientInfo *client[], int gamemode) {
  Server_info *server_info = malloc(sizeof(Server_info));
  if (!server_info) {
    perror("Failed to allocate memory for server info");
    return;
  }
  pthread_mutex_lock(&port_mutex);
  uint16_t port_mdff = udp_var++;
  server_info->port_mdff = port_mdff;
  uint16_t port_udp = udp_var++;
  server_info->port_udp = port_udp;
  pthread_mutex_unlock(&port_mutex);

  for (int i = 0; i < 4; i++) {
    int equipe = (gamemode == 1) ? 0 : (i % 2);
    int code_req = (gamemode == 1) ? 9 : 10;
    server_info->sock_tcp[i] = client[i]->socket;

    send_multicast_info(client[i]->socket, code_req, i, equipe, port_udp,
                        port_mdff);
    free(client[i]);
  }
  pthread_t game_thread;
  pthread_create(&game_thread, NULL, init_game, server_info);
  pthread_detach(game_thread);
}

// function to initialize the game with board and players
void *init_game(void *arg) {
  Server_info *server_info = (Server_info *)arg;
  Player players[4];
  Board board;
  Deque *bombs = init_deque();
  char buff[BUFFER_SIZE];
  setup_board(&board);
  Pos pos[4] = {
      {0, 0}, {board.w - 1, 0}, {0, board.h - 1}, {board.w - 1, board.h - 1}};

  // recep des header
  uint16_t header_id;
  for (int i = 0; i < 4; i++) {
    ssize_t rd = recv(server_info->sock_tcp[i], buff, sizeof(uint16_t), 0);
    if (rd < 0) {
      perror("read failed");
      close(server_info->sock_tcp[i]);
    }
    memcpy(&header_id, buff, sizeof(uint16_t));
    header_id = ntohs(header_id);
    int id, code_req, eq;
    header_id_decode(header_id, &code_req, &id, &eq);
    init_player(players + i, id, eq, pos + i);
  }

  start_udp_servers(server_info->port_udp, server_info->port_mdff, server_info);
  gen_map(&board);
  for (int i = 0; i < 4; i++) {
    set_grid_cell(&board, players[i].pos->x, players[i].pos->y, (char)i);
  }
  game(server_info, &board, players, bombs);
  free_deque(bombs, free_bomb);
  for (int i = 0; i < 4; i++) {
    close(server_info->sock_tcp[i]);
  }
  close(server_info->sock_mdff);
  close(server_info->sock_udp);
  free(server_info);
  return NULL;
}

// game loop
void game(Server_info *server_info, Board *board, Player *players,
          Deque *bombs) {
  // recevoir 4 messages TCP des 4 joueurs pour qu'ils disent qu'ils sont prets
  int game_mode =
      players[0].eq || players[1].eq || players[2].eq || players[3].eq;
  game_mode = game_mode ? 2 : 1;
  int nb_alive = 4;
  int turn = 0;
  int diff_turn = 1000000 / 40;
  int activity = 0;
  int id_winner = -1;
  int eq_winner = -1;
  uint16_t num_request_wr = 0;
  Deque **players_actions = malloc(sizeof(Deque *) * 4);
  for (int i = 0; i < 4; i++) {
    players_actions[i] = init_deque();
  }
  printf("A game has started.\n");
  while (1) {
    // envoie de la grille à tous les joueurs
    send_board(server_info, board, turn, diff_turn, num_request_wr);
    turn++;
    num_request_wr = (uint16_t)(num_request_wr + 1) % 65536;

    // reception des requetes
    fd_set readfds;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 40;

    FD_ZERO(&readfds);
    FD_SET(server_info->sock_udp, &readfds);
    for (int i = 0; i < 4; i++) {
      FD_SET(server_info->sock_tcp[i], &readfds);
    }

    activity = select(FD_SETSIZE, &readfds, NULL, NULL, &tv);

    if (!activity) {
      usleep(40);
      continue;
    }

    // read all received actions
    if (FD_ISSET(server_info->sock_udp, &readfds)) {
      // lire les messages UDP
      char buff[2 * BUFFER_SIZE];
      ssize_t rd;
      struct sockaddr_in6 addr_client;
      rd = read_message(server_info->sock_udp, &addr_client, buff, BUFFER_SIZE,
                        0);

      // get all actions
      size_t index = get_all_actions(rd, buff, players_actions);
      if (!index)
        goto chat_update;

      // execute all actions
      perform_actions(players_actions, players, board, bombs);

      // check players state
      if (check_players_state(players, bombs, board, server_info, &nb_alive,
                              &id_winner, &eq_winner, game_mode)) break;
    }

  // chat update
  chat_update:
    check_chat_update(server_info, players, &readfds);
    usleep(30);
  }

  // free all players actions
  for (int i = 0; i < 4; i++) {
    free_deque(players_actions[i], free);
  }
  free(players_actions);
}

int main() {
  pthread_t check_game_modes_thread;
  int server_socket;
  socklen_t address_length = sizeof(struct sockaddr_storage);

  ffa = init_deque();
  team = init_deque();

  // Create server and initialize connection queue
  __uint16_t port = 8080;
  server_socket = create_server(port);
  if (server_socket < 0) {
    perror("Failed to create server");
    return EXIT_FAILURE;
  }

  printf("Server started, ready to receive connection on port %d\n", port);

  if (pthread_create(&check_game_modes_thread, NULL, check_queues, NULL) != 0) {
    perror("Failed to create check thread");
    return EXIT_FAILURE;
  }
  // Accept and handle client connections
  while (1) {
    int client_socket = accept(server_socket, NULL, &address_length);
    if (client_socket < 0) {
      perror("Failed to accept connection");
      continue;
    }

    pthread_t client_init_thread;
    // Create a thread to handle the client connection
    if (pthread_create(&client_init_thread, NULL, init_client,
                       &client_socket) != 0) {
      perror("Failed to create thread");
      continue;
    }
    pthread_detach(client_init_thread);
  }

  // Close server socket
  close(server_socket);

  return 0;
}