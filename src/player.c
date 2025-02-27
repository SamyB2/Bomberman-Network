#include "../include/player.h"

void init_player(Player *p, int id, int eq, Pos *pos) {
  p->id = id;
  p->eq = eq;
  p->pos = pos;
  p->alive = 1;
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

// Drop a bomb on the player's position
void drop_bomb(Player *player, Board *board, Deque *bombs, Bomb *bomb) {
  if (get_grid(board, player->pos->x, player->pos->y) != player->id)
    return;
  push_back(bombs, bomb);
  set_grid(board, bomb->pos->x, bomb->pos->y, (char)('A' + player->id));
}

int *explode_bomb(Bomb *bomb, Board *board, Player *Players) {
  int nbPlayers = 4;
  int *playersHit =
      calloc((size_t)nbPlayers, sizeof(int)); // Initialize all elements to 0

  // Check each direction
  for (int i = 0; i < 8; i++) {
    Direction *direction = dir_of(i);

    // Check each point in the bomb's range
    for (int k = 0; k <= bomb->range; k++) {
      int hitPointX = bomb->pos->x + direction->coefX * k;
      int hitPointY = bomb->pos->y + direction->coefY * k;

      // If the point is out of bounds, skip to the next point
      if (!is_in_bounds(board, hitPointX, hitPointY))
        continue;

      int gridValue = get_grid(board, hitPointX, hitPointY);
      if (gridValue == D_WALL) {
        set_grid(board, hitPointX, hitPointY, PATH);
      } else if (gridValue >= 0 && gridValue < nbPlayers) {
        playersHit[gridValue] = 1;
      }
    }
  }

  int bombDropper = get_grid(board, bomb->pos->x, bomb->pos->y) - 'A';
  if (bombDropper >= 0 && bombDropper < nbPlayers) {
      int bombXX = bomb->pos->x;
      int bombYY = bomb->pos->y;
      int playerXX = (Players+bombDropper)->pos->x;
      int playerYY = (Players+bombDropper)->pos->y;
      if (bombXX == playerXX && bombYY == playerYY)  playersHit[bombDropper] = 1;
  }
  set_grid(board, bomb->pos->x, bomb->pos->y, PATH);
  return playersHit;
}

// Check if the bomb is in range of the other bomb
void bombs_in_range(Bomb *bomb, Deque *bombs, Deque *bombsInRange) {
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

// Reduce the timer of each bomb and explode the bombs that have a timer <= 0
int *reduce_timer(Deque *bombs, Board *board, Player *Players, int dt) {
  for (Node *node = bombs->start; node != NULL; node = node->next) {
    Bomb *bomb = node->data;
    bomb->timer -= dt;
  }

  Deque *bombsToExplode = init_deque();

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
  free_deque(bombsToExplode, free_bomb);
  return playersHit;
}
