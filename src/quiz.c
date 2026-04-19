#include "quiz.h"
#include <stdio.h>
#include <stdlib.h>

int charger(QuizQuestion t[], int *n) {
    FILE *f = fopen("assets/quiz/questions.txt", "r");
    if (!f) {
        printf("Error: Could not open assets/quiz/questions.txt\n");
        return 0;
    }

    *n = 0;
    while (*n < MAX && !feof(f)) {
        if (fscanf(f, "Q: %[^\n]\n", t[*n].question) != 1) break;
        fscanf(f, "A: %[^\n]\n", t[*n].A);
        fscanf(f, "B: %[^\n]\n", t[*n].B);
        fscanf(f, "C: %[^\n]\n", t[*n].C);
        fscanf(f, "R: %c\n\n", &t[*n].correcte);
        t[*n].dejaVu = 0;
        (*n)++;
    }

    fclose(f);
    return 1;
}

int generer(QuizQuestion t[], int n) {
    if (n <= 0) return -1;
    int i;
    int tries = 0;
    do {
        i = rand() % n;
        tries++;
    } while (t[i].dejaVu && tries < 100);

    t[i].dejaVu = 1;
    return i;
}

SDL_Texture* renderText(char *msg, TTF_Font *font, SDL_Color color, SDL_Renderer *renderer, Uint32 wrapLength) {
    if (!msg || !font) return NULL;
    SDL_Surface *surf;
    if (wrapLength > 0)
        surf = TTF_RenderUTF8_Blended_Wrapped(font, msg, color, wrapLength);
    else
        surf = TTF_RenderUTF8_Blended(font, msg, color);
        
    if (!surf) return NULL;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

void drawText(SDL_Texture *tex, int x, int y, SDL_Renderer *renderer) {
    if (!tex) return;
    SDL_Rect r;
    SDL_QueryTexture(tex, NULL, NULL, &r.w, &r.h);
    r.x = x;
    r.y = y;
    SDL_RenderCopy(renderer, tex, NULL, &r);
}



