CC      = gcc
CFLAGS  = -Wall -Wextra -O2 $(shell sdl2-config --cflags)
LIBS    = $(shell sdl2-config --libs) -lSDL2_image -lm
TARGET  = game

all: $(TARGET)

$(TARGET): main.c game.c game.h
	$(CC) $(CFLAGS) main.c game.c -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)
