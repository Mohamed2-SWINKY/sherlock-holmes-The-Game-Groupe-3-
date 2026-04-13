# Nom de l'exécutable final
EXEC = jeu_minimap

# Compilateur
CC = gcc

# Options de compilation (Warnings, Debug, et Standard C99)
CFLAGS = -Wall -Wextra -g -std=c99

# Bibliothèques SDL2 à lier
LDFLAGS = -lSDL2 -lSDL2_image

# Liste des fichiers sources
SRC = main.c minimap.c

# Liste des fichiers objets (générés automatiquement à partir de SRC)
OBJ = $(SRC:.c=.o)

# Règle par défaut : compile le projet
all: $(EXEC)

# Création de l'exécutable
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

# Compilation des fichiers .c en .o
%.o: %.c minimap.h
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers temporaires
clean:
	rm -f $(OBJ)

# Nettoyage complet (objets + exécutable)
mrproper: clean
	rm -f $(EXEC)

# Pour éviter les conflits avec des fichiers du même nom
.PHONY: all clean mrproper
