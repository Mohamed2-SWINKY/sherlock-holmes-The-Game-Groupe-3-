CC     = gcc
CFLAGS = -Wall
LIBS   = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lm

all: game

game: main.c rss.c header.h
	$(CC) $(CFLAGS) main.c rss.c -o game $(LIBS)

clean:
	rm -f game
