#ifndef PLAYER_H
#define PLAYER_H

#include "pos.h"
#include "board.h"
#include "bomb.h"
#include "dequeue.h"
#include "direction.h"

typedef struct {
    int id;
    int id_game;
    Pos* pos;
    int sockTCP;
} Player;

Player* init_player(int id , int id_game , Pos* pos , int sockTCP);
void free_player(Player* player);
void move_player (Player* player , int dir , Board* board);
void drop_bomb(Player* player , Board* board , Dequeue* bombs , Bomb* bomb);
int* explode_bomb (Bomb* bomb , Board* board , Player* Players[]);
int* reduce_timer(Dequeue* bombs , Board* board , Player* Players[] , int dt);
void bombs_in_range(Bomb* bomb , Dequeue* bombs , Dequeue* bombsInRange);

#endif