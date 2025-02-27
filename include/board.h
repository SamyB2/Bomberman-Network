#ifndef BOARD_H
#define BOARD_H

#include "pos.h"
#include "direction.h"
#include "stack.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024
#define WALL 'W'
#define D_WALL '*'
#define PATH 'P'

typedef struct {
    char* grid;
    int w;
    int h;

    // For optimization, we keep track of the cells that have changed since the last refresh
    char* changed_cells;
    size_t changed_cells_size;
    size_t changed_cells_capacity;
    int changed_cells_count;
} Board;

void setup_board(Board* Board);
void free_board(Board* Board);
char get_grid(Board* b, int x, int y);
int is_in_bounds(Board* b, int x, int y);
void set_grid(Board* b, int x, int y, char v);
void set_grid_cell(Board* b, int x, int y, char v);
void reset_changed_cells(Board* b);
void gen_map(Board* Board);

#endif
