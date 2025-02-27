#ifndef BOMB_H
#define BOMB_H

#include "pos.h"
#include "stdlib.h"

typedef struct {
    Pos* pos;
    int range;
    int timer;
} Bomb;

Bomb* init_bomb(Pos* pos , int range , int timer);
void free_bomb(void* bomb);
int in_range(Pos* pos , Pos* bombPos , int range);

#endif
