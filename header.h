#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define W    800
#define H    500
#define BG_X 10
#define BG_Y 10
#define BG_W 560
#define BG_H 480
#define PW   120
#define PH   110
#define NB   5
#define TMAX 30000
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

extern int score;

int  load_resources (SDL_Renderer *rend, int fate,
                     SDL_Texture **bg,
                     Mix_Chunk   **snd,
                     TTF_Font    **fnt,
                     SDL_Texture **wTex,
                     SDL_Texture **lTex);

void free_resources (SDL_Texture *bg,
                     SDL_Texture *wTex, SDL_Texture *lTex,
                     Mix_Chunk   *snd,  TTF_Font    *fnt);

void init_puzzle    (SDL_Texture *bg,
                     Piece pc[3], SDL_Rect slots[3],
                     SDL_Rect *hScr, SDL_Rect *target);

void on_quit        (int *run, int *result);

void on_mouse_down  (SDL_MouseButtonEvent *ev,
                     Piece pc[3],
                     int *dIdx, int *ox, int *oy);

void on_mouse_motion(SDL_MouseMotionEvent *ev,
                     Piece pc[3], int dIdx, int ox, int oy);

void on_mouse_up    (SDL_MouseButtonEvent *ev,
                     Piece pc[3], SDL_Rect target,
                     int *dIdx,
                     int *run, int *result);

void render_frame   (SDL_Renderer *rend, SDL_Texture *bg,
                     Piece pc[3], SDL_Rect hScr,
                     Uint32 elapsed);

void rotozoom       (SDL_Renderer *rend,
                     SDL_Texture *tex, int win);

int  enigme2        (SDL_Renderer *rend, int fate, int *solved);
