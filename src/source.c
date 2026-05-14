#include "header.h"
#include "status.h"
#include "map.h"
#include "players.h"

/* ============ TEXTURE & AUDIO STORAGE ============ */
typedef struct {
    SDL_Texture *mainBg;
    SDL_Texture *optionsBg;
    SDL_Texture *gameBg;
    SDL_Texture *jouerBtn;
    SDL_Texture *optionsBtn;
    SDL_Texture *scoresBtn;
    SDL_Texture *scoresTitleImg;
    SDL_Texture *quitBtn;
    SDL_Texture *backBtn;
    SDL_Texture *yesBtn;
    SDL_Texture *noBtn;
    SDL_Texture *okBtn;
    SDL_Texture *okBtnHover;
    SDL_Texture *backBtnHover;
    SDL_Texture *exitBtn;
    SDL_Texture *exitBtnHover;
    SDL_Texture *plusBtn;
    SDL_Texture *minusBtn;
    SDL_Texture *fsBtn;
    Mix_Music *bgMusic;
    Mix_Music *gameMusic;
    Mix_Chunk *hoverSound;
    Mix_Chunk *clickSound;
    Mix_Music *scoresHoverMusic;

    /* ---- Cheour's character selection assets ---- */
    SDL_Texture *charSelecBg;
    SDL_Texture *singleTex;
    SDL_Texture *multiTex;
    SDL_Texture *charExitTex;
    SDL_Texture *charOkTex;
    SDL_Texture *sherlockFrame;
    SDL_Texture *drWatsonFrame;
    Mix_Music   *charSelecMusic;
    Mix_Chunk   *charBtnHoverSnd;
} Assets;
Assets assets = {0};
int musicPlaying = 0;

/* ---- Cheour's avatar name input state ---- */
typedef struct {
    char player1Name[50];
    char player2Name[50];
    int  activeField;   /* 0 = player1, 1 = player2, -1 = none */
} AvatarInputState;
AvatarInputState avatarInput = {
    .player1Name = "",
    .player2Name = "",
    .activeField = -1
};

/* ---- Cheour's game state (single/multi + avatar select) ---- */
state charState = { .single_multi = 1, .avatar_select = 0 };

/* ============ BUTTON RECTANGLES ============ */
SDL_Rect jouerRect    = {50, 300, 200, 80};
SDL_Rect optionsRect  = {50, 380, 200, 80};
SDL_Rect scoresRect   = {50, 460, 200, 80};
SDL_Rect quitRect     = {50, 540, 200, 80};
SDL_Rect plusRect     = {600, 250, 60, 30};
SDL_Rect minusRect    = {500, 250, 60, 30};
SDL_Rect fsRect       = {530, 290, 150, 40};
SDL_Rect backRect     = {50, 510, 200, 80};
SDL_Rect yesRect      = {200, 300, 200, 80};
SDL_Rect noRect       = {600, 300, 200, 80};
SDL_Rect singleRect   = {200, 250, 200, 80};
SDL_Rect multiRect    = {200, 350, 200, 80};

SDL_Rect scoresOkRect   = {300, 400, 200, 60};
SDL_Rect scoresBackRect = {300, 470, 200, 60};

/* Fullscreen positions */
SDL_Rect jouerRectFS    = {96,  498,  384, 132};
SDL_Rect optionsRectFS  = {96,  631,  384, 132};
SDL_Rect scoresRectFS   = {96,  764,  384, 132};
SDL_Rect quitRectFS     = {96,  897,  384, 132};
SDL_Rect plusRectFS     = {1152, 415,  115, 49};
SDL_Rect minusRectFS    = {960,415,  115, 49};
SDL_Rect fsRectFS       = {1017,481,  288, 66};
SDL_Rect backRectFS     = {96,  847,  384, 132};
SDL_Rect yesRectFS      = {384, 498,  384, 132};
SDL_Rect noRectFS       = {1152,498,  384, 132};
SDL_Rect singleRectFS   = {384, 415,  384, 132};
SDL_Rect multiRectFS    = {384, 581,  384, 132};

SDL_Rect scoresOkRectFS   = {576, 664, 384, 99};
SDL_Rect scoresBackRectFS = {576, 781, 384, 99};

/* Cheour's char-selec button rects — windowed */
static SDL_Rect charSingleRect   = {50, 300, 200, 80};
static SDL_Rect charMultiRect    = {50, 380, 200, 80};
static SDL_Rect charExitRect     = {50, 540, 200, 80};
static SDL_Rect charOkRect       = {740, 540, 200, 80};

/* Cheour's char-selec button rects — fullscreen */
static SDL_Rect charSingleRectFS = {96, 498, 384, 132};
static SDL_Rect charMultiRectFS  = {96, 631, 384, 132};
static SDL_Rect charExitRectFS   = {96, 897, 384, 132};
static SDL_Rect charOkRectFS     = {1440, 897, 384, 132};

/* Pointers that swap on fullscreen toggle */
static SDL_Rect *currentCharSingleRect = &charSingleRect;
static SDL_Rect *currentCharMultiRect  = &charMultiRect;
static SDL_Rect *currentCharExitRect   = &charExitRect;
static SDL_Rect *currentCharOkRect     = &charOkRect;

