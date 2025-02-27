#ifndef POS_H
#define POS_H

#include <stdlib.h>

typedef struct {
    int x;
    int y;
} Pos;

Pos* init_pos(int x , int y);
Pos* copy_pos(Pos* pos);

#endif