// Build with -lncurses option
#include "../include/main.h"

void refresh_game(Board *b, Chat *chat) {
  int x, y, color = 1;
  for (y = 0; y < b->h; y++) {
    for (x = 0; x < b->w; x++) {
      char c;
      switch ((c = get_grid(b, x, y))) {
      case PATH:
        color = 0;
        c = ' ';
        break;
      case WALL:
        color = 2;
        c = '#';
        break;
      case D_WALL:
        color = 3; // Enable custom color 4
        c = 'X';
        break;
      case 0:
        color = 4; // Enable custom color 5
        c = '0';
        break;
      case 1:
        color = 5; // Enable custom color 6
        c = '1';
        break;
      case 2:
        color = 6; // Enable custom color 7
        c = '2';
        break;
      case 3:
        color = 7; // Enable custom color 8
        c = '3';
        break;
      case 'A':
        color = 4; // Enable custom color 4
        c = 'A';
        break;
      case 'B':
        color = 5; // Enable custom color 5
        c = 'B';
        break;
      case 'C':
        color = 6; // Enable custom color 6
        c = 'C';
        break;  
      case 'D':
        color = 7; // Enable custom color 7
        c = 'D';
        break;
      default:
        break;
      }
      attron(COLOR_PAIR(color)); // Enable custom color 4
      mvaddch(y + 1, x + 1, (chtype)c);
      attroff(COLOR_PAIR(color)); // Disable custom color 4
    }
  }
  for (x = 0; x < b->w + 2; x++) {
    mvaddch(0, x, '-');
    mvaddch(b->h + 1, x, '-');
  }
  for (y = 0; y < b->h + 2; y++) {
    mvaddch(y, 0, '|');
    mvaddch(y, b->w + 1, '|');
  }
  refresh_chat(chat, b->h + 2);
  refresh();
}

void refresh_chat(Chat *c, int pos) {
  attron(COLOR_PAIR(2)); // Enable custom color 2
  attron(A_BOLD);        // Enable bold
  for (int i = 0; i < c->max_messages; i++) {
    if (i < c->nb_messages) {
      char *message = get_message(c, i);
      mvprintw(pos + i, 0, "%s", message);
      size_t len = strlen(message);
      for (size_t j = len; j < TEXT_SIZE; j++) {
        mvprintw(pos + i, (int)j, " ");
      }
    } else {
      mvprintw(pos + i, 0, " ");
    }
  }
  attron(COLOR_PAIR(1)); // Enable custom color 1
  mvprintw(pos + c->max_messages, 0, "%s", c->prompt);
  mvprintw(pos + c->max_messages, 11, "%s", c->current_message->data);
  size_t len = strlen(c->current_message->data) + 11;
  for (size_t j = len; j < TEXT_SIZE; j++) {
    mvprintw(pos + c->max_messages, (int)j, " ");
  }
  attroff(A_BOLD);        // Disable bold
  attroff(COLOR_PAIR(1)); // Disable custom color 2
}

ACTION control(Chat *chat) {
  int c;
  int prev_c = ERR;
  char *message;
  // We consume all similar consecutive key presses
  while ((c = getch()) !=
         ERR) { // getch returns the first key press in the queue
    if (prev_c != ERR && prev_c != c) {
      ungetch(c); // put 'c' back in the queue
      break;
    }
    prev_c = c;
  }
  ACTION a = NONE;
  Line *l = chat->current_message;
  switch (prev_c) {
  case ERR:
    break;
  case KEY_LEFT:
    a = LEFT;
    break;
  case KEY_RIGHT:
    a = RIGHT;
    break;
  case KEY_UP:
    a = UP;
    break;
  case KEY_DOWN:
    a = DOWN;
    break;
  case '~':
    a = QUIT;
    break;
  case '.':
    a = DROP;
    break;
  case '\n':
    a = SEND;
    message = malloc(sizeof(char) * TEXT_SIZE);
    strcpy(message, l->data);
    add_message(chat, message);
    clear_line(l);
    break;
  case KEY_BACKSPACE:
    remove_char(chat->current_message);
    break;
  default:
    if (prev_c >= ' ' && prev_c <= '~')
      add_char(l, prev_c);
    break;
  }
  return a;
}

bool perform_action(Board *b, Player *p, ACTION a, Dequeue *bombs) {
  switch (a) {
  case NONE:
  case SEND:
    return false;
  case QUIT:
    return true;
  case DROP:
    drop_bomb(p, b, bombs, init_bomb(copy_pos(p->pos), 1, 100));
    return false;
  default:
    move_player(p, a, b);
    return false;
  }
  return false;
}

void setup_view() {
  initscr();                /* Start curses mode */
  raw();                    /* Disable line buffering */
  intrflush(stdscr, FALSE); /* No need to flush when intr key is pressed */
  keypad(stdscr, TRUE);     /* Required in order to get events from keyboard */
  nodelay(stdscr, TRUE);    /* Make getch non-blocking */
  noecho(); /* Don't echo() while we do getch (we will manually print characters
               when relevant) */
  curs_set(0);                             // Set the cursor to invisible
  start_color();                           // Enable colors
  init_pair(1, COLOR_YELLOW, COLOR_BLACK); // Define a new color style (text is
                                           // yellow, background is black)
  init_pair(2, COLOR_WHITE, COLOR_BLACK);  // Define a new color style (text is
                                           // white, background is black)
  init_pair(3, COLOR_RED, COLOR_BLACK);    // Define a new color style (text is
                                           // red, background is black
  init_pair(4, COLOR_BLUE, COLOR_BLACK);

  init_pair(5, COLOR_GREEN, COLOR_BLACK);

  init_pair(6, COLOR_MAGENTA, COLOR_BLACK);

  init_pair(7, COLOR_CYAN, COLOR_BLACK);

  init_pair(0, COLOR_BLACK, COLOR_BLACK); // Define a new color style (text is
                                           // black, background is black)

}

int main() {
  Board *b = malloc(sizeof(Board));
  Chat *chat = create_chat();
  Player *p = init_player(0, 0, init_pos(0, 0), 0);
  Dequeue *bombs = init_dequeue();

  // NOTE: All ncurses operations (getch, mvaddch, refresh, etc.) must be done
  // on the same thread.
  setup_view();

  setup_board(b);
  gen_map(b);
  b->grid[0] = 0;

  while (true) {
    ACTION a = control(chat);
    if (perform_action(b, p, a, bombs))
      break;
    reduce_timer(bombs, b, &p, 1);
    refresh_game(b, chat);
    usleep(30 * 1000);
  }
  curs_set(1); // Set the cursor to visible again
  endwin();    /* End curses mode */

  free_board(b);
  free_player(p);
  free_chat(chat);
  free_dequeue(bombs, free_bomb);

  return 0;
}
