#include "board.h"
#include "dequeue.h"
#include "pos.h"
#include "bomb.h"
#include "player.h"
#include "chat.h"

#include <ncurses.h>

typedef struct line {
    char data[TEXT_SIZE];
    int cursor;
} line;

typedef enum ACTION { NONE = 5 , UP = 0, DOWN = 2, LEFT = 3, RIGHT = 1 , QUIT  = 6 , DROP = 7 , SEND = 8} ACTION;

void refresh_game(Board* b, Chat* chat);
void refresh_chat(Chat* c, int pos );
ACTION control(Chat* chat);
bool perform_action(Board* b, Player * p, ACTION a , Dequeue * bombs);
void setup_view(void);
