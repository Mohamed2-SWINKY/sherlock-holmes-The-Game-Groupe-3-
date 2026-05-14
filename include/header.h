#ifndef HEADER_H
#define HEADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "status.h"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 650
#define MAX_PLAYERS 100
#define NAME_LEN 50

typedef enum {
    MENU_STATE_MAIN_MENU,
    MENU_STATE_OPTIONS,
    MENU_STATE_SAVE_LOAD,
    MENU_STATE_PLAYER_SELECT,
    MENU_STATE_SCORES,
    MENU_STATE_QUIT,
    MENU_STATE_CHAR_SELECT
} MenuState;

typedef struct {
    char name[NAME_LEN];
    int score;
} ScoreEntry;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    MenuState currentState;
    MenuState nextState;
    int running;
    Mix_Music *bgMusic;
    Mix_Chunk *hoverSound;
    int volume;
    int fullscreen;
    TTF_Font *mainFont;
    TTF_Font *titleFont;
    char playerName[NAME_LEN];
    int playerScore;
    int singlePlayer;
    int mouseX, mouseY;
    int mousePressed;
    SDL_Event event;
} MenuContext;

/* Function Prototypes */
MenuContext* menu_init(void);
void menu_cleanup(MenuContext *ctx);
void menu_update(MenuContext *ctx);
void menu_render(MenuContext *ctx);
void menu_run(MenuContext *ctx);

void render_main_menu(MenuContext *ctx);
void render_options_menu(MenuContext *ctx);
void render_save_load_menu(MenuContext *ctx);
void render_player_menu(MenuContext *ctx);
void render_scores_menu(MenuContext *ctx);

SDL_Texture* load_texture(SDL_Renderer *renderer, const char *path);
Mix_Music* load_music(const char *path);
Mix_Chunk* load_sound(const char *path);
void save_score(const char *name, int score);
int load_top_scores(ScoreEntry scores[], int maxCount);

#endif
