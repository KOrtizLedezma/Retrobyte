CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lncurses

# Paths
LAUNCHER = retrobyte
LAUNCHER_SRC = retrobyte.c

GAMES_DIR = Games
GAMES = arcadepong archsnake brickfall
GAME_SRCS = $(addsuffix .c, $(addprefix $(GAMES_DIR)/, $(GAMES)))
GAME_BINS = $(addprefix $(GAMES_DIR)/, $(GAMES))

PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

# Default target
all: $(LAUNCHER) $(GAME_BINS)

# Build launcher
$(LAUNCHER): $(LAUNCHER_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Build each game
$(GAMES_DIR)/%: $(GAMES_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Install all executables
install: all
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(LAUNCHER) $(DESTDIR)$(BINDIR)
	install -m 755 $(GAME_BINS) $(DESTDIR)$(BINDIR)

# Clean build artifacts
clean:
	rm -f $(LAUNCHER)
	rm -f $(GAME_BINS)

.PHONY: all install clean
