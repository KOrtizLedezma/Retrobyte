#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <stdbool.h>
#include <time.h>

#define WIDTH 10
#define HEIGHT 20
#define SIDE_WIDTH 11
#define SIDE_HEIGHT 20
#define DELAY 250000

typedef struct {
  int x, y;
} Point;

#define NUM_BLOCKS 7

const Point tetromino_shapes[NUM_BLOCKS][4] = {
  {{0,0}, {0,1}, {0,2}, {0,3}},
  {{0,0}, {1,0}, {0,1}, {1,1}},
  {{0,0}, {-1,1}, {0,1}, {1,1}},
  {{0,0}, {1,0}, {0,1}, {-1,1}},
  {{0,0}, {-1,0}, {0,1}, {1,1}},
  {{0,0}, {0,1}, {0,2}, {-1,2}},
  {{0,0}, {0,1}, {0,2}, {1,2}}
};

int offset_x = 0;
int offset_y = 0;
int score = 0;
int delay = DELAY;
int last_drop_time = 0;
bool just_moved = FALSE;

WINDOW* score_win;
WINDOW* next_win;
WINDOW* board_win;

int current_block = 0;
int next_block = 0;
Point block_origin;
Point active_block[4];

int grid[HEIGHT][WIDTH] = {0};

void draw_next_block() {
  for (int y = 11; y < 19; y++) {
    for (int x = 1; x < 18; x++) {
      mvwprintw(next_win, y, x, " ");
    }
  }

  int center_y = 11;
  int center_x = 9;

  wattron(next_win, COLOR_PAIR(next_block + 1));
  for (int i = 0; i < 4; i++) {
    int x = tetromino_shapes[next_block][i].x;
    int y = tetromino_shapes[next_block][i].y;
    mvwprintw(next_win, center_y + y, center_x + x * 2, "[]");
  }
  wattroff(next_win, COLOR_PAIR(next_block + 1));
}


void draw_side_panels() {
  box(score_win, 0, 0);
  mvwprintw(score_win, 9, 8, "Score");
  mvwprintw(score_win, 11, 10, "%d", score);
  wrefresh(score_win);

  box(next_win, 0, 0);
  mvwprintw(next_win, 9, 8, "Next");
  draw_next_block();
  wrefresh(next_win);
}

void draw_grid() {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      if (grid[y][x]) {
        wattron(board_win, COLOR_PAIR(grid[y][x]));
        mvwprintw(board_win, y + 1, x * 2 + 1, "[]");
        wattroff(board_win, COLOR_PAIR(grid[y][x]));
      } else {
        mvwprintw(board_win, y + 1, x * 2 + 1, " ");
      }
    }
  }
}

void draw_border() {
  box(board_win, 0, 0);
  wrefresh(board_win);
}

void draw_current_block() {
  wattron(board_win, COLOR_PAIR(current_block + 1));
  for (int i = 0; i < 4; i++) {
    int bx = block_origin.x + active_block[i].x;
    int by = block_origin.y + active_block[i].y;
    mvwprintw(board_win, by + 1, bx * 2 + 1, "[]");
  }
  wattroff(board_win, COLOR_PAIR(current_block + 1));
}

void draw() {
  werase(board_win);
  draw_border();
  draw_grid();
  draw_current_block();
  draw_side_panels();
  wrefresh(board_win);
}

bool check_collision(Point origin, Point shape[4]) {
  for (int i = 0; i < 4; i++) {
    int x = origin.x + shape[i].x;
    int y = origin.y + shape[i].y;
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return TRUE;
    if (grid[y][x]) return TRUE;
  }
  return FALSE;
}

void lock_block_into_grid() {
  for (int i = 0; i < 4; i++) {
    int x = block_origin.x + active_block[i].x;
    int y = block_origin.y + active_block[i].y;
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
      grid[y][x] = current_block + 1;
    }
  }
}

void clear_lines() {
  for (int y = HEIGHT - 1; y >= 0; y--) {
    bool full = TRUE;
    for (int x = 0; x < WIDTH; x++) {
      if (!grid[y][x]) {
        full = FALSE;
        break;
      }
    }
    if (full) {
      for (int row = y; row > 0; row--) {
        for (int x = 0; x < WIDTH; x++) {
          grid[row][x] = grid[row - 1][x];
        }
      }
      for (int x = 0; x < WIDTH; x++) grid[0][x] = 0;
      y++;
      score += 100;
    }
  }
}

void init_next_block() {
  current_block = next_block;
  next_block = rand() % NUM_BLOCKS;
  for (int i = 0; i < 4; i++) {
    active_block[i] = tetromino_shapes[current_block][i];
  }
  block_origin.x = WIDTH / 2;
  block_origin.y = 1;

  last_drop_time = 0;
}

