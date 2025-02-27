#include "../include/board.h"

void setup_board(Board *b) {
  int lines = 20;
  int columns = 30;
  b->h = lines - 2 - 1; // 2 rows reserved for border, 1 row for chat
  b->w = columns - 2;   // 2 columns reserved for border
  b->grid = calloc((size_t)((b->w) * (b->h)), sizeof(char));

  // Intialize the tracking of changed cells
  b->changed_cells = malloc(BUFFER_SIZE * sizeof(char));
  b->changed_cells_capacity = BUFFER_SIZE;
  b->changed_cells_size = 0;
  b->changed_cells_count = 0;
}

void free_board(Board *b) {
  free(b->grid);
  free(b->changed_cells);
  free(b);
}

void print_board(Board *b) {
  for (int y = 0; y < b->h; y++) {
    for (int x = 0; x < b->w; x++) {
      char c = get_grid(b, x, y);
      if (c == 0) printf("0");
      else if (c == 2) printf("2");
      else if (c == 3) printf("3");
      else if (c == 1) printf("1");
      else printf("%c", get_grid(b, x, y));
    }
    printf("\n");
  }
} 

char get_grid(Board *b, int x, int y) { return b->grid[y * b->w + x]; }

int is_in_bounds(Board *b, int x, int y) {
  return x >= 0 && x < b->w && y >= 0 && y < b->h;
}

void set_grid_cell(Board *b, int x, int y, char v) {
  b->grid[y * b->w + x] = v;
}


void set_grid(Board *b, int x, int y, char v) {
  set_grid_cell(b, x, y, v);

  // Keep track of the changed cell for differential update of the board
  b->changed_cells_count++;
  u_int8_t xc = (u_int8_t)x;
  u_int8_t yc = (u_int8_t)y;
  u_int8_t vc = (u_int8_t)v;
  if (b->changed_cells_size + 3 * sizeof(u_int8_t) >
      b->changed_cells_capacity) {
    b->changed_cells_capacity += BUFFER_SIZE;
    b->changed_cells = realloc(b->changed_cells, b->changed_cells_capacity);
  }
  memcpy(b->changed_cells + b->changed_cells_size, &xc, sizeof(u_int8_t));
  b->changed_cells_size += sizeof(u_int8_t);
  memcpy(b->changed_cells + b->changed_cells_size, &yc, sizeof(u_int8_t));
  b->changed_cells_size += sizeof(u_int8_t);
  memcpy(b->changed_cells + b->changed_cells_size, &vc, sizeof(u_int8_t));
  b->changed_cells_size += sizeof(u_int8_t);
  b->changed_cells[b->changed_cells_size] = '\0';
}

void reset_changed_cells(Board *b) {
  b->changed_cells_count = 0;
  b->changed_cells_size = 0;
  b->changed_cells[0] = '\0';
}

void initialize_maze(Board *b) {
  // Initialize maze with walls
  for (int i = 0; i < b->w * b->h; i++) {
    if (rand() % 2) {
      b->grid[i] = WALL;
    } else {
      b->grid[i] = D_WALL;
    }
  }
}

// function of generating the map using the Growing Tree algorithm
void gen_map(Board *b) {
  srand((unsigned int)time(NULL));
  Pos stack[b->w * b->h];
  int top = 0;

  initialize_maze(b);

  // Choose a random starting Pos
  int startX = rand() % b->w;
  int startY = rand() % b->h;
  b->grid[startY * b->w + startX] = PATH;
  push(&(Pos){startX, startY}, stack, &top, sizeof(Pos));

  // Growing Tree algorithm
  while (!isEmpty(&top)) {
    Pos pos;
    peek(&pos, stack, &top, sizeof(Pos));
    Direction directions[4] = {(Direction){0, -1}, (Direction){0, 1},
                               (Direction){-1, 0}, (Direction){1, 0}};
    int tries = 4;
    // Randomly choose a direction to carve a path to an unvisited cell
    while (tries--) {
      int dirIndex = rand() % (tries + 1);
      Direction dir = directions[dirIndex];
      directions[dirIndex] = directions[tries];

      int nx = pos.x + 2 * dir.coefX;
      int ny = pos.y + 2 * dir.coefY;

      if (nx >= 0 && nx < b->w && ny >= 0 && ny < b->h &&
          (b->grid[ny * b->w + nx] == WALL ||
           b->grid[ny * b->w + nx] == D_WALL)) {
        b->grid[(pos.y + dir.coefY) * b->w + pos.x + dir.coefX] = PATH;
        b->grid[ny * b->w + nx] = PATH;
        push(&(Pos){nx, ny}, stack, &top, sizeof(Pos));
        break;
      }
    }

    if (tries < 0) {
      pop(&pos, stack, &top, sizeof(Pos));
    }
  }

  // Add border walls
  b->grid[0] = b->grid[1] = b->grid[b->w] = b->grid[b->w + 1] =
      b->grid[b->w + 2] = b->grid[b->w - 1] = b->grid[b->w * b->h - b->w] =
          b->grid[b->w * b->h - 1] = PATH;
}