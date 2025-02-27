#ifndef DIRECTION_H
#define DIRECTION_H

#include <stdlib.h>

typedef struct {
    int coefX ; 
    int coefY ;
} Direction ;

Direction* init_dir(int coefX , int coefY);
void free_dir(void* direction);
Direction* dir_of (int dir);

#endif