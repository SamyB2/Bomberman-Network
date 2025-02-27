#include "../include/player.h"

Player *init_player(int id, int id_game, Pos *pos, int sockTCP) {
  Player *player = malloc(sizeof(Player));
  player->id = id;
  player->id_game = id_game;
  player->pos = pos;
  player->sockTCP = sockTCP;
  return player;
}

void free_player(Player *player) {
  free(player->pos);
  free(player);
}

void move_player(Player *player, int dir, Board *board) {
  Direction *direction = dir_of(dir);
  // new x and y of the player
  int nx = player->pos->x + direction->coefX;
  int ny = player->pos->y + direction->coefY;

  // check if the player can move to the new position
  if (is_in_bounds(board, nx, ny) && get_grid(board, nx, ny) == PATH) {

    // grid update
    set_grid(board, nx, ny, (char)player->id);

    // if the player was on a path, we put a path in his old position
    if (get_grid(board, player->pos->x, player->pos->y) == player->id) {
      set_grid(board, player->pos->x, player->pos->y, PATH);
    }

    // player's position update
    player->pos->x = nx;
    player->pos->y = ny;
  }
}

void drop_bomb(Player *player, Board *board, Dequeue *bombs, Bomb *bomb) {
  if (get_grid(board, player->pos->x, player->pos->y) != player->id)
    return;
  push_back(bombs, bomb);
  set_grid(board, bomb->pos->x, bomb->pos->y, (char)('A' + player->id));
}

int *explode_bomb(Bomb *bomb, Board *board, Player *Players[]) {
  int nbPlayers = 4;
  int *playersHit =
      calloc((size_t)nbPlayers,
             sizeof(int)); // Use calloc to initialize all elements to 0

  // check the 8 directions
  for (int i = 0; i < 8; i++) {
    Direction *direction = dir_of(i);

    // check the range of the bomb
    for (int k = 0; k <= bomb->range; k++) {
      // hitting point
      int hp_x = bomb->pos->x + direction->coefX * k;
      int hp_y = bomb->pos->y + direction->coefY * k;
      
      // if the point is out of bounds, we stop
      if (!is_in_bounds(board, hp_x, hp_y))
        continue;

      int grid_value = get_grid(board, hp_x, hp_y);
      if (grid_value == D_WALL) {
        set_grid(board, hp_x, hp_y, PATH);
      } else if (grid_value >= 0 && grid_value < nbPlayers) {
        playersHit[grid_value] = 1;
      }
    }
    free_dir(direction);
  }

  int bomb_dropper = 'A' - get_grid(board, bomb->pos->x, bomb->pos->y);
  if (bomb->pos->x == Players[bomb_dropper]->pos->x &&
      bomb->pos->y == Players[bomb_dropper]->pos->y) {
    playersHit[bomb_dropper] = 1;
  }
  set_grid(board, bomb->pos->x, bomb->pos->y, PATH);
  return playersHit;
}

void bombs_in_range(Bomb *bomb, Dequeue *bombs, Dequeue *bombsInRange) {
  Node *node = bombs->start;
  while (node) {
    Bomb *bomb2 = node->data;
    if (in_range(bomb2->pos, bomb->pos, bomb->range)) {
      push_back(bombsInRange, bomb2);
      node = node->next;
      remove_el(bombs, bomb2);
      continue;
    }
    node = node->next;
  }
}

int *reduce_timer(Dequeue *bombs, Board *board, Player *Players[], int dt) {
  for (Node *node = bombs->start; node != NULL; node = node->next) {
    Bomb *bomb = node->data;
    bomb->timer -= dt;
  }

  Dequeue *bombsToExplode = init_dequeue();

  Node *node = bombs->start;
  while (node) {
    Bomb *bomb = node->data;
    if (bomb->timer <= 0) {
      push_back(bombsToExplode, bomb);
      node = node->next;
      remove_el(bombs, bomb);
      continue;
    }
    node = node->next;
  }

  int *playersHit = NULL;
  if (!bombsToExplode->start)
    goto exit;

  for (Node *no = bombsToExplode->start; no; no = no->next) {
    bombs_in_range(no->data, bombs, bombsToExplode);
  }

  int nbPlayers = 4;
  int nbHit = 0;
  playersHit = calloc((size_t)nbPlayers, sizeof(int));
  for (Node *no = bombsToExplode->start; no; no = no->next) {
    Bomb *bomb = no->data;
    int *playersHit2 = explode_bomb(bomb, board, Players);
    for (int i = 0; i < nbPlayers; i++) {
      playersHit[i] = playersHit[i] || playersHit2[i];
      nbHit |= playersHit2[i];
    }
    free(playersHit2);
  }

  if (!nbHit) {
    free(playersHit);
    playersHit = NULL;
  }
exit:
  free_dequeue(bombsToExplode, free_bomb);
  return playersHit;
}
