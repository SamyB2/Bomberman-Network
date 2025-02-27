#ifndef PLAYER_H
#define PLAYER_H

#include "pos.h"
#include "board.h"
#include "bomb.h"
#include "deque.h"
#include "direction.h"

typedef struct {
    int id;
    int eq;
    Pos* pos;
    int alive;
} Player;

void init_player(Player *p ,int id, int eq, Pos* pos);
void free_player(Player* player);
void move_player (Player* player , int dir , Board* board);
void drop_bomb(Player* player , Board* board , Deque* bombs , Bomb* bomb);
int* explode_bomb (Bomb* bomb , Board* board , Player* Players);
int* reduce_timer(Deque* bombs , Board* board , Player* Players , int dt);
void bombs_in_range(Bomb* bomb , Deque* bombs , Deque* bombsInRange);

#endif