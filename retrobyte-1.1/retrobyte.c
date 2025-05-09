#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>


#define WIDTH 60
#define HEIGHT 30
#define DELAY 25000

int offset_x = 0;
int offset_y = 0;

const char* retrobyte_title[] = {
" ____      _             ____        _       ",
"|  _ \\ ___| |_ _ __ ___ | __ ) _   _| |_ ___ ",
"| |_) / _ \\ __| '__/ _ \\|  _ \\| | | | __/ _ \\",
"|  _ <  __/ |_| | | (_) | |_) | |_| | ||  __/",
"|_| \\_\\___|\\__|_|  \\___/|____/ \\__, |\\__\\___|",
"                               |___/         ",
};

const int title_lines = 6;

const char* menu_items[] = { "ArcadePong", "ArchSnake", "Brickfall", "Exit" };
const int menu_count = 4;
int selected_index = 0;

void draw_border() {
  for (int x = 0; x <= WIDTH + 1; x++) {
    mvprintw(offset_y, x + offset_x, "█");
    mvprintw(offset_y + HEIGHT + 1, x + offset_x, "█");
  }

  for (int y = 1; y <= HEIGHT; y++) {
    mvprintw(offset_y + y, offset_x, "█");
    mvprintw(offset_y + y, offset_x + WIDTH + 1, "█");
  }
}

void draw_options() {
  int center_x = offset_x + WIDTH / 2;

  for (int i = 0; i < title_lines; i++) {
    int color_pair = 1 + (i % 6);
    attron(COLOR_PAIR(color_pair));
    mvprintw(offset_y + 5 + i, center_x - 22, "%s", retrobyte_title[i]);
    attroff(COLOR_PAIR(color_pair));
  }

  for (int i = 0; i < menu_count; i++) {
    if (i == selected_index) {
      attron(A_REVERSE);
    }
    mvprintw(offset_y + 14 + i * 2, center_x - strlen(menu_items[i]) / 2, "%s", menu_items[i]);
    if (i == selected_index) {
      attroff(A_REVERSE);
    }
  }
}

void draw() {
  clear();
  draw_border();
  draw_options();
  refresh();
}

void launch_game(const char* game_name) {
  endwin();
  pid_t pid = fork();

  if (pid == 0) {
    execlp(game_name, game_name, NULL);
    perror("Failed to launch game");
    exit(EXIT_FAILURE);
  }
  else if (pid > 0) {
    int status;
    waitpid(pid, &status, 0);
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_RED, -1);
    init_pair(2, COLOR_YELLOW, -1);
    init_pair(3, COLOR_GREEN, -1);
    init_pair(4, COLOR_CYAN, -1);
    init_pair(5, COLOR_BLUE, -1);
    init_pair(6, COLOR_MAGENTA, -1);
  } else {
    perror("fork failed");
  }
}

void handle_input() {
  int ch = getch();
  switch (ch) {
    case KEY_UP:
      selected_index = (selected_index - 1 + menu_count) % menu_count;
    break;
    case KEY_DOWN:
      selected_index = (selected_index + 1) % menu_count;
    break;
    case '\n':
      if (strcmp(menu_items[selected_index], "Exit") == 0) {
        endwin();
        exit(0);
      } else {
        char cmd[64];
        snprintf(cmd, sizeof(cmd), "%s", menu_items[selected_index]);
        for (int i = 0; cmd[i]; i++) {
          if (cmd[i] >= 'A' && cmd[i] <= 'Z') {
            cmd[i] = cmd[i] + 32;
          }
        }
        endwin();
        printf("DEBUG: launching: %s\n", cmd);
        fflush(stdout);
        sleep(2);
        launch_game(cmd);
      }
    break;
  }
}

int main() {
  setlocale(LC_ALL, "");
  initscr();
  start_color();
  use_default_colors();
  init_pair(1, COLOR_RED,     -1);
  init_pair(2, COLOR_YELLOW,  -1);
  init_pair(3, COLOR_GREEN,   -1);
  init_pair(4, COLOR_CYAN,    -1);
  init_pair(5, COLOR_BLUE,    -1);
  init_pair(6, COLOR_MAGENTA, -1);
  int max_x, max_y;
  getmaxyx(stdscr, max_y, max_x);
  offset_x = (max_x - WIDTH) / 2;
  offset_y = (max_y - HEIGHT) / 2;
  noecho();
  curs_set(FALSE);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  while (1) {
    handle_input();
    draw();
    usleep(DELAY);
  }
  endwin();
  return 0;
}
