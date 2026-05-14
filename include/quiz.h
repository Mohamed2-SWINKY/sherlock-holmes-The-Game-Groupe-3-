#ifndef QUIZ_H
#define QUIZ_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include "status.h"

#define MAX 50

typedef struct {
    char question[256];
    char A[100];
    char B[100];
    char C[100];
    char correcte;
    int dejaVu;
} QuizQuestion;

int charger(QuizQuestion t[], int *n);
int generer(QuizQuestion t[], int n);

SDL_Texture* renderText(char *msg, TTF_Font *font, SDL_Color color, SDL_Renderer *renderer, Uint32 wrapLength);
void drawText(SDL_Texture *tex, int x, int y, SDL_Renderer *renderer);

#endif

