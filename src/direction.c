#include "../include/direction.h"

Direction *init_dir(int x, int y) {
  Direction *dir = malloc(sizeof(Direction));
  dir->coefX = x;
  dir->coefY = y;
  return dir;
}

Direction directions[] = {{0, -1}, {1, 0},  {0, 1},  {-1, 0},
                          {1, 1},  {-1, 1}, {1, -1}, {-1, -1}};

Direction *dir_of(int dir) {
  if (dir < 0 || dir >= 8) {
    return NULL;
  }
  return &directions[dir];
}

void free_dir(void *dir) { free(dir); }