void game_over() {
  delwin(board_win);
  delwin(score_win);
  delwin(next_win);

  clear();
  refresh();

  int board_width = WIDTH * 2 + 2;
  int board_height = HEIGHT + 2;
  WINDOW* gameover_win = newwin(board_height, board_width, offset_y, offset_x);

  box(gameover_win, 0, 0);

  int center_y = board_height / 2;
  int center_x = board_width / 2;

  mvwprintw(gameover_win, center_y - 1, center_x - 6, " GAME OVER ");
  mvwprintw(gameover_win, center_y, center_x - 8, " Final Score: %d", score);
  mvwprintw(gameover_win, center_y + 2, center_x - 10, " Press R to Restart ");
  mvwprintw(gameover_win, center_y + 3, center_x - 7, " or Q to Quit ");
  wrefresh(gameover_win);

  nodelay(stdscr, FALSE);
  int ch;
  while (1) {
    ch = getch();
    if (ch == 'q' || ch == 'Q') {
      delwin(gameover_win);
      endwin();
      exit(0);
    } else if (ch == 'r' || ch == 'R') {
      delwin(gameover_win);

      clear();
      refresh();

      int max_x, max_y;
      getmaxyx(stdscr, max_y, max_x);
      int board_width = WIDTH * 2 + 2;
      int board_height = HEIGHT + 2;

      board_win = newwin(board_height, board_width, offset_y, offset_x);
      score_win = newwin(board_height, 20, offset_y, offset_x - 22);
      next_win = newwin(board_height, 20, offset_y, offset_x + board_width + 2);

      box(board_win, 0, 0);
      box(score_win, 0, 0);
      box(next_win, 0, 0);

      wrefresh(board_win);
      wrefresh(score_win);
      wrefresh(next_win);

      for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
          grid[y][x] = 0;
        }
      }
      score = 0;
      next_block = rand() % NUM_BLOCKS;
      init_next_block();
      nodelay(stdscr, TRUE);
      return;
    }
  }
}



void update(int current_time) {
  if (current_time - last_drop_time >= delay && !just_moved) {
    Point next = { block_origin.x, block_origin.y + 1 };
    if (!check_collision(next, active_block)) {
      block_origin.y++;
    } else {
      lock_block_into_grid();
      clear_lines();
      init_next_block();
      if (check_collision(block_origin, active_block)) game_over();
    }
    last_drop_time = current_time;
  }

  just_moved = FALSE;
}

bool rotate_block(Point rotated[4]) {
  for (int i = 0; i < 4; i++) {
    int x = active_block[i].x;
    int y = active_block[i].y;
    rotated[i].x = y;
    rotated[i].y = -x;
  }
  return TRUE;
}

void handle_input(int current_time) {
  while (TRUE) {
    int ch = getch();
    if (ch == ERR) break;

    Point test_origin = block_origin;
    switch (ch) {
      case KEY_LEFT:
        test_origin.x--;
        if (!check_collision(test_origin, active_block)) {
          block_origin = test_origin;
          just_moved = TRUE;
        }
        break;
      case KEY_RIGHT:
        test_origin.x++;
        if (!check_collision(test_origin, active_block)) {
          block_origin = test_origin;
          just_moved = TRUE;
        }
        break;
      case KEY_DOWN:
        test_origin.y++;
        if (!check_collision(test_origin, active_block)) {
          block_origin = test_origin;
          last_drop_time = current_time;
          score += 1;
        }
        break;
      case KEY_UP:
        if (current_block == 1) break;
        Point rotated[4];
        rotate_block(rotated);
        if (!check_collision(block_origin, rotated)) {
          for (int i = 0; i < 4; i++) active_block[i] = rotated[i];
          just_moved = TRUE;
        }
        break;
    }
  }
}

int main() {
  setlocale(LC_ALL, "");
  initscr();

  start_color();
  use_default_colors();

  init_pair(1, COLOR_CYAN,    -1);
  init_pair(2, COLOR_YELLOW,  -1);
  init_pair(3, COLOR_MAGENTA, -1);
  init_pair(4, COLOR_BLUE,    -1);
  init_pair(5, COLOR_WHITE,   -1);
  init_pair(6, COLOR_GREEN,   -1);
  init_pair(7, COLOR_RED,     -1);
  init_pair(8, COLOR_WHITE,   -1);

  refresh();
  int max_x, max_y;
  getmaxyx(stdscr, max_y, max_x);
  int board_width = WIDTH * 2 + 2;
  int board_height = HEIGHT + 2;
  offset_x = (max_x - board_width) / 2;
  offset_y = (max_y - board_height) / 2;

  board_win = newwin(board_height, board_width, offset_y, offset_x);
  score_win = newwin(SIDE_HEIGHT + 2, 21, offset_y, offset_x - 22);
  next_win  = newwin(SIDE_HEIGHT + 2, 21, offset_y, offset_x + board_width + 2);

  noecho();
  curs_set(FALSE);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  srand(time(NULL));
  next_block = rand() % NUM_BLOCKS;
  init_next_block();

  int current_time = 0;
  while (1) {
    handle_input(current_time);
    update(current_time);
    draw();
    usleep(DELAY / 10);
    current_time += DELAY / 10;
  }

  delwin(board_win);
  delwin(score_win);
  delwin(next_win);
  endwin();
  return 0;
}