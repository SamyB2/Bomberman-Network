#include "../include/pos.h"

Pos *init_pos(int x, int y) {
  Pos *pos = malloc(sizeof(Pos));
  pos->x = x;
  pos->y = y;
  return pos;
}

Pos *copy_pos(Pos *pos) {
  Pos *res = malloc(sizeof(Pos));
  res->x = pos->x;
  res->y = pos->y;
  return res;
}