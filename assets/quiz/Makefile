CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS = -lSDL2 -lSDL2_ttf -lSDL2_mixer -lSDL2_image

SRC = main.c source.c
OBJ = main.o source.o

all: quiz

quiz: $(OBJ)
	$(CC) $(OBJ) -o quiz $(LIBS)

main.o: main.c header.h
	$(CC) $(CFLAGS) -c main.c

source.o: source.c header.h
	$(CC) $(CFLAGS) -c source.c

clean:
	rm -f *.o quiz
