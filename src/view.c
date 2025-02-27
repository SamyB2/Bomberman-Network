#include "../include/view.h"

void refresh_board(Board *b) {
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
  attroff(COLOR_PAIR(2)); // Disable custom color 2
  attroff(A_BOLD);        // Disable bold
  refresh();
}

void refresh_line(Line *l, const char* prompt, int pos) {
  attron(A_BOLD); // Enable bold
  attron(COLOR_PAIR(1)); // Enable custom color 1
  mvprintw(pos, 0, "%s %s", prompt, l->data);
  size_t len = strlen(l->data) + strlen(prompt);
  for (size_t j = len; j < TEXT_SIZE; j++) {
    mvprintw(pos, (int)j, " ");
  }
  attroff(COLOR_PAIR(1)); // Disable custom color 1
  attroff(A_BOLD);        // Disable bold
  refresh();
}

ACTION control(Chat *chat, int gamemode) {
  int c;
  int prev_c = ERR;
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
	case ',':
		if (gamemode == 1) break;
    char *dst[2] = {"[ALL]: >_", "[TEAM]: >_"};
    size_t len = 10;
    memset(chat->prompt, 0, len);
		chat->destination =   (chat->destination + 1) % 2;
    strncpy(chat->prompt, dst[chat->destination], len);
    a = REFRESH;
		break;
  case '\n':
    a = SEND;
    break;
  case KEY_BACKSPACE:
    remove_char(chat->current_message);
    a = REFRESH;
    break;
  default:
    if (prev_c >= ' ' && prev_c <= '~') {
      add_char(l, prev_c);
      a = REFRESH;
    }
    break;
  }
  return a;
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