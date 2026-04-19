#ifndef GAME_H
#define GAME_H
#define DEBUG_HITBOXES

#include "minimap.h"
#include "map.h"
#include "enigme.h"
#include "puzzle.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BUTTON_W 130
#define BUTTON_H 40

#define WALKING_SPEED 3 
#define WALKING_FRAME_DELAY 10 

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 650
#define MAX_PLAYERS 100
#define NAME_LEN 50

#define PLAYER1_X 250
#define PLAYER1_Y 650
#define PLAYER1_W 40
#define PLAYER1_H 70

#define PLAYER2_X 300
#define PLAYER2_Y 650
#define PLAYER2_W 30
#define PLAYER2_H 70

#define PLAYER1HP_X 10
#define PLAYER1HP_Y 20
#define PLAYER1HP_W 200
#define PLAYER1HP_H 40

#define PLAYER2HP_X 780
#define PLAYER2HP_Y 20
#define PLAYER2HP_W 200
#define PLAYER2HP_H 40

//sound channels
#define CH_P1_WALK    1
#define CH_P2_WALK    2
#define CH_P1_JUMP    3
#define CH_P2_JUMP    4
#define CH_P1_ATTACK  5
#define CH_P2_ATTACK  6
#define CH_P1_GETHIT  7
#define CH_P2_GETHIT  8
#define CH_P1_DEATH   9
#define CH_P2_DEATH  10
#define CH_BUTTONS 11

#define ENEMY_FRAME_DELAY 8

typedef enum {
    STATE_CUTSCENE,
    STATE_CUTSCENE_L2_INTRO,
    STATE_CUTSCENE_L2_ENDING,
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_PAUSED_MAIN,
    STATE_PAUSED_PLAYERS,
    STATE_PAUSED_OUTFITS,
    STATE_PAUSED_CHARSELECT,
    STATE_GAME_OVER,
    STATE_ENIGME,
    STATE_PUZZLE
} GameState;

typedef struct{
  SDL_Texture *currentState;
  SDL_Texture *idle;
  SDL_Texture *walkRight[5];
  SDL_Texture *walkLeft[5];
  SDL_Texture *walkUp[2];
  SDL_Texture *walkDown[2];
  SDL_Texture *attackRight[6];
  SDL_Texture *attackLeft[6];
  SDL_Texture *hpBar[8];
  SDL_Rect rect;
  SDL_Rect healthRect;
  Mix_Chunk *walkingSound;
  Mix_Chunk *runningSound;
  Mix_Chunk *jumpingSound;
  Mix_Chunk *attackingSound;
  Mix_Chunk *gettingHitSound;
  Mix_Chunk *deathSound;


  int frame;       // 0 or 1
  int frameTimer;  // counts up each tick
  int frameDelay;  // ticks before switching frame
  int lastDir;
  int lastHDir;
  int jumping;      // 1 if currently jumping
  int jumpTimer;    // counts up during jump
  int jumpOffset;   // pixels to draw the sprite ABOVE its real position 
  int attacking;      // 1 if currently attacking
  int attackTimer;    // counts up during attack
  int attackFrame;    // current attack frame 0-5
  int baseX;
  int healthStatus;
  int alive;
  int walkToRun;
  int knockbackX;   // pixels to move per frame
  int knockbackXTimer; // how many frames left
  int speed;
  int score;
  int outfitNum;
  int outfitSwitched;
  int selectedOutfit;
  int moving;
  int selectedChar;

} Player;

typedef struct {
    SDL_Rect rect;       
    SDL_Texture* tex;
    void (*onClick)(void* data);
    int hovered;
}Button;

typedef struct{
    SDL_Rect menuBoardRect;
    SDL_Texture *menuBoard;
    SDL_Texture *playersBg;
    SDL_Texture *charSelectBg;
    SDL_Texture *p1preview;   // preview image for player 1's character
    SDL_Texture *p2preview;   // preview image for player 2's character
    Button p1o1Btn; //player 1 outfit 1
    Button p1o2Btn; //player 1 outfit 2
    Button p2o1Btn; //player 1 outfit 1
    Button p2o2Btn; //player 2 outfit 2
    Button resumeBtn;
    Button saveBtn;
    Button loadBtn;
    Button playerBtn;
    Button scoreBtn;
    Button quitBtn;
    Button charSelectBtn;
    Button outfitsBtn;
    Button buttonsBtn;
    Button okBtn;
    Button backBtn;
    Button p1SwapBtn;
    Button p2SwapBtn;
    int playerBtnSwitched;
    Mix_Chunk *hoverSound;

} subMenu;

