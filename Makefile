SH_UE: obj/main.o obj/players.o obj/minimap.o obj/map.o
	gcc obj/main.o obj/players.o obj/minimap.o obj/map.o -o SH_UE -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lm

obj/main.o: src/main.c
	mkdir -p obj
	gcc -c src/main.c -o obj/main.o

obj/players.o: src/players.c
	mkdir -p obj
	gcc -c src/players.c -o obj/players.o

obj/minimap.o: src/minimap.c
	mkdir -p obj
	gcc -c src/minimap.c -o obj/minimap.o
	
obj/map.o: src/map.c
	mkdir -p obj
	gcc -c src/map.c -o obj/map.o

clean:
	rm -f SH_UE obj/*.o
