#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* ─────────────────────────────────────────────
   CONSTANTS
───────────────────────────────────────────── */
#define MAX_ATTACK_FRAMES   3
#define MAX_WALK_RIGHT      4
#define MAX_WALK_UP         4
#define MAX_WALK_DOWN       4
#define MAX_HP_BARS         8
#define MAX_BUTTONS         7
#define BOARD_SCALE         2.4f
#define PLAYER_SPEED        100.0f
#define RECT_SPEED          250.0f
#define FRAME_DELAY         40
#define ATTACK_DELAY        40
#define STAR_SIZE           24      /* pixel width/height of the star */

/* ─────────────────────────────────────────────
   ENUMS
───────────────────────────────────────────── */
#define DIR_DOWN  0
#define DIR_UP    1
#define DIR_RIGHT 2
#define DIR_LEFT  3
typedef int Direction;

#define ANIM_IDLE   0
#define ANIM_WALK   1
#define ANIM_ATTACK 2
typedef int AnimState;

/* ─────────────────────────────────────────────
   STRUCTS
───────────────────────────────────────────── */

/* --- Texture Atlas --- */
typedef struct {
    SDL_Texture *walkRight[MAX_WALK_RIGHT];
    SDL_Texture *walkUp[MAX_WALK_UP];
    SDL_Texture *walkDown[MAX_WALK_DOWN];
    SDL_Texture *attack[MAX_ATTACK_FRAMES];
    SDL_Texture *hpBars[MAX_HP_BARS];
} SpriteAtlas;

/* --- Animation State --- */
typedef struct {
    AnimState   state;
    Direction   dir;
    int         currentFrame;
    int         frameCounter;
    int         maxFrames;
    SDL_RendererFlip flip;

    /* attack sub-state */
    int         attackFrame;
    int         attackCounter;
} Animation;

/* --- Enemy (the moving rectangle / AI character) --- */
typedef struct {
    float       x, y;
    float       targetX, targetY;
    float       speed;
    int         w, h;
    SDL_Rect    rect;           /* cached SDL_Rect for rendering / collision */
    Direction   facing;

    /* animation mirrors the player struct */
    Animation   anim;
    SpriteAtlas *atlas;         /* enemies share the player atlas for now */
} Enemy;

/* --- Player (the ZQSD-controlled red rectangle) --- */
typedef struct {
    float   x, y;
    int     w, h;
    SDL_Rect rect;

    /* movement flags */
    int     up, down, left, right;
    float   speed;
    SDL_Color color;
} Player;

/* --- HP / Health --- */
typedef struct {
    int     current;            /* index into hpBars[] (0 = full, 7 = dead) */
    int     max;
} Health;

/* --- Attack Projectile / Trigger --- */
typedef struct {
    int     active;
    SDL_Rect hitbox;
    int     damage;             /* HP bar steps to remove */
    Direction dir;
    float   lifetime;           /* seconds remaining */
} AttackTrigger;

/* --- Button --- */
typedef struct {
    SDL_Rect    rect;
    SDL_Texture *texture;
} Button;

/* --- Menu --- */
typedef struct {
    int         visible;
    SDL_Rect    boardRect;
    SDL_Texture *boardTexture;
    Button      buttons[MAX_BUTTONS];
} Menu;

/* --- Input snapshot --- */
typedef struct {
    int up, down, left, right;
    int escape;
    int quit;
} Input;

/* --- Star collectible --- */
typedef struct {
    SDL_Rect    rect;
    SDL_Texture *texture;   /* optional: set to NULL to draw a yellow polygon */
    int         score;      /* total stars collected this session */
} Star;

/* --- Game context (everything in one place) --- */
typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    int           screenW, screenH;
    int           running;
    int           paused;

    SpriteAtlas   atlas;
    Enemy         enemy;
    Player        player;
    Health        health;
    AttackTrigger attack;
    Menu          menu;
    Input         input;
    Star          star;
} Game;

/* ─────────────────────────────────────────────
   FUNCTION DECLARATIONS
───────────────────────────────────────────── */

/* -- utils -- */
SDL_Texture *LoadTexture(const char *path, SDL_Renderer *renderer);
float        Vec2Length(float dx, float dy);
void         Vec2Normalize(float *dx, float *dy);

/* -- game lifecycle -- */
int  Game_Init(Game *g);
void Game_Shutdown(Game *g);
void Game_Run(Game *g);

/* -- input -- */
void Input_Poll(Game *g);

/* -- enemy -- */
void Enemy_Init(Game *g);
void Enemy_Update(Enemy *e, float dt, int screenW, int screenH);
void Enemy_ChooseNewTarget(Enemy *e, int screenW, int screenH);
void Enemy_UpdateAnimation(Enemy *e, float dx, float dy);
void Enemy_Render(Game *g);

/* -- player -- */
void Player_Init(Game *g);
void Player_Update(Game *g, float dt);
void Player_Render(Game *g);

/* -- attack / transmission -- */
void Attack_Trigger(AttackTrigger *a, SDL_Rect *origin, Direction dir, int damage);
void Attack_Update(AttackTrigger *a, float dt);
void Attack_Render(Game *g);
int  Attack_CheckHit(AttackTrigger *a, SDL_Rect *target);

/* -- collision -- */
int  Collision_Check(SDL_Rect *a, SDL_Rect *b);
void Collision_ResolveEnemyPlayer(Game *g, float oldEX, float oldEY,
                                  float oldPX, float oldPY);

/* -- health -- */
void Health_Init(Health *h, int maxHP);
void Health_Damage(Health *h, int amount);
void Health_Render(Game *g);
int  Health_IsDead(Health *h);

/* -- animation -- */
void Animation_Init(Animation *a);
void Animation_Update(Animation *a, float dx, float dy,
                      SpriteAtlas *atlas);
void Animation_Render(SDL_Renderer *r, Animation *a,
                      SpriteAtlas *atlas, SDL_Rect *dst);

/* -- menu -- */
void Menu_Init(Game *g);
void Menu_ComputeLayout(Menu *m, int screenW, int screenH);
void Menu_HandleClick(Game *g, int mx, int my);
void Menu_Render(Game *g);

/* -- atlas / resource loading -- */
int  Atlas_Load(SpriteAtlas *a, SDL_Renderer *renderer);
void Atlas_Destroy(SpriteAtlas *a);

/* -- star collectible -- */
void Star_Init(Game *g);
void Star_Respawn(Game *g);
void Star_Update(Game *g);
void Star_Render(Game *g);

#endif /* GAME_H */