/* Current active rectangles */
SDL_Rect *currentJouerRect   = &jouerRect;
SDL_Rect *currentOptionsRect = &optionsRect;
SDL_Rect *currentScoresRect  = &scoresRect;
SDL_Rect *currentQuitRect    = &quitRect;
SDL_Rect *currentPlusRect    = &plusRect;
SDL_Rect *currentMinusRect   = &minusRect;
SDL_Rect *currentFsRect      = &fsRect;
SDL_Rect *currentBackRect    = &backRect;
SDL_Rect *currentYesRect     = &yesRect;
SDL_Rect *currentNoRect      = &noRect;
SDL_Rect *currentSingleRect  = &singleRect;
SDL_Rect *currentMultiRect   = &multiRect;
SDL_Rect *currentScoresOkRect   = &scoresOkRect;
SDL_Rect *currentScoresBackRect = &scoresBackRect;

int hoveredButton = -1;
int previousHoveredButton = -1;
#define SCALE_FACTOR 1.15f

/* ============ UTILITY FUNCTIONS ============ */
SDL_Rect menu_scale_rect(SDL_Rect *rect, float scale)
{
    SDL_Rect scaled;
    float w = rect->w, h = rect->h;
    float sw = w * scale, sh = h * scale;
    scaled.w = (int)sw;
    scaled.h = (int)sh;
    scaled.x = rect->x + (int)((w - sw) / 2);
    scaled.y = rect->y + (int)((h - sh) / 2);
    return scaled;
}

void update_button_positions(int fullscreen)
{
    if (fullscreen) {
        currentJouerRect   = &jouerRectFS;
        currentOptionsRect = &optionsRectFS;
        currentScoresRect  = &scoresRectFS;
        currentQuitRect    = &quitRectFS;
        currentPlusRect    = &plusRectFS;
        currentMinusRect   = &minusRectFS;
        currentFsRect      = &fsRectFS;
        currentBackRect    = &backRectFS;
        currentYesRect     = &yesRectFS;
        currentNoRect      = &noRectFS;
        currentSingleRect  = &singleRectFS;
        currentMultiRect   = &multiRectFS;
        currentScoresOkRect   = &scoresOkRectFS;
        currentScoresBackRect = &scoresBackRectFS;
        currentCharSingleRect = &charSingleRectFS;
        currentCharMultiRect  = &charMultiRectFS;
        currentCharExitRect   = &charExitRectFS;
        currentCharOkRect     = &charOkRectFS;
        printf("Switched to FULLSCREEN button positions\n");
    } else {
        currentJouerRect   = &jouerRect;
        currentOptionsRect = &optionsRect;
        currentScoresRect  = &scoresRect;
        currentQuitRect    = &quitRect;
        currentPlusRect    = &plusRect;
        currentMinusRect   = &minusRect;
        currentFsRect      = &fsRect;
        currentBackRect    = &backRect;
        currentYesRect     = &yesRect;
        currentNoRect      = &noRect;
        currentSingleRect  = &singleRect;
        currentMultiRect   = &multiRect;
        currentScoresOkRect   = &scoresOkRect;
        currentScoresBackRect = &scoresBackRect;
        currentCharSingleRect = &charSingleRect;
        currentCharMultiRect  = &charMultiRect;
        currentCharExitRect   = &charExitRect;
        currentCharOkRect     = &charOkRect;
        printf("Switched to WINDOWED button positions\n");
    }
}

void change_music(Mix_Music *newMusic, const char *musicName)
{
    if (newMusic) {
        Mix_HaltMusic();
        if (Mix_PlayMusic(newMusic, -1) == 0) {
            musicPlaying = 1;
            printf("Switched to music: %s\n", musicName);
        } else {
            printf("ERROR: Could not play music: %s\n", Mix_GetError());
        }
    }
}

