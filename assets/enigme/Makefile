all: main.o enigme.o
	gcc main.o enigme.o -o enigme -lSDL2 -lSDL2_image -lSDL2_mixer

main.o: main.c
	gcc -c main.c

enigme.o: enigme.c
	gcc -c enigme.c

run:
	./enigme
