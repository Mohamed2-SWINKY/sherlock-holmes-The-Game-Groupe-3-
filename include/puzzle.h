#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "status.h"

#define W    800
#define H    500
#define BG_X 10
#define BG_Y 10
#define BG_W 560
#define BG_H 480
#define PW   120
#define PH   110
#define NB   5
#define TMAX 10000
#define SNAP 60

typedef struct {
    SDL_Rect hole;
    SDL_Rect src[3];
    int correct;
} Puzzle;

typedef struct {
    SDL_Rect src;
    SDL_Rect pos;
    SDL_Rect orig;
    int drag;
    int placed;
    int correct;
} Piece;

typedef struct {
    SDL_Texture *bg;
    SDL_Texture *wTex;
    SDL_Texture *lTex;
    Mix_Chunk   *snd;
    TTF_Font    *fnt;
    Piece       pc[3];
    SDL_Rect    slots[3];
    SDL_Rect    hScr;
    SDL_Rect    target;
    Uint32      startTime;
    int         dIdx;
    int         ox, oy;
    int         run;
    int         result; // -1: playing, 1: win, 0: loss
    int         over;   // Flag to signal main game loop
    int         fate;   // Which puzzle image to use
    double      angle;
    float       scale;
} PuzzleState;

void puzzle_init_state (PuzzleState *ps, SDL_Renderer *rend);
void puzzle_handle_event(PuzzleState *ps, SDL_Event *ev);
void puzzle_update      (PuzzleState *ps);
void puzzle_render      (PuzzleState *ps, SDL_Renderer *rend);
void puzzle_free_state  (PuzzleState *ps);

// Keep some original logic if needed, but we'll mostly use the new functions
int  enigme2(SDL_Renderer *rend, int fate, int *solved);