SDL_Texture* load_texture(SDL_Renderer *renderer, const char *path)
{
    SDL_Surface *surface = IMG_Load(path);
    if (!surface) { printf("Cannot load texture: %s\n", path); return NULL; }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

Mix_Music* load_music(const char *path)
{
    Mix_Music *music = Mix_LoadMUS(path);
    if (!music) printf("ERROR loading music %s: %s\n", path, Mix_GetError());
    else        printf("Music loaded: %s\n", path);
    return music;
}

Mix_Chunk* load_sound(const char *path)
{
    Mix_Chunk *chunk = Mix_LoadWAV(path);
    if (!chunk) printf("ERROR loading sound %s: %s\n", path, Mix_GetError());
    else        printf("Sound loaded: %s\n", path);
    return chunk;
}

int menu_point_in_rect(int x, int y, SDL_Rect *rect)
{
    return x >= rect->x && x <= rect->x + rect->w &&
           y >= rect->y && y <= rect->y + rect->h;
}

int isInside(Button *b, int mx, int my)
{
    return mx >= b->rect.x && mx <= b->rect.x + b->rect.w &&
           my >= b->rect.y && my <= b->rect.y + b->rect.h;
}

/* ============ ASSET LOADING ============ */
void load_assets(SDL_Renderer *renderer)
{
    assets.mainBg    = load_texture(renderer, "assets/backgrounds/Menu_background.jpg");
    assets.optionsBg = load_texture(renderer, "assets/backgrounds/optionsBg.jpg");
    assets.gameBg    = load_texture(renderer, "assets/backgrounds/game_background.jpg");

    assets.jouerBtn   = load_texture(renderer, "assets/buttons/jouer.png");
    assets.optionsBtn = load_texture(renderer, "assets/buttons/options.png");
    assets.scoresBtn  = load_texture(renderer, "assets/buttons/scores.png");
    assets.quitBtn    = load_texture(renderer, "assets/buttons/quitter.png");

    assets.scoresTitleImg = load_texture(renderer, "assets/backgrounds/saisir_nom.png");
    assets.okBtn          = load_texture(renderer, "assets/buttons/ok.png");
    assets.okBtnHover     = load_texture(renderer, "assets/buttons/okHover.png");
    assets.exitBtn        = load_texture(renderer, "assets/buttons/quitter.png");
    assets.exitBtnHover   = load_texture(renderer, "assets/buttons/quitterHover.png");

    assets.backBtn      = load_texture(renderer, "assets/buttons/back.png");
    assets.backBtnHover = load_texture(renderer, "assets/buttons/backHover.png");
    assets.yesBtn       = load_texture(renderer, "assets/buttons/yes.png");
    assets.noBtn        = load_texture(renderer, "assets/buttons/no.png");
    assets.plusBtn      = load_texture(renderer, "assets/buttons/Augmenter.png");
    assets.minusBtn     = load_texture(renderer, "assets/buttons/Diminuer.png");
    assets.fsBtn        = load_texture(renderer, "assets/buttons/fullscreen.png");

    /* ---- Cheour's assets ---- */
    assets.charSelecBg   = load_texture(renderer, "assets/backgrounds/game_background.jpg");
    assets.singleTex     = load_texture(renderer, "assets/buttons/singleplayer.png");
    assets.multiTex      = load_texture(renderer, "assets/buttons/multiplayer.png");
    assets.charExitTex   = load_texture(renderer, "assets/buttons/back.png");
    assets.charOkTex     = load_texture(renderer, "assets/buttons/ok.png");
    assets.sherlockFrame = load_texture(renderer, "assets/backgrounds/sherlock_character_frame.png");
    assets.drWatsonFrame = load_texture(renderer, "assets/backgrounds/dr_watson_character_frame.png");
    assets.charSelecMusic  = load_music("assets/music/music2.mp3");
    assets.charBtnHoverSnd = load_sound("assets/music/buttonHover.wav");

    assets.bgMusic = load_music("assets/music/music.mp3");
    Mix_Volume(-1, 128);
    if (!assets.bgMusic) assets.bgMusic = load_music("assets/music/bgm.mp3");
    assets.gameMusic = load_music("assets/music/music3.mp3");
    if (!assets.gameMusic) assets.gameMusic = assets.bgMusic;
    assets.scoresHoverMusic = load_music("assets/music/buttonHover.mp3");

    assets.hoverSound = load_sound("assets/music/hover.wav");
    assets.clickSound = load_sound("assets/music/buttonHover.wav");

    Mix_AllocateChannels(8);

    if (assets.bgMusic) {
        Mix_VolumeMusic(90);
        if (Mix_PlayMusic(assets.bgMusic, -1) == 0) {
            musicPlaying = 1;
            printf("Background music is PLAYING\n");
        }
    }
}

void menu_play_hover_sound()
{
    if (assets.hoverSound) {
        Mix_HaltChannel(1);
        Mix_Volume(1, 100);
        Mix_PlayChannel(1, assets.hoverSound, 0);
    }
}

void play_click_sound()
{
    if (assets.clickSound) {
        Mix_HaltChannel(2);
        Mix_Volume(2, 100);
        Mix_PlayChannel(2, assets.clickSound, 0);
    }
}

/* ============ CHEOUR'S CHAR-SELEC RENDER HELPERS ============ */
static void render_char_single_multi(SDL_Renderer *ren, int hovered)
{
    SDL_Rect d;
    d = (hovered == 0) ? menu_scale_rect(currentCharSingleRect, SCALE_FACTOR) : *currentCharSingleRect;
    if (assets.singleTex) SDL_RenderCopy(ren, assets.singleTex, NULL, &d);

    d = (hovered == 1) ? menu_scale_rect(currentCharMultiRect, SCALE_FACTOR) : *currentCharMultiRect;
    if (assets.multiTex) SDL_RenderCopy(ren, assets.multiTex, NULL, &d);

    d = (hovered == 2) ? menu_scale_rect(currentCharExitRect, SCALE_FACTOR) : *currentCharExitRect;
    if (assets.charExitTex) SDL_RenderCopy(ren, assets.charExitTex, NULL, &d);
}

/* ---------------------------------------------------------------------------
 * render_char_avatar_select
 *
 * Text boxes match the scores menu style exactly:
 *   - Black fill, coloured border (blue when active, white when inactive)
 *   - "Player 1 Name:" / "Player 2 Name:" labels above each box
 *   - Blinking "|" cursor in the active box
 *   - Text clamped so it never overflows
 * --------------------------------------------------------------------------- */
static void render_char_avatar_select(SDL_Renderer *ren, TTF_Font *font,
                                      int hovered, int winW, int winH)
{
    /* ---- Back / exit button ---- */
    SDL_Rect d = (hovered == 0) ? menu_scale_rect(currentCharExitRect, SCALE_FACTOR)
                                : *currentCharExitRect;
    if (assets.charExitTex) SDL_RenderCopy(ren, assets.charExitTex, NULL, &d);

    /* ---- OK button ---- */
    SDL_Rect dok = (hovered == 1) ? menu_scale_rect(currentCharOkRect, SCALE_FACTOR)
                                  : *currentCharOkRect;
    if (assets.charOkTex) SDL_RenderCopy(ren, assets.charOkTex, NULL, &dok);

    /* ---- Frame layout ---- */
    int frameW = (int)(winW * 0.22f);
    int frameH = (int)(winH * 0.45f);
    int gap    = (int)(winW * 0.04f);
    int totalW = frameW * 2 + gap;
    int startX = (winW - totalW) / 2;
    int frameY = (int)(winH * 0.13f) + 75;

    SDL_Rect sherlockR = { startX,             frameY, frameW, frameH };
    SDL_Rect drWatsonR = { startX + frameW + gap, frameY, frameW, frameH };

    if (assets.sherlockFrame) SDL_RenderCopy(ren, assets.sherlockFrame, NULL, &sherlockR);
    if (assets.drWatsonFrame) SDL_RenderCopy(ren, assets.drWatsonFrame, NULL, &drWatsonR);

    /* ---- Name input boxes — scores-menu style ---- */
    int boxH    = (int)(winH * 0.06f);
    int boxY    = frameY + frameH + (int)(winH * 0.03f);
    int padding = (int)(winW * 0.005f);

    SDL_Rect box1 = { sherlockR.x,  boxY, frameW, boxH };
    SDL_Rect box2 = { drWatsonR.x,  boxY, frameW, boxH };

    SDL_Color white       = {255, 255, 255, 255};
    SDL_Color activeColor = {100, 200, 255, 255};

    /* Fill both boxes black */
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderFillRect(ren, &box1);
    SDL_RenderFillRect(ren, &box2);

    /* Border: blue if active, white if not */
    SDL_SetRenderDrawColor(ren,
        avatarInput.activeField == 0 ? activeColor.r : white.r,
        avatarInput.activeField == 0 ? activeColor.g : white.g,
        avatarInput.activeField == 0 ? activeColor.b : white.b, 255);
    SDL_RenderDrawRect(ren, &box1);

    SDL_SetRenderDrawColor(ren,
        avatarInput.activeField == 1 ? activeColor.r : white.r,
        avatarInput.activeField == 1 ? activeColor.g : white.g,
        avatarInput.activeField == 1 ? activeColor.b : white.b, 255);
    SDL_RenderDrawRect(ren, &box2);

    if (!font) return;

    /* Helper: render label above box, then typed text (+ cursor) inside box */
    struct { SDL_Rect *box; const char *label; const char *text; int field; } fields[2] = {
        { &box1, "Player 1 Name:", avatarInput.player1Name, 0 },
        { &box2, "Player 2 Name:", avatarInput.player2Name, 1 },
    };
}

/* ============ RENDERING FUNCTIONS ============ */
void render_main_menu(MenuContext *ctx)
{
    if (assets.mainBg) SDL_RenderCopy(ctx->renderer, assets.mainBg, NULL, NULL);
    else { SDL_SetRenderDrawColor(ctx->renderer, 0,0,0,255); SDL_RenderClear(ctx->renderer); }

    if (assets.jouerBtn)   { SDL_Rect d=(hoveredButton==0)?menu_scale_rect(currentJouerRect,  SCALE_FACTOR):*currentJouerRect;   SDL_RenderCopy(ctx->renderer,assets.jouerBtn,  NULL,&d); }
    if (assets.optionsBtn) { SDL_Rect d=(hoveredButton==1)?menu_scale_rect(currentOptionsRect,SCALE_FACTOR):*currentOptionsRect; SDL_RenderCopy(ctx->renderer,assets.optionsBtn,NULL,&d); }
    if (assets.scoresBtn)  { SDL_Rect d=(hoveredButton==2)?menu_scale_rect(currentScoresRect, SCALE_FACTOR):*currentScoresRect;  SDL_RenderCopy(ctx->renderer,assets.scoresBtn, NULL,&d); }
    if (assets.quitBtn)    { SDL_Rect d=(hoveredButton==3)?menu_scale_rect(currentQuitRect,   SCALE_FACTOR):*currentQuitRect;    SDL_RenderCopy(ctx->renderer,assets.quitBtn,   NULL,&d); }
}

void render_options_menu(MenuContext *ctx)
{
    if (assets.optionsBg) SDL_RenderCopy(ctx->renderer, assets.optionsBg, NULL, NULL);
    else { SDL_SetRenderDrawColor(ctx->renderer,0,0,0,255); SDL_RenderClear(ctx->renderer); }

    if (assets.plusBtn)  { SDL_Rect d=(hoveredButton==0)?menu_scale_rect(currentPlusRect, SCALE_FACTOR):*currentPlusRect;  SDL_RenderCopy(ctx->renderer,assets.plusBtn, NULL,&d); }
    if (assets.minusBtn) { SDL_Rect d=(hoveredButton==1)?menu_scale_rect(currentMinusRect,SCALE_FACTOR):*currentMinusRect; SDL_RenderCopy(ctx->renderer,assets.minusBtn,NULL,&d); }
    if (assets.fsBtn)    { SDL_Rect d=(hoveredButton==2)?menu_scale_rect(currentFsRect,   SCALE_FACTOR):*currentFsRect;    SDL_RenderCopy(ctx->renderer,assets.fsBtn,   NULL,&d); }
    if (assets.backBtn)  { SDL_Rect d=(hoveredButton==3)?menu_scale_rect(currentBackRect, SCALE_FACTOR):*currentBackRect;  SDL_RenderCopy(ctx->renderer,assets.backBtn, NULL,&d); }
}

void render_save_load_menu(MenuContext *ctx)
{
    if (assets.gameBg) SDL_RenderCopy(ctx->renderer, assets.gameBg, NULL, NULL);
    else { SDL_SetRenderDrawColor(ctx->renderer,0,0,0,255); SDL_RenderClear(ctx->renderer); }

    int winW, winH;
    SDL_GetWindowSize(ctx->window, &winW, &winH);

    if (ctx->titleFont) {
        SDL_Color gold = {212,175,55,255};
        SDL_Surface *s = TTF_RenderText_Solid(ctx->titleFont, "SAVE/LOAD", gold);
        if (s) {
            SDL_Texture *t = SDL_CreateTextureFromSurface(ctx->renderer, s);
            SDL_Rect d = {(winW-s->w)/2, ctx->fullscreen?300:200, s->w, s->h};
            SDL_FreeSurface(s);
            if (t) { SDL_RenderCopy(ctx->renderer,t,NULL,&d); SDL_DestroyTexture(t); }
        }
    }
    if (assets.yesBtn) { SDL_Rect d=(hoveredButton==0)?menu_scale_rect(currentYesRect, SCALE_FACTOR):*currentYesRect; SDL_RenderCopy(ctx->renderer,assets.yesBtn,NULL,&d); }
    if (assets.noBtn)  { SDL_Rect d=(hoveredButton==1)?menu_scale_rect(currentNoRect,  SCALE_FACTOR):*currentNoRect;  SDL_RenderCopy(ctx->renderer,assets.noBtn, NULL,&d); }
    if (assets.backBtn){ SDL_Rect d=(hoveredButton==2)?menu_scale_rect(currentBackRect,SCALE_FACTOR):*currentBackRect;SDL_RenderCopy(ctx->renderer,assets.backBtn,NULL,&d); }
}

/* ---- Cheour's character selection screen ---- */
void render_char_selec_menu(MenuContext *ctx)
{
    if (assets.charSelecBg) SDL_RenderCopy(ctx->renderer, assets.charSelecBg, NULL, NULL);
    else { SDL_SetRenderDrawColor(ctx->renderer, 20, 20, 40, 255); SDL_RenderClear(ctx->renderer); }

    int winW, winH;
    SDL_GetWindowSize(ctx->window, &winW, &winH);

    if (charState.single_multi == 1)
        render_char_single_multi(ctx->renderer, hoveredButton);
    else
        render_char_avatar_select(ctx->renderer, ctx->mainFont, hoveredButton, winW, winH);
}

void render_player_menu(MenuContext *ctx)
{
    if (assets.gameBg) SDL_RenderCopy(ctx->renderer, assets.gameBg, NULL, NULL);
    else { SDL_SetRenderDrawColor(ctx->renderer,0,0,0,255); SDL_RenderClear(ctx->renderer); }

    int winW, winH;
    SDL_GetWindowSize(ctx->window, &winW, &winH);

    if (ctx->titleFont) {
        SDL_Color gold = {212,175,55,255};
        SDL_Surface *s = TTF_RenderText_Solid(ctx->titleFont, "PLAYER SELECTION", gold);
        if (s) {
            SDL_Texture *t = SDL_CreateTextureFromSurface(ctx->renderer, s);
            SDL_Rect d = {(winW-s->w)/2, ctx->fullscreen?120:80, s->w, s->h};
            SDL_FreeSurface(s);
            if (t) { SDL_RenderCopy(ctx->renderer,t,NULL,&d); SDL_DestroyTexture(t); }
        }
    }
    if (ctx->mainFont) {
        SDL_Color white = {255,255,255,255};
        SDL_Surface *s1 = TTF_RenderText_Solid(ctx->mainFont, "[S] SINGLE PLAYER", white);
        if (s1) {
            SDL_Texture *t1 = SDL_CreateTextureFromSurface(ctx->renderer, s1);
            SDL_Rect d1 = {ctx->fullscreen?300:200, ctx->fullscreen?375:250, s1->w, s1->h};
            SDL_FreeSurface(s1);
            if (t1) { SDL_RenderCopy(ctx->renderer,t1,NULL,&d1); SDL_DestroyTexture(t1); }
        }
        SDL_Surface *s2 = TTF_RenderText_Solid(ctx->mainFont, "[P] MULTIPLAYER", white);
        if (s2) {
            SDL_Texture *t2 = SDL_CreateTextureFromSurface(ctx->renderer, s2);
            SDL_Rect d2 = {ctx->fullscreen?300:200, ctx->fullscreen?525:350, s2->w, s2->h};
            SDL_FreeSurface(s2);
            if (t2) { SDL_RenderCopy(ctx->renderer,t2,NULL,&d2); SDL_DestroyTexture(t2); }
        }
    }
}

void render_scores_menu(MenuContext *ctx)
{
    ClassementTB classement;
    tb_charger(&classement);

    // Convert MenuContext → GameContext-like renderer/font usage
    GameContext fakeCtx;
    fakeCtx.renderer = ctx->renderer;
    fakeCtx.font = ctx->mainFont;

    // draw background like map style
    tb_fond(ctx->renderer);

    // display scoreboard exactly like map.c
    tb_afficher_classement(&fakeCtx, &classement);

    // BACK button
    if (assets.backBtn) {
        SDL_Rect d = (hoveredButton == 1)
            ? menu_scale_rect(&backRect, SCALE_FACTOR)
            : backRect;

        SDL_RenderCopy(ctx->renderer, assets.backBtn, NULL, &d);
    }
}


/* ============ CORE GAME ============ */
MenuContext* menu_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError()); return NULL;
    }
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    int flags = MIX_INIT_MP3;
    if ((Mix_Init(flags) & flags) != flags)
        printf("Warning: SDL_mixer audio limited: %s\n", Mix_GetError());
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        printf("ERROR: Cannot open audio: %s\n", Mix_GetError());
    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError()); return NULL;
    }

    MenuContext *ctx = malloc(sizeof(MenuContext));
    if (!ctx) { fprintf(stderr, "Out of memory\n"); return NULL; }

    ctx->window = SDL_CreateWindow("Sherlock Holmes: The Unseen Enemy",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!ctx->window) { fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); free(ctx); return NULL; }

    ctx->renderer = SDL_CreateRenderer(ctx->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ctx->renderer) { fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError()); SDL_DestroyWindow(ctx->window); free(ctx); return NULL; }

    ctx->currentState = MENU_STATE_MAIN_MENU;
    ctx->nextState    = MENU_STATE_MAIN_MENU;
    ctx->running    = 1;
    ctx->volume     = 64;
    ctx->fullscreen = 0;

    ctx->mainFont  = TTF_OpenFont("assets/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    ctx->titleFont = TTF_OpenFont("assets/fonts/truetype/dejavu/DejaVuSans.ttf", 48);
    if (!ctx->mainFont)  printf("ERROR: Could not load main font!\n");
    if (!ctx->titleFont) printf("ERROR: Could not load title font!\n");

    ctx->bgMusic    = NULL;
    ctx->hoverSound = NULL;

    printf("Loading assets...\n");
    load_assets(ctx->renderer);

    memset(avatarInput.player1Name, 0, sizeof(avatarInput.player1Name));
    memset(avatarInput.player2Name, 0, sizeof(avatarInput.player2Name));
    avatarInput.activeField = -1;

    charState.single_multi  = 1;
    charState.avatar_select = 0;

    update_button_positions(0);

    return ctx;
}

