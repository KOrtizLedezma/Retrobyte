#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <stdbool.h>

#define WIDTH 40
#define HEIGHT 20
#define DELAY 100000
#define MIN_DELAY 20000

typedef struct {
  int x,y;
} Point;

enum direction {UP, DOWN, LEFT, RIGHT};

Point dir = {1, 0};
Point snake[800];
Point food;

int snake_length = 5;
int offset_x = 0;
int offset_y = 0;
int score = 0;
int delay = DELAY;

enum direction curr_dir = RIGHT;

bool food_available = FALSE;

void init_snake() {
  for (int i = 0; i < snake_length; i++) {
    snake[i].x = 10-i;
    snake[i].y = 10;
  }
}

void draw_border() {
  attron(COLOR_PAIR(3));
  for (int x = 0; x <= WIDTH + 1; x++) {
    mvprintw(offset_y, x + offset_x, "█");                    // Top
    mvprintw(offset_y + HEIGHT + 1, x + offset_x, "█");       // Bottom
  }

  for (int y = 1; y <= HEIGHT; y++) {
    mvprintw(offset_y + y, offset_x, "█");                    // Left
    mvprintw(offset_y + y, offset_x + WIDTH + 1, "█");        // Right
  }
}

void draw() {
  clear();
  draw_border();
  attron(COLOR_PAIR(1));
  for (int i = 0; i < snake_length; i++) {
    mvprintw(snake[i].y + offset_y, snake[i].x + offset_x, "■");
  }
  attron(COLOR_PAIR(1));

  if (food_available) {
    attron(COLOR_PAIR(2));
    mvprintw(food.y + offset_y, food.x + offset_x, "■");
    attroff(COLOR_PAIR(2));
  }

  refresh();
}

void check_food_collision() {
  if (snake[0].x == food.x && snake[0].y == food.y) {
    snake_length += 1;
    score += 10;
    food_available = FALSE;

    if (score % 50 == 0 && delay > MIN_DELAY) {
      delay -= 10000;
    }
  }
}

void game_over() {
  clear();
  mvprintw(offset_y + HEIGHT / 2, offset_x + WIDTH / 2 - 5, "Game Over!");
  mvprintw(offset_y + HEIGHT / 2 + 1, offset_x + WIDTH / 2 - 8, "Final Score: %d", score);
  mvprintw(offset_y + HEIGHT / 2 + 3, offset_x + WIDTH / 2 - 10, "Press R to Restart");
  mvprintw(offset_y + HEIGHT / 2 + 4, offset_x + WIDTH / 2 - 7, "or Q to Quit");
  refresh();

  nodelay(stdscr, FALSE);

  int ch;
  while (1) {
    ch = getch();
    if (ch == 'q' || ch == 'Q') {
      endwin();
      exit(0);
    } else if (ch == 'r' || ch == 'R') {
      snake_length = 5;
      score = 0;
      delay = DELAY;
      dir.x = 1; dir.y = 0;
      curr_dir = RIGHT;
      food_available = FALSE;
      init_snake();
      nodelay(stdscr, TRUE);
      return;
    }
  }
}


void check_body_collision() {
  for (int i = 1; i < snake_length; i++) {
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
      game_over();
    }
  }
}

void update() {

  check_food_collision();
  check_body_collision();

  for (int i = snake_length - 1; i > 0; i--) {
    snake[i] = snake[i - 1];
  }

  snake[0].x += dir.x;
  snake[0].y += dir.y;

  if (snake[0].x >= WIDTH + 1) snake[0].x = 1;
  if (snake[0].x <= 0) snake[0].x = WIDTH;
  if (snake[0].y >= HEIGHT + 1) snake[0].y = 1;
  if (snake[0].y <= 0) snake[0].y = HEIGHT;

}

void handle_input() {
  int ch = getch();
  switch (ch) {
    case KEY_UP:
      if (curr_dir != DOWN) {
        dir.x = 0; dir.y = -1;
        curr_dir = UP;
        break;
      }
    case KEY_DOWN:
      if (curr_dir != UP) {
        dir.x = 0; dir.y = 1;
        curr_dir = DOWN;
        break;
      }
    case KEY_LEFT:
      if (curr_dir != RIGHT) {
        dir.x = -1; dir.y = 0;
        curr_dir = LEFT;
        break;
      }
    case KEY_RIGHT:
      if (curr_dir != LEFT) {
        dir.x = 1; dir.y = 0;
        curr_dir = RIGHT;
        break;
      }
  }
}

bool position_on_snake(int x, int y) {
  for (int i = 0; i < snake_length; i++) {
    if (snake[i].x == x && snake[i].y == y) return TRUE;
  }
  return FALSE;
}

void spawn_food(){
  if (food_available) return;

  do {
    food.x = rand() % WIDTH + 1;
    food.y = rand() % HEIGHT + 1;
  } while (position_on_snake(food.x, food.y));

  food_available = TRUE;
}

int main() {
  setlocale(LC_ALL, "");
  initscr();

  start_color();
  use_default_colors(); // Allows transparency on terminals that support it

  // Define color pairs
  init_pair(1, COLOR_GREEN, -1);  // Snake color
  init_pair(2, COLOR_RED, -1);    // Food color
  init_pair(3, COLOR_BLUE, -1);   // Border color


  int max_x, max_y;
  getmaxyx(stdscr, max_y, max_x);
  offset_x = (max_x - WIDTH) / 2;
  offset_y = (max_y - HEIGHT) / 2;

  noecho();
  curs_set(FALSE);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);

  init_snake();

  while (1) {
    handle_input();
    update();
    draw();
    spawn_food();
    usleep(delay);
  }

  endwin();
  return 0;
}