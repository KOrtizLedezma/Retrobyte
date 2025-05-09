#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <stdbool.h>
#include <time.h>

#define WIDTH 81
#define HEIGHT 40
#define DELAY 50000

typedef struct {
  int x,y;
} Point;

int offset_x = 0;
int offset_y = 0;

int paddle_length = 7;
Point paddle[7];

Point paddle_pc[7];
int paddle_pc_length = 7;

Point dir = {0, 0};

Point ball;
Point ball_dir = {1, -1};

int frame_counter = 0;
int ball_frame_counter = 0;

int score_player = 0;
int score_pc = 0;

void init_paddle() {
  int center = WIDTH / 2;
  for (int i = 0; i < paddle_length; i++) {
    paddle[i].x = center - paddle_length / 2 + i;
    paddle[i].y = HEIGHT - 2;
  }
}

void init_paddle_pc() {
  int center_pc = WIDTH / 2;
  for (int i = 0; i < paddle_pc_length; i++) {
    paddle_pc[i].x = center_pc - paddle_pc_length / 2 + i;
    paddle_pc[i].y = 2;
  }
}

void init_ball() {
  ball.x = WIDTH / 2;
  ball.y = HEIGHT / 2;

  ball_dir.x = (rand() % 2 == 0) ? 1 : -1;
  ball_dir.y = (rand() % 2 == 0) ? 1 : -1;
}

void draw_border() {
  for (int x = 0; x <= WIDTH + 1; x++) {
    mvprintw(offset_y, x + offset_x, "█");
    mvprintw(offset_y + HEIGHT + 1, x + offset_x, "█");
  }

  for (int y = 1; y <= HEIGHT; y++) {
    mvprintw(offset_y + y, offset_x, "█");
    mvprintw(offset_y + y, offset_x + WIDTH + 1, "█");
  }

  for (int x = 1; x <= WIDTH; x += 2) {
    mvprintw(offset_y + HEIGHT / 2, offset_x + x, "·");
  }
}

void draw_paddle_player() {
  attron(COLOR_PAIR(1));
  for (int i = 0; i < paddle_length; i++) {
    mvprintw(offset_y + paddle[i].y, offset_x + paddle[i].x, "■");
  }
  attroff(COLOR_PAIR(1));
}

void draw_paddle_pc() {
  attron(COLOR_PAIR(2));
  for (int i = 0; i < paddle_pc_length; i++) {
    mvprintw(offset_y + paddle_pc[i].y, offset_x + paddle_pc[i].x, "■");
  }
  attroff(COLOR_PAIR(2));
}

void draw() {
  clear();
  draw_border();
  draw_paddle_player();
  draw_paddle_pc();

  attron(COLOR_PAIR(3));
  mvprintw(offset_y + ball.y, offset_x + ball.x, "●");
  attroff(COLOR_PAIR(3));

  mvprintw(offset_y + HEIGHT / 2 - 2, offset_x + WIDTH / 2 - 2, "%d", score_pc);
  mvprintw(offset_y + HEIGHT / 2 + 2, offset_x + WIDTH / 2 - 2, "%d", score_player);

  refresh();
}

void reset_game() {
  init_ball();
  init_paddle();
  init_paddle_pc();
}

void ball_movement() {
  ball_frame_counter++;

  if (ball_frame_counter % 2 == 0) {
    ball.x += ball_dir.x;
    ball.y += ball_dir.y;


    if (ball.x <= 1) {
      ball.x = 2;
      ball_dir.x *= -1;
    } else if (ball.x >= WIDTH) {
      ball.x = WIDTH - 1;
      ball_dir.x *= -1;
    }

    for (int i = 0; i < paddle_length; i++) {
      if (ball.y == paddle[i].y - 1 && ball.x >= paddle[i].x - 1 && ball.x <= paddle[i].x + 1) {
        ball_dir.y = -1;

        int paddle_center = paddle[paddle_length / 2].x;
        if (ball.x < paddle_center) {
          ball_dir.x = -1;
        } else if (ball.x > paddle_center) {
          ball_dir.x = 1;
        }

        break;
      }
    }

    for (int i = 0; i < paddle_pc_length; i++) {
      if (ball.y == paddle_pc[i].y + 1 && ball.x >= paddle_pc[i].x - 1 && ball.x <= paddle_pc[i].x + 1) {
        ball_dir.y = 1;

        int paddle_center = paddle_pc[paddle_pc_length / 2].x;
        if (ball.x < paddle_center) {
          ball_dir.x = -1;
        } else if (ball.x > paddle_center) {
          ball_dir.x = 1;
        }

        break;
      }
    }

    if (ball.y >= HEIGHT) {
      score_pc++;
      if (score_pc >= 10) {
        mvprintw(offset_y + HEIGHT / 2, offset_x + WIDTH / 2 - 5, "GAME OVER");
        refresh();
        sleep(2);
        endwin();
        exit(0);
      } else {
        reset_game();
      }
    }

    if (ball.y <= 1) {
      score_player++;
      if (score_player >= 10) {
        mvprintw(offset_y + HEIGHT / 2, offset_x + WIDTH / 2 - 4, "YOU WIN!");
        refresh();
        sleep(2);
        endwin();
        exit(0);
      } else {
        reset_game();
      }
    }
  }
}