void menu_cleanup(MenuContext *ctx)
{
    SDL_StopTextInput();
    if (ctx->mainFont)  TTF_CloseFont(ctx->mainFont);
    if (ctx->titleFont) TTF_CloseFont(ctx->titleFont);
    if (assets.bgMusic)  { Mix_HaltMusic(); Mix_FreeMusic(assets.bgMusic);  assets.bgMusic=NULL; }
    if (assets.gameMusic && assets.gameMusic != assets.bgMusic) { Mix_FreeMusic(assets.gameMusic); assets.gameMusic=NULL; }
    if (assets.charSelecMusic) { Mix_FreeMusic(assets.charSelecMusic); assets.charSelecMusic=NULL; }
    if (assets.scoresHoverMusic) Mix_FreeMusic(assets.scoresHoverMusic);
    if (assets.hoverSound)     Mix_FreeChunk(assets.hoverSound);
    if (assets.clickSound)     Mix_FreeChunk(assets.clickSound);
    if (assets.charBtnHoverSnd) Mix_FreeChunk(assets.charBtnHoverSnd);

    SDL_Texture **texList[] = {
        &assets.mainBg,&assets.optionsBg,&assets.gameBg,&assets.scoresTitleImg,
        &assets.okBtn,&assets.okBtnHover,&assets.exitBtn,&assets.exitBtnHover,
        &assets.backBtnHover,&assets.charSelecBg,&assets.singleTex,&assets.multiTex,
        &assets.charExitTex,&assets.charOkTex,&assets.sherlockFrame,&assets.drWatsonFrame,NULL
    };
    for (int i=0; texList[i]; i++)
        if (*texList[i]) { SDL_DestroyTexture(*texList[i]); *texList[i]=NULL; }

    SDL_DestroyRenderer(ctx->renderer);
    SDL_DestroyWindow(ctx->window);
    Mix_CloseAudio(); Mix_Quit(); TTF_Quit(); IMG_Quit(); SDL_Quit();
    free(ctx);
}

