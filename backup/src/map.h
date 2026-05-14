#ifndef MAPP_H
#define MAPP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <math.h>

/* ── Dimensions ─────────────────────────────────────────────────────── */
#define MAP_W        1400
#define MAP_H        742
#define ZOOM_FACTOR  2.0f
#define MAX_OBS      90
#define MAX_DOORS    4
#define MAX_KEYS 4

typedef struct {
    SDL_Rect rect;
    int      id;
    int      visible;
    int      collected;
} Key;

/* ── Structs ─────────────────────────────────────────────────────────── */
#define LEVEL_1 0
#define LEVEL_2 1
typedef int LevelID;

typedef struct {
    SDL_Rect rect;
    int      locked;
    int      key_id;
    int      to_level;
} MapDoor;

#define BOX_IDLE    0
#define BOX_FALLING 1
#define BOX_BROKEN  2
typedef int BoxState;

typedef struct {
    SDL_Rect rect;
    float    fy;
    float    vel;
    BoxState state;
} FallingBox;

/* ── Map data container ──────────────────────────────────────────────── */
typedef struct {
    SDL_Texture *tex_map1;
    SDL_Texture *tex_map2;
    SDL_Texture *tex_box;
    SDL_Texture *tex_broken_box;

    LevelID      level;

    SDL_Rect     obs1[MAX_OBS];
    int          obs1_cnt;
    MapDoor      doors1[MAX_DOORS];
    int          doors1_cnt;
    int          door1_open[MAX_DOORS];
    FallingBox   fbox;

    SDL_Rect     obs2[MAX_OBS];
    int          obs2_cnt;
    MapDoor      doors2[MAX_DOORS];
    int          doors2_cnt;
    int          door2_open[MAX_DOORS];
    
    Key      keys1[MAX_KEYS];
	int      keys1_cnt;
	Key      keys2[MAX_KEYS];
	int      keys2_cnt;
    Mix_Chunk *boxFallSound;
} MapData;

/* ── Function declarations ───────────────────────────────────────────── */
void map_init(MapData *m, SDL_Renderer *renderer);
void map_cleanup(MapData *m);
void setup_level1(MapData *m);
void setup_level2(MapData *m);
void update_falling_box(MapData *m, int p1x, int p1y, int p2x, int p2y);
int  map_rects_overlap(SDL_Rect a, SDL_Rect b);

#endif /* MAPP_H */
