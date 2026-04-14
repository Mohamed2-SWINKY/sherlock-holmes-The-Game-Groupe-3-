joueur: main.o source.o
	gcc main.o source.o -o joueur -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lm

main.o: main.c
	gcc -c main.c

source.o: source.c
	gcc -c source.c

clean:
	rm -f menu *.o
