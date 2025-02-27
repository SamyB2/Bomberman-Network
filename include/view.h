#ifndef VIEW_H
#define VIEW_H

#include "board.h"
#include "deque.h"
#include "pos.h"
#include "bomb.h"
#include "player.h"
#include "chat.h"

#include <ncurses.h>

typedef struct {
  char c;
  int color;
} CellRepresentation;

typedef enum ACTION { NONE = 5, UP = 0, DOWN = 2, LEFT = 3, RIGHT = 1,
        QUIT  = 6, DROP = 4, SEND = 8, REFRESH = 9} ACTION;

typedef struct {
  int key;
  ACTION action;
} KeyActionMapping;

void refresh_board(Board* b);
void refresh_chat(Chat* c, int pos);
void refresh_line(Line *l, const char* prompt, int pos);
ACTION control(Chat* chat, int gamemode);
void setup_view(void);

#endif