void paddle_pc_movement() {
  int pc_center_x = 0;
  for (int i = 0; i < paddle_pc_length; i++) {
    pc_center_x += paddle_pc[i].x;
  }
  pc_center_x /= paddle_pc_length;

  int predicted_x = ball.x;

  if (ball_dir.y < 0) {
    int distance_y = ball.y - paddle_pc[0].y;
    if (distance_y > 0) {
      predicted_x = ball.x + (ball_dir.x * distance_y / abs(ball_dir.y));

      if (predicted_x < 1) predicted_x = 1;
      if (predicted_x > WIDTH) predicted_x = WIDTH;

      int difficulty = 3 - (score_player / 2);
      if (difficulty < 1) difficulty = 1;

      int random_error = (rand() % difficulty) - (difficulty / 2);
      predicted_x += random_error;
    }
  } else {
    predicted_x = WIDTH / 2 + (rand() % 11) - 5;
  }

  int move_dir = 0;
  if (predicted_x < pc_center_x - 1) move_dir = -1;
  else if (predicted_x > pc_center_x + 1) move_dir = 1;

  int max_speed = 2 + (score_player / 3);
  if (max_speed > 4) max_speed = 4;

  if (move_dir != 0) {
    int new_left_x = paddle_pc[0].x + move_dir;
    int new_right_x = new_left_x + paddle_pc_length - 1;

    if (new_left_x >= 1 && new_right_x <= WIDTH) {
      if (move_dir < 0) {
        for (int i = paddle_pc_length - 1; i > 0; i--) {
          paddle_pc[i] = paddle_pc[i - 1];
        }
        paddle_pc[0].x = new_left_x;
      } else {
        for (int i = 0; i < paddle_pc_length - 1; i++) {
          paddle_pc[i] = paddle_pc[i + 1];
        }
        paddle_pc[paddle_pc_length - 1].x = new_right_x;
      }
    }
  }
}

void update() {
  if (dir.x != 0) {
    int next_x = paddle[0].x + dir.x;
    int last_x = next_x + paddle_length - 1;

    if (next_x >= 1 && last_x <= WIDTH) {
      if (dir.x < 0) {
        for (int i = paddle_length - 1; i > 0; i--) {
          paddle[i] = paddle[i - 1];
        }
        paddle[0].x = next_x;
      } else {
        for (int i = 0; i < paddle_length - 1; i++) {
          paddle[i] = paddle[i + 1];
        }
        paddle[paddle_length - 1].x = last_x;
      }
    }
  }

  paddle_pc_movement();
  ball_movement();
}

void handle_input() {
  int ch = getch();
  switch (ch) {
    case KEY_LEFT:
      dir.x = -1;
      break;
    case KEY_RIGHT:
      dir.x = 1;
      break;
    case KEY_DOWN:
      dir.x = 0;
      break;
    case 'q':
      endwin();
      exit(0);
      break;
  }
}

int main() {
  srand(time(NULL));

  setlocale(LC_ALL, "");
  initscr();

  start_color();
  use_default_colors();

  init_pair(1, COLOR_GREEN, -1);
  init_pair(2, COLOR_RED, -1);
  init_pair(3, COLOR_YELLOW, -1);

  int max_x, max_y;
  getmaxyx(stdscr, max_y, max_x);
  offset_x = (max_x - WIDTH) / 2;
  offset_y = (max_y - HEIGHT) / 2;

  noecho();
  curs_set(FALSE);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);

  init_paddle();
  init_paddle_pc();
  init_ball();

  mvprintw(offset_y + HEIGHT / 2, offset_x + WIDTH / 4, "PONG - Use LEFT/RIGHT arrows to move");
  mvprintw(offset_y + HEIGHT / 2 + 1, offset_x + WIDTH / 4, "PONG - Use DOWN arrow to stop");
  mvprintw(offset_y + HEIGHT / 2 + 2, offset_x + WIDTH / 4, "Press any key to start");
  refresh();
  nodelay(stdscr, FALSE);
  getch();
  nodelay(stdscr, TRUE);

  while (1) {
    handle_input();
    update();
    draw();
    usleep(DELAY);
  }

  endwin();
  return 0;
}