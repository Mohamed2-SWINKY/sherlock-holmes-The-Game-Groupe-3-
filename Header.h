/* header.h - Game definitions, structs, and function declarations */
#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* --- Window dimensions (fixed screen size) --- */
#define WINDOW_W   1400
#define WINDOW_H    742

/* --- Map dimensions (the full world, same as original) --- */
#define MAP_W      1400
#define MAP_H       742

/* --- Camera zoom --- */
#define ZOOM_FACTOR 2.0f

/* --- Camera scroll speed (pixels per frame, in map coordinates) --- */
#define CAM_SCROLL_SPEED 5

/* --- Player --- */
#define PLAYER_W   22
#define PLAYER_H   28
#define PLAYER_SPEED 3

/* --- Falling box --- */
#define BOX_W           48
#define BOX_H           56
#define BOX_TRIGGER_DIST 130
#define BOX_FALL_SPEED   4.5f
/* Starting X,Y of the falling box (on crate pile in right dungeon area) */
#define BOX_START_X  1318
#define BOX_START_Y   258
/* Y position where the box lands on the dungeon floor and breaks */
#define BOX_LAND_Y   358

/* --- Timer format buffer size --- */
#define TIMER_BUF 16

/* --- Max counts --- */
#define MAX_OBS    90
#define MAX_KEYS    4
#define MAX_DOORS   4

/* --- Colors (RGBA) --- */
#define COL_PLAYER1  {  30, 180, 255, 200 }
#define COL_PLAYER2  { 255, 100,  40, 200 }
#define COL_KEY      { 255, 215,   0, 255 }
#define COL_DOOR_L   { 180,  60,  20, 220 }
#define COL_DOOR_O   {  20, 180,  40, 100 }

/* ======================================================
   Enumerations
   ====================================================== */

/* Game state machine */
typedef enum {
    STATE_GUIDE1,       /* Tutorial page 1 – controls          */
    STATE_GUIDE2,       /* Tutorial page 2 – objective         */
    STATE_MODE_MENU,    /* Choose mono / multi (SHIFT pressed)  */
    STATE_PLAYING
} GameState;

typedef enum { LEVEL_1, LEVEL_2 } LevelID;
typedef enum { MODE_MONO, MODE_MULTI } GameMode;

typedef enum {
    BOX_IDLE,           /* waiting for player proximity        */
    BOX_FALLING,        /* dropping downward                   */
    BOX_BROKEN          /* landed; shows broken texture        */
} BoxState;

/* ======================================================
   Structs
   ====================================================== */

/* Player (colored rectangle, no sprite) */
typedef struct {
    SDL_Rect  rect;
    int       has_key[MAX_KEYS]; /* indexed by key id */
} Player;

/* Collectible key */
typedef struct {
    SDL_Rect rect;
    int      id;        /* unique id matching door.key_id      */
    int      visible;   /* spawned on map                      */
    int      collected; /* picked up by a player               */
} Key;

/* Door / passage that can be locked */
typedef struct {
    SDL_Rect rect;
    int      locked;
    int      key_id;    /* which key unlocks this (-1 = always open) */
    int      to_level;  /* -1 = normal door, >=0 = level transition  */
} Door;

/* Falling box (map 1) */
typedef struct {
    SDL_Rect  rect;
    float     fy;       /* floating-point Y for smooth movement */
    float     vel;
    BoxState  state;
} FallingBox;

/* ======================================================
   Camera – tracks player position for 4-direction scrolling
   ====================================================== */
typedef struct {
    int x;   /* top-left corner of the camera view on the map */
    int y;
    int w;   /* viewport width  (pixels)                      */
    int h;   /* viewport height (pixels)                      */
} Camera;

/* ======================================================
   Main game context
   ====================================================== */
typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;

    /* --- Textures ---
       Set map paths in source.c setup functions:
         Map 1: assets/Gemini_Generated_Image_lvee1dlvee1dlvee.png
         Map 2: assets/Gemini_Generated_Image_9hznjl9hznjl9hzn.png  */
    SDL_Texture *tex_map1;
    SDL_Texture *tex_map2;
    SDL_Texture *tex_box;        /* assets/box.png         */
    SDL_Texture *tex_broken_box; /* assets/broken_box.png  */

    TTF_Font *font_lg;
    TTF_Font *font_sm;

    /* --- State --- */
    GameState  state;
    GameMode   mode;
    LevelID    level;

    /* --- Players --- */
    Player p1;
    Player p2;

    /* --- Level 1 data --- */
    SDL_Rect obs1[MAX_OBS];
    int      obs1_cnt;
    Key      keys1[MAX_KEYS];
    int      keys1_cnt;
    Door     doors1[MAX_DOORS];
    int      doors1_cnt;
    FallingBox fbox;

    /* --- Level 2 data --- */
    SDL_Rect obs2[MAX_OBS];
    int      obs2_cnt;
    Key      keys2[MAX_KEYS];
    int      keys2_cnt;
    Door     doors2[MAX_DOORS];
    int      doors2_cnt;

    /* --- Cameras (one per player, used for 4-direction scrolling) --- */
    Camera cam1;   /* follows p1 */
    Camera cam2;   /* follows p2 */

    /* --- Timer --- */
    Uint32 timer_start; /* SDL_GetTicks at game start */
    int    timer_on;

    /* shared door-open flags (so both players share progress) */
    int door1_open[MAX_DOORS]; /* level 1 doors */
    int door2_open[MAX_DOORS]; /* level 2 doors */
} Game;

/* ======================================================
   Function declarations (implemented in source.c)
   ====================================================== */

/* scrolling dans les quatre sens */
void camera_init(Camera *cam, int vp_w, int vp_h);
void scroll_camera_left(Camera *cam);    /* défilement gauche  */
void scroll_camera_right(Camera *cam);   /* défilement droite  */
void scroll_camera_up(Camera *cam);      /* défilement haut    */
void scroll_camera_down(Camera *cam);    /* défilement bas     */
void afficher_scrolling(Camera *cam, int left, int right, int up, int down);

/* Initialisation / teardown */
int  game_init(Game *g);
void game_cleanup(Game *g);

/* Level setup */
void setup_level1(Game *g);
void setup_level2(Game *g);

/* Input */
void input_guide(Game *g, SDL_Event *e);
void input_mode_menu(Game *g, SDL_Event *e);
void input_playing(Game *g, SDL_Event *e);

/* Update */
void update_player(Game *g, Player *p, int up, int dn, int lt, int rt);
void update_falling_box(Game *g);
void update_timer_str(Game *g, char *buf);

/* Rendering */
void render_guide(Game *g);
void render_mode_menu(Game *g);
void render_game(Game *g);

/* Helpers */
int  rects_overlap(SDL_Rect a, SDL_Rect b);
void draw_text(Game *g, TTF_Font *fnt, const char *txt,
               int x, int y, SDL_Color col);
void filled_rect(SDL_Renderer *r, SDL_Rect rect, SDL_Color col);

#endif /* GAME_H */