void menu_update(MenuContext *ctx)
{
    static int lastHovered = -1;
    int currentlyHovered = -1;
    ctx->currentState = ctx->nextState;
    previousHoveredButton = hoveredButton;

    int windowWidth, windowHeight;
    SDL_GetWindowSize(ctx->window, &windowWidth, &windowHeight);

    while (SDL_PollEvent(&ctx->event)) {
        if (ctx->event.type == SDL_QUIT) { ctx->running=0; }

        /* ---- Mouse motion (hover) ---- */
        if (ctx->event.type == SDL_MOUSEMOTION) {
            int x=ctx->event.motion.x, y=ctx->event.motion.y;
            currentlyHovered=-1;

            if (ctx->currentState==MENU_STATE_MAIN_MENU) {
                if      (menu_point_in_rect(x,y,currentJouerRect))   currentlyHovered=0;
                else if (menu_point_in_rect(x,y,currentOptionsRect))  currentlyHovered=1;
                else if (menu_point_in_rect(x,y,currentScoresRect))   currentlyHovered=2;
                else if (menu_point_in_rect(x,y,currentQuitRect))     currentlyHovered=3;
            } else if (ctx->currentState==MENU_STATE_OPTIONS) {
                if      (menu_point_in_rect(x,y,currentPlusRect))    currentlyHovered=0;
                else if (menu_point_in_rect(x,y,currentMinusRect))   currentlyHovered=1;
                else if (menu_point_in_rect(x,y,currentFsRect))      currentlyHovered=2;
                else if (menu_point_in_rect(x,y,currentBackRect))    currentlyHovered=3;
            } else if (ctx->currentState==MENU_STATE_SAVE_LOAD) {
                if      (menu_point_in_rect(x,y,currentYesRect))     currentlyHovered=0;
                else if (menu_point_in_rect(x,y,currentNoRect))      currentlyHovered=1;
                else if (menu_point_in_rect(x,y,currentBackRect))    currentlyHovered=2;
            } else if (ctx->currentState==MENU_STATE_PLAYER_SELECT) {
                if      (menu_point_in_rect(x,y,currentSingleRect))  currentlyHovered=0;
                else if (menu_point_in_rect(x,y,currentMultiRect))   currentlyHovered=1;
                else if (menu_point_in_rect(x,y,currentBackRect))    currentlyHovered=2;
            } else if (ctx->currentState==MENU_STATE_CHAR_SELECT) {
                if (charState.single_multi==1) {
                    if      (menu_point_in_rect(x,y,currentCharSingleRect)) currentlyHovered=0;
                    else if (menu_point_in_rect(x,y,currentCharMultiRect))  currentlyHovered=1;
                    else if (menu_point_in_rect(x,y,currentCharExitRect))   currentlyHovered=2;
                } else {
                    if      (menu_point_in_rect(x,y,currentCharExitRect)) currentlyHovered=0;
                    else if (menu_point_in_rect(x,y,currentCharOkRect))   currentlyHovered=1;
                }
            }

            if (currentlyHovered!=lastHovered && currentlyHovered!=-1)
                menu_play_hover_sound();
            hoveredButton=currentlyHovered;
            lastHovered=currentlyHovered;
        }

        /* ---- Mouse click ---- */
        else if (ctx->event.type == SDL_MOUSEBUTTONDOWN) {
            int x=ctx->event.button.x, y=ctx->event.button.y;

            if (ctx->currentState==MENU_STATE_MAIN_MENU) {
                if      (menu_point_in_rect(x,y,currentJouerRect))   { printf("CLICKED: Play\n");    play_click_sound(); if(assets.gameMusic)change_music(assets.gameMusic,"music3.mp3"); ctx->nextState=MENU_STATE_SAVE_LOAD; }
                else if (menu_point_in_rect(x,y,currentOptionsRect)) { printf("CLICKED: Options\n"); play_click_sound(); if(assets.gameMusic)change_music(assets.gameMusic,"music3.mp3"); ctx->nextState=MENU_STATE_OPTIONS; }
                else if (menu_point_in_rect(x,y,currentScoresRect)) {
    printf("CLICKED: Scores\n");
    play_click_sound();
    ctx->nextState = MENU_STATE_SCORES;
}
                else if (menu_point_in_rect(x,y,currentQuitRect))    { printf("CLICKED: Quit\n");    play_click_sound(); ctx->running=0; }
            }
            else if (ctx->currentState==MENU_STATE_OPTIONS) {
                if      (menu_point_in_rect(x,y,currentPlusRect))  { ctx->volume=SDL_min(ctx->volume+10,128); play_click_sound(); printf("Volume: %d\n",ctx->volume); }
                else if (menu_point_in_rect(x,y,currentMinusRect)) { ctx->volume=SDL_max(ctx->volume-10,0);   play_click_sound(); printf("Volume: %d\n",ctx->volume); }
                else if (menu_point_in_rect(x,y,currentFsRect))    { ctx->fullscreen=!ctx->fullscreen; play_click_sound(); SDL_SetWindowFullscreen(ctx->window,ctx->fullscreen?SDL_WINDOW_FULLSCREEN_DESKTOP:0); update_button_positions(ctx->fullscreen); }
                else if (menu_point_in_rect(x,y,currentBackRect))  { printf("CLICKED: Back\n"); play_click_sound(); if(assets.bgMusic)change_music(assets.bgMusic,"music.mp3"); ctx->nextState=MENU_STATE_MAIN_MENU; }
            }
            else if (ctx->currentState==MENU_STATE_SAVE_LOAD) {
                if      (menu_point_in_rect(x,y,currentYesRect))  { printf("CLICKED: Yes\n"); play_click_sound(); charState.single_multi=1; charState.avatar_select=0; if(assets.charSelecMusic)change_music(assets.charSelecMusic,"music2.mp3"); ctx->nextState=MENU_STATE_CHAR_SELECT; }
                else if (menu_point_in_rect(x,y,currentNoRect))   { printf("CLICKED: No\n");  play_click_sound(); charState.single_multi=1; charState.avatar_select=0; if(assets.charSelecMusic)change_music(assets.charSelecMusic,"music2.mp3"); ctx->nextState=MENU_STATE_CHAR_SELECT; }
                else if (menu_point_in_rect(x,y,currentBackRect)) { printf("CLICKED: Back\n"); play_click_sound(); if(assets.bgMusic)change_music(assets.bgMusic,"music.mp3"); ctx->nextState=MENU_STATE_MAIN_MENU; }
            }
            else if (ctx->currentState==MENU_STATE_PLAYER_SELECT) {
                if      (menu_point_in_rect(x,y,currentSingleRect)) { printf("Single\n"); play_click_sound(); }
                else if (menu_point_in_rect(x,y,currentMultiRect))  { printf("Multi\n");  play_click_sound(); }
                else if (menu_point_in_rect(x,y,currentBackRect))   { printf("Back\n");   play_click_sound(); if(assets.bgMusic)change_music(assets.bgMusic,"music.mp3"); ctx->nextState=MENU_STATE_MAIN_MENU; }
            }
            else if (ctx->currentState==MENU_STATE_CHAR_SELECT) {
                if (charState.single_multi==1) {
                    if      (menu_point_in_rect(x,y,currentCharSingleRect)) { printf("Single player selected\n"); play_click_sound(); charState.single_multi=0; charState.avatar_select=1; avatarInput.activeField=0; }
                    else if (menu_point_in_rect(x,y,currentCharMultiRect))  { printf("Multiplayer selected\n");   play_click_sound(); charState.single_multi=0; charState.avatar_select=1; avatarInput.activeField=0; }
                    else if (menu_point_in_rect(x,y,currentCharExitRect))   { printf("Exit char select\n");       play_click_sound(); if(assets.bgMusic)change_music(assets.bgMusic,"music.mp3"); ctx->nextState=MENU_STATE_MAIN_MENU; }
                } else {
                    if      (menu_point_in_rect(x,y,currentCharExitRect)) { printf("Back to single/multi\n"); play_click_sound(); charState.single_multi=1; charState.avatar_select=0; avatarInput.activeField=-1; }
                    else if (menu_point_in_rect(x,y,currentCharOkRect))   { printf("OK\n"); play_click_sound(); ctx->running=0;}
                }
            }
        }

        /* ---- Keyboard ---- */
        else if (ctx->event.type == SDL_KEYDOWN) {
            int key=ctx->event.key.keysym.sym;
            if (key==SDLK_ESCAPE) {
                if      (ctx->currentState==MENU_STATE_MAIN_MENU) ctx->running=0;
                else if (ctx->currentState==MENU_STATE_CHAR_SELECT) {
                    if (charState.single_multi==0) { charState.single_multi=1; charState.avatar_select=0; avatarInput.activeField=-1; }
                    else { if(assets.bgMusic)change_music(assets.bgMusic,"music.mp3"); ctx->nextState=MENU_STATE_MAIN_MENU; }
                }
                else { if(assets.bgMusic)change_music(assets.bgMusic,"music.mp3"); ctx->nextState=MENU_STATE_MAIN_MENU; }
            }
            if (ctx->currentState==MENU_STATE_MAIN_MENU) {
                if (key==SDLK_j){play_click_sound();if(assets.gameMusic)change_music(assets.gameMusic,"music3.mp3");ctx->nextState=MENU_STATE_SAVE_LOAD;}
                if (key==SDLK_o){play_click_sound();if(assets.gameMusic)change_music(assets.gameMusic,"music3.mp3");ctx->nextState=MENU_STATE_OPTIONS;}
            } else if (ctx->currentState==MENU_STATE_OPTIONS) {
                if(key==SDLK_PLUS||key==SDLK_EQUALS){ctx->volume=SDL_min(ctx->volume+10,128);play_click_sound();}
                if(key==SDLK_MINUS){ctx->volume=SDL_max(ctx->volume-10,0);play_click_sound();}
                if(key==SDLK_f){ctx->fullscreen=!ctx->fullscreen;play_click_sound();SDL_SetWindowFullscreen(ctx->window,ctx->fullscreen?SDL_WINDOW_FULLSCREEN_DESKTOP:0);update_button_positions(ctx->fullscreen);}
            } else if (ctx->currentState==MENU_STATE_SAVE_LOAD) {
                if(key==SDLK_y){play_click_sound();charState.single_multi=1;charState.avatar_select=0;if(assets.charSelecMusic)change_music(assets.charSelecMusic,"music2.mp3");ctx->nextState=MENU_STATE_CHAR_SELECT;}
                if(key==SDLK_n){play_click_sound();charState.single_multi=1;charState.avatar_select=0;if(assets.charSelecMusic)change_music(assets.charSelecMusic,"music2.mp3");ctx->nextState=MENU_STATE_CHAR_SELECT;}
            } else if (ctx->currentState==MENU_STATE_PLAYER_SELECT) {
                if(key==SDLK_s){printf("Single player\n");play_click_sound();}
                if(key==SDLK_p){printf("Multiplayer\n");  play_click_sound();}
            }
        }
    }

    Mix_VolumeMusic(ctx->volume);
}

void menu_render(MenuContext *ctx)
{
    switch (ctx->currentState) {
        case MENU_STATE_MAIN_MENU:     render_main_menu(ctx);       break;
        case MENU_STATE_OPTIONS:       render_options_menu(ctx);    break;
        case MENU_STATE_SAVE_LOAD:     render_save_load_menu(ctx);  break;
        case MENU_STATE_PLAYER_SELECT: render_player_menu(ctx);     break;
        case MENU_STATE_SCORES:        render_scores_menu(ctx);     break;
        case MENU_STATE_CHAR_SELECT:   render_char_selec_menu(ctx); break;
        default: break;
    }
    SDL_RenderPresent(ctx->renderer);
}

void menu_run(MenuContext *ctx)
{
    while (ctx->running) {
        menu_update(ctx);
        menu_render(ctx);
    }
}
