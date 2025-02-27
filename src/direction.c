#include "../include/direction.h"

Direction *init_dir(int x, int y) {
  Direction *dir = malloc(sizeof(Direction));
  dir->coefX = x;
  dir->coefY = y;
  return dir;
}

Direction *dir_of(int dir) {
  switch (dir) {
  case 0:
    return init_dir(0, -1);
  case 1:
    return init_dir(1, 0);
  case 2:
    return init_dir(0, 1);
  case 3:
    return init_dir(-1, 0);
  case 4:
    return init_dir(1, 1);
  case 5:
    return init_dir(-1, 1);
  case 6:
    return init_dir(1, -1);
  case 7:
    return init_dir(-1, -1);
  default:
    return NULL;
  }
}

void free_dir(void *dir) { free(dir); }