/* ── Enemy system ── */
#define MAX_ATTACK_FRAMES  3
#define MAX_WALK_RIGHT     4
#define MAX_WALK_UP        4
#define MAX_WALK_DOWN      4
#define FRAME_DELAY        40
#define ATTACK_DELAY       40

typedef enum { DIR_DOWN=0, DIR_UP, DIR_RIGHT, DIR_LEFT } EnemyDir;
typedef enum { ANIM_IDLE=0, ANIM_WALK, ANIM_ATTACK }    EnemyAnim;

typedef struct {
    SDL_Texture *walkRight[MAX_WALK_RIGHT];
    SDL_Texture *walkUp[MAX_WALK_UP];
    SDL_Texture *walkDown[MAX_WALK_DOWN];
    SDL_Texture *attack[MAX_ATTACK_FRAMES];
} EnemyAtlas;

typedef struct {
    EnemyAnim   state;
    EnemyDir    dir;
    int         currentFrame;
    int         frameCounter;
    int         maxFrames;
    SDL_RendererFlip flip;
    int         attackFrame;
    int         attackCounter;
} EnemyAnimation;

typedef struct {
    float        x, y;
    float        targetX, targetY;
    float        speed;
    int          w, h;
    SDL_Rect     rect;
    EnemyDir     facing;
    EnemyAnimation anim;
    EnemyAtlas  *atlas;
    int          alive;
    int healthStatus;
    int maxHealth;
    float knockbackX;
    float knockbackY;
    int   knockbackTimer;
} Enemy;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    subMenu sm;
    int running;
    int volume;
    int fullscreen;
    int mousePressed;
    Player player1;
    Player player2;
    int keys[SDL_NUM_SCANCODES];
    SDL_Event event;
    TTF_Font* font;
    int paused;
    int pauseSwitched;
    GameState currentState;
    float cam1X;
    float cam2X;
    float cam1Y;
    float cam2Y;
    MiniMap minimap;   // P1 bottom-left
    MiniMap minimap2;  // P2 bottom-right
    MapData map;
    int cameraFocusTimer;     // How long the camera stays on the door (in frames)
    SDL_Point cameraOffset;   // Current x, y offset for rendering
    SDL_Point cameraTarget;   // Where we want to look (the door)
    int isCameraPanning;      // Boolean flag
    Enemy      enemy;
    EnemyAtlas enemyAtlas;
    Mix_Music *musicLevel2;
    Mix_Music *musicLevel1;
    int cutsceneTimer;
    int cutsceneAlpha;
    int cutsceneL2Timer;
    int cutsceneL2Alpha;
    Enigme en;
    PuzzleState pz;
    int lastPlayerToPickupKey;
} GameContext;

SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer);
void initOutfit(GameContext *ctx, int playerNum, int outfitNum, int charNum);
void initPlayer1(GameContext *ctx);
void initPlayer2(GameContext *ctx);
int hasIntersection(SDL_Rect r1, SDL_Rect r2);
void playerMechanics(GameContext *ctx);
SDL_Rect scale_rect(SDL_Rect rect, float scale);
int point_in_rect(int x, int y, SDL_Rect *rect);
void subMenuFn(GameContext *ctx);
void playersMenuFn(GameContext *ctx);
void changeOutfitsFn(GameContext *ctx);
void charSelectFn(GameContext *ctx);
void buttonFn(GameContext *ctx);
GameContext* game_init(void);
void game_cleanup(GameContext *ctx);
void game_update(GameContext *ctx);
void game_render(GameContext *ctx);
void game_run(GameContext *ctx);
int hasIntersection(SDL_Rect r1, SDL_Rect r2);


#endif
