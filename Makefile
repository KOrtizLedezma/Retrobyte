CC = gcc
CFLAGS = -lncurses

# Paths
LAUNCHER = main
LAUNCHER_SRC = main.c

GAMES_DIR = Games
GAMES = pingpong snake tetris

# Default target
all: $(LAUNCHER) $(GAMES)

# Build launcher
$(LAUNCHER): $(LAUNCHER_SRC)
	$(CC) $(LAUNCHER_SRC) -o $(LAUNCHER) $(CFLAGS)

# Build each game in Games/
$(GAMES):
	$(CC) $(GAMES_DIR)/$@.c -o $(GAMES_DIR)/$@ $(CFLAGS)

# Clean up executables
clean:
	rm -f $(LAUNCHER)
	rm -f $(addprefix $(GAMES_DIR)/, $(GAMES))

.PHONY: all clean $(GAMES)
