#include "../include/bomb.h"

Bomb *init_bomb(Pos *pos, int range, int timer) {
  Bomb *bomb = malloc(sizeof(Bomb));
  bomb->pos = pos;
  bomb->range = range;
  bomb->timer = timer;
  return bomb;
}

void free_bomb(void *bomb) {
  free(((Bomb *)bomb)->pos);
  free(bomb);
}

int in_range(Pos *pos, Pos *bombPos, int range) {
  int dx = pos->x - bombPos->x;
  int dy = pos->y - bombPos->y;
  return dx * dx + dy * dy <= range * range;
}
