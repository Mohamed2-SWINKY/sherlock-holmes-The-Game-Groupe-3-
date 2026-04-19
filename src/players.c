#include "players.h"

static int is_blocked(SDL_Rect r,
                      SDL_Rect *obs,  int obs_cnt,
                      MapDoor  *doors, int dc, int *door_open)
{
    for (int i = 0; i < obs_cnt; i++)
        if (map_rects_overlap(r, obs[i])) return 1;
    for (int i = 0; i < dc; i++)
        if (!door_open[i] && map_rects_overlap(r, doors[i].rect)) return 1;
    return 0;
}

/* ── Enemy helpers ── */
static float enemy_vec2len(float dx, float dy) {
    return sqrtf(dx*dx + dy*dy);
}
static void enemy_normalize(float *dx, float *dy) {
    float len = enemy_vec2len(*dx, *dy);
    if (len > 0.0f) { *dx /= len; *dy /= len; }
}

void enemyAtlas_load(EnemyAtlas *a, SDL_Renderer *renderer) {
    /* adjust paths to match YOUR assets folder */
    const char *rightFiles[MAX_WALK_RIGHT] = {
        "assets/enemy/right/f1.png","assets/enemy/right/f2.png",
        "assets/enemy/right/f3.png","assets/enemy/right/f4.png"
    };
    const char *upFiles[MAX_WALK_UP] = {
        "assets/enemy/up/f1u.png","assets/enemy/up/f2u.png",
        "assets/enemy/up/f3u.png","assets/enemy/up/f4u.png"
    };
    const char *downFiles[MAX_WALK_DOWN] = {
        "assets/enemy/down/f1d.png","assets/enemy/down/f2d.png",
        "assets/enemy/down/f3d.png","assets/enemy/down/f4d.png"
    };
    const char *attackFiles[MAX_ATTACK_FRAMES] = {
        "assets/enemy/attacks/att1.png",
        "assets/enemy/attacks/att2.png",
        "assets/enemy/attacks/att3.png"
    };
    for (int i=0;i<MAX_WALK_RIGHT;   i++) a->walkRight[i]=loadTexture(rightFiles[i],  renderer);
    for (int i=0;i<MAX_WALK_UP;      i++) a->walkUp[i]   =loadTexture(upFiles[i],     renderer);
    for (int i=0;i<MAX_WALK_DOWN;    i++) a->walkDown[i] =loadTexture(downFiles[i],   renderer);
    for (int i=0;i<MAX_ATTACK_FRAMES;i++) a->attack[i]   =loadTexture(attackFiles[i], renderer);
}

void enemy_anim_update(EnemyAnimation *a, float dx, float dy) {
    if (a->state == ANIM_ATTACK) return;
    if      (dy < 0) { a->dir=DIR_UP;    a->maxFrames=MAX_WALK_UP;    a->flip=SDL_FLIP_NONE; }
    else if (dy > 0) { a->dir=DIR_DOWN;  a->maxFrames=MAX_WALK_DOWN;  a->flip=SDL_FLIP_NONE; }
    else if (dx > 0) { a->dir=DIR_RIGHT; a->maxFrames=MAX_WALK_RIGHT; a->flip=SDL_FLIP_NONE; }
    else if (dx < 0) { a->dir=DIR_RIGHT; a->maxFrames=MAX_WALK_RIGHT; a->flip=SDL_FLIP_HORIZONTAL; }
    if (dx != 0 || dy != 0) {
        a->state = ANIM_WALK;
        if (++a->frameCounter >= ENEMY_FRAME_DELAY) {
            a->currentFrame = (a->currentFrame+1) % a->maxFrames;
            a->frameCounter = 0;
        }
    } else {
        a->state = ANIM_IDLE;
        a->currentFrame = 0;
    }
}

void enemy_anim_render(SDL_Renderer *r, EnemyAnimation *a, EnemyAtlas *atlas, SDL_Rect *dst) {
    SDL_Texture *frame = NULL;
    if (a->state == ANIM_ATTACK) {
        frame = atlas->attack[a->attackFrame];
        if (frame) SDL_RenderCopyEx(r, frame, NULL, dst, 0, NULL, a->flip);
        if (++a->attackCounter >= ATTACK_DELAY) {
            a->attackCounter = 0;
            if (++a->attackFrame >= MAX_ATTACK_FRAMES) {
                a->state = ANIM_IDLE;
                a->attackFrame = 0;
            }
        }
        return;
    }
    switch (a->dir) {
        case DIR_UP:    frame = atlas->walkUp[a->currentFrame];    break;
        case DIR_DOWN:  frame = atlas->walkDown[a->currentFrame];  break;
        default:        frame = atlas->walkRight[a->currentFrame]; break;
    }
    if (frame) SDL_RenderCopyEx(r, frame, NULL, dst, 0, NULL, a->flip);
}

void enemy_choose_target(Enemy *e) {
    /* random wandering inside map bounds */
    if (rand() % 2)
        e->targetX = (float)(rand() % (MAP_W - e->w)), e->targetY = e->y;
    else
        e->targetY = (float)(rand() % (MAP_H - e->h)), e->targetX = e->x;
}

void enemy_init(GameContext *ctx) {
    Enemy *e  = &ctx->enemy;
    e->w      = 80;
    e->h      = 90;
    e->x      = 1220.0f;
    e->y      = 190.0f;
    e->speed  = 80.0f;
    e->alive  = 1;
    e->atlas  = &ctx->enemyAtlas;
    e->rect   = (SDL_Rect){(int)e->x, (int)e->y, e->w, e->h};
    e->anim.state        = ANIM_IDLE;
    e->anim.dir          = DIR_DOWN;
    e->anim.currentFrame = 0;
    e->anim.frameCounter = 0;
    e->anim.maxFrames    = MAX_WALK_DOWN;
    e->anim.flip         = SDL_FLIP_NONE;
    e->anim.attackFrame  = 0;
    e->anim.attackCounter= 0;
    e->healthStatus = 0;
    e->maxHealth    = 4;  /* change this to whatever you want */
    e->knockbackX     = 0;
    e->knockbackY     = 0;
    e->knockbackTimer = 0;
    enemy_choose_target(e);
}

void enemy_update(GameContext *ctx, float dt) {
    Enemy *e = &ctx->enemy;
    if (!e->alive || e->anim.state == ANIM_ATTACK) return;

    SDL_Rect *obs     = ctx->map.obs2;
    int       obs_cnt = ctx->map.obs2_cnt;
    MapDoor  *doors   = ctx->map.doors2;
    int       dc      = ctx->map.doors2_cnt;
    int      *door_open = ctx->map.door2_open;

    float dx = e->targetX - e->x;
    float dy = e->targetY - e->y;
    if (fabsf(dx) > fabsf(dy)) dy = 0.0f; else dx = 0.0f;

    float dist = enemy_vec2len(dx, dy);
    if (dist > 2.0f) {
        enemy_normalize(&dx, &dy);
        float stepX = dx * e->speed * dt;
        float stepY = dy * e->speed * dt;

        /* test X */
        SDL_Rect testX = { (int)(e->x + stepX), (int)e->y, e->w, e->h };
        if (!is_blocked(testX, obs, obs_cnt, doors, dc, door_open))
            e->x += stepX;
        else
            dx = 0.0f;

        /* test Y */
        SDL_Rect testY = { (int)e->x, (int)(e->y + stepY), e->w, e->h };
        if (!is_blocked(testY, obs, obs_cnt, doors, dc, door_open))
            e->y += stepY;
        else
            dy = 0.0f;

        /* if fully blocked choose a new target */
        if (dx == 0.0f && dy == 0.0f)
            enemy_choose_target(e);
    } else {
        enemy_choose_target(e);
        dx = dy = 0.0f;
    }

    e->rect.x = (int)e->x;
    e->rect.y = (int)e->y;
    enemy_anim_update(&e->anim, dx, dy);

    /* enemy touches player1 */
if (ctx->player1.alive && map_rects_overlap(e->rect, ctx->player1.rect)) {
    ctx->player1.knockbackX      = (e->x < ctx->player1.rect.x) ? 8 : -8;
    ctx->player1.knockbackXTimer = 15;
    if (ctx->player1.healthStatus < 6) {
        ctx->player1.healthStatus++;
        if (ctx->player1.healthStatus == 6) {
            ctx->player1.alive        = 0;
            ctx->player1.healthStatus = 7;
            Mix_PlayChannel(CH_P1_DEATH, ctx->player1.deathSound, 0);
        }
    }
    if (!Mix_Playing(CH_P1_GETHIT))
        Mix_PlayChannel(CH_P1_GETHIT, ctx->player1.gettingHitSound, 0);
    /* push enemy back so it doesn't keep dealing damage every frame */
    e->x -= (e->x < ctx->player1.rect.x) ? 20.0f : -20.0f;
    enemy_choose_target(e);
}

/* enemy touches player2 */
if (ctx->player2.alive && map_rects_overlap(e->rect, ctx->player2.rect)) {
    ctx->player2.knockbackX      = (e->x < ctx->player2.rect.x) ? 8 : -8;
    ctx->player2.knockbackXTimer = 15;
    if (ctx->player2.healthStatus < 6) {
        ctx->player2.healthStatus++;
        if (ctx->player2.healthStatus == 6) {
            ctx->player2.alive        = 0;
            ctx->player2.healthStatus = 7;
            Mix_PlayChannel(CH_P2_DEATH, ctx->player2.deathSound, 0);
        }
    }
    if (!Mix_Playing(CH_P2_GETHIT))
        Mix_PlayChannel(CH_P2_GETHIT, ctx->player2.gettingHitSound, 0);
    e->x -= (e->x < ctx->player2.rect.x) ? 20.0f : -20.0f;
    enemy_choose_target(e);
}
if (e->knockbackTimer > 0) {
    SDL_Rect testK = { (int)(e->x + e->knockbackX * dt), (int)e->y, e->w, e->h };
    if (!is_blocked(testK, obs, obs_cnt, doors, dc, door_open))
        e->x += e->knockbackX * dt;
    e->knockbackX *= 0.85f;
    e->knockbackTimer--;
}
}

void enemy_render(GameContext *ctx, int camX, int camY) {
    Enemy *e = &ctx->enemy;
    if (!e->alive) return;
    SDL_Rect dst = {
        (int)((e->x - camX) * ZOOM_FACTOR),
        (int)((e->y - camY) * ZOOM_FACTOR),
        (int)(e->w * ZOOM_FACTOR),
        (int)(e->h * ZOOM_FACTOR)
    };
    enemy_anim_render(ctx->renderer, &e->anim, e->atlas, &dst);
}


SDL_Texture *loadTexture(const char *path, SDL_Renderer *renderer)
{
    SDL_Texture *texture = IMG_LoadTexture(renderer, path);
    if (!texture)
        printf("Failed to load texture: %s\n", IMG_GetError());
    return texture;
}

void initOutfit(GameContext *ctx, int playerNum, int outfitNum, int charNum)
{
    char fullPath[64];
    Player *p = (playerNum == 1) ? &ctx->player1 : &ctx->player2;

    sprintf(fullPath, "assets/player%d/outfit%d/idle.png", charNum, outfitNum);
    p->idle = loadTexture(fullPath, ctx->renderer);
    p->currentState = p->idle;

    for (int i = 0; i < 5; i++) {
        sprintf(fullPath, "assets/player%d/outfit%d/right%d.png", charNum, outfitNum, i+1);
        p->walkRight[i] = loadTexture(fullPath, ctx->renderer);
        sprintf(fullPath, "assets/player%d/outfit%d/left%d.png", charNum, outfitNum, i+1);
        p->walkLeft[i] = loadTexture(fullPath, ctx->renderer);
    }
    for (int i = 0; i < 2; i++) {
        sprintf(fullPath, "assets/player%d/outfit%d/up%d.png", charNum, outfitNum, i+1);
        p->walkUp[i] = loadTexture(fullPath, ctx->renderer);
        sprintf(fullPath, "assets/player%d/outfit%d/down%d.png", charNum, outfitNum, i+1);
        p->walkDown[i] = loadTexture(fullPath, ctx->renderer);
    }
    for (int i = 0; i < 6; i++) {
        sprintf(fullPath, "assets/player%d/outfit%d/attackRight%d.png", charNum, outfitNum, i+1);
        p->attackRight[i] = loadTexture(fullPath, ctx->renderer);
        sprintf(fullPath, "assets/player%d/outfit%d/attackLeft%d.png", charNum, outfitNum, i+1);
        p->attackLeft[i] = loadTexture(fullPath, ctx->renderer);
    }
}

void initPlayer1(GameContext *ctx)
{
    ctx->player1.outfitNum     = 1;
    ctx->player1.outfitSwitched = 0;
    initOutfit(ctx, 1, 1, 1);

    for (int i = 0; i < 8; i++) {
        char path[64];
        sprintf(path, "assets/hpBar/hpBar%d.png", i+1);
        ctx->player1.hpBar[i] = loadTexture(path, ctx->renderer);
    }

    ctx->player1.attacking    = 0;
    ctx->player1.attackTimer  = 0;
    ctx->player1.attackFrame  = 0;
    ctx->player1.frame        = 0;
    ctx->player1.frameTimer   = 0;
    ctx->player1.frameDelay   = WALKING_FRAME_DELAY;
    ctx->player1.lastDir      = -1;
    ctx->player1.lastHDir     = SDL_SCANCODE_D;
    ctx->player1.jumping      = 0;
    ctx->player1.jumpTimer    = 0;
    ctx->player1.jumpOffset   = 0;

    ctx->player1.rect.x = PLAYER1_X;
    ctx->player1.rect.y = PLAYER1_Y;
    ctx->player1.rect.w = PLAYER1_W;
    ctx->player1.rect.h = PLAYER1_H;

    ctx->player1.healthRect = (SDL_Rect){PLAYER1HP_X, PLAYER1HP_Y, PLAYER1HP_W, PLAYER1HP_H};

    ctx->player1.walkingSound    = Mix_LoadWAV("assets/sounds/walking.mp3");
    ctx->player1.runningSound    = Mix_LoadWAV("assets/sounds/running.mp3");
    ctx->player1.jumpingSound    = Mix_LoadWAV("assets/sounds/jump.mp3");
    ctx->player1.attackingSound  = Mix_LoadWAV("assets/sounds/attack.mp3");
    ctx->player1.gettingHitSound = Mix_LoadWAV("assets/sounds/hitCharacter.mp3");
    ctx->player1.deathSound      = Mix_LoadWAV("assets/sounds/death.mp3");

    ctx->player1.healthStatus    = 0;
    ctx->player1.alive           = 1;
    ctx->player1.speed           = WALKING_SPEED;
    ctx->player1.walkToRun       = 0;
    ctx->player1.knockbackX      = 0;
    ctx->player1.knockbackXTimer = 0;
    ctx->player1.score           = 0;
    ctx->player1.moving          = 0;
    ctx->player1.selectedOutfit  = 1;
    ctx->player1.selectedChar    = 1;
}

void initPlayer2(GameContext *ctx)
{
    ctx->player2.outfitNum      = 1;
    ctx->player2.outfitSwitched = 0;
    initOutfit(ctx, 2, 1, 2);

    for (int i = 0; i < 8; i++) {
        char path[64];
        sprintf(path, "assets/hpBar/hpBar%d.png", i+1);
        ctx->player2.hpBar[i] = loadTexture(path, ctx->renderer);
    }

    ctx->player2.attacking    = 0;
    ctx->player2.attackTimer  = 0;
    ctx->player2.attackFrame  = 0;
    ctx->player2.frame        = 0;
    ctx->player2.frameTimer   = 0;
    ctx->player2.frameDelay   = WALKING_FRAME_DELAY;
    ctx->player2.lastDir      = -1;
    ctx->player2.lastHDir     = SDL_SCANCODE_RIGHT;
    ctx->player2.jumping      = 0;
    ctx->player2.jumpTimer    = 0;
    ctx->player2.jumpOffset   = 0;

    ctx->player2.rect.x = PLAYER2_X;
    ctx->player2.rect.y = PLAYER2_Y;
    ctx->player2.rect.w = PLAYER2_W;
    ctx->player2.rect.h = PLAYER2_H;

    ctx->player2.healthRect = (SDL_Rect){PLAYER2HP_X, PLAYER2HP_Y, PLAYER2HP_W, PLAYER2HP_H};

    ctx->player2.walkingSound    = Mix_LoadWAV("assets/sounds/walking.mp3");
    ctx->player2.runningSound    = Mix_LoadWAV("assets/sounds/running.mp3");
    ctx->player2.jumpingSound    = Mix_LoadWAV("assets/sounds/jump.mp3");
    ctx->player2.attackingSound  = Mix_LoadWAV("assets/sounds/attack.mp3");
    ctx->player2.gettingHitSound = Mix_LoadWAV("assets/sounds/hitCharacter.mp3");
    ctx->player2.deathSound      = Mix_LoadWAV("assets/sounds/death.mp3");

    ctx->player2.healthStatus    = 0;
    ctx->player2.alive           = 1;
    ctx->player2.speed           = WALKING_SPEED;
    ctx->player2.walkToRun       = 0;
    ctx->player2.knockbackX      = 0;
    ctx->player2.knockbackXTimer = 0;
    ctx->player2.score           = 0;
    ctx->player2.moving          = 0;
    ctx->player2.selectedOutfit  = 1;
    ctx->player2.selectedChar    = 2;
}

GameContext *game_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError()); return NULL;
    }

    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    int flags = MIX_INIT_MP3;
    if ((Mix_Init(flags) & flags) != flags)
        printf("Warning: SDL_mixer audio limited: %s\n", Mix_GetError());
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        printf("Error: Cannot open audio: %s\n", Mix_GetError());
    Mix_AllocateChannels(16);

    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError()); return NULL;
    }

    GameContext *ctx = malloc(sizeof(GameContext));
    if (!ctx) { fprintf(stderr, "Out of memory\n"); return NULL; }

    ctx->window = SDL_CreateWindow("Sherlock Holmes: The Unseen Enemy",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!ctx->window) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        free(ctx); return NULL;
    }

    ctx->renderer = SDL_CreateRenderer(ctx->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ctx->renderer) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(ctx->window); free(ctx); return NULL;
    }

    memset(ctx->keys, 0, sizeof(ctx->keys));
    ctx->musicLevel2 = Mix_LoadMUS("assets/sounds/level2music.mp3");
    ctx->musicLevel1 = Mix_LoadMUS("assets/sounds/level1music.mp3");

    map_init(&ctx->map, ctx->renderer);
    init_minimap(&ctx->minimap,   ctx->renderer, WINDOW_HEIGHT);
    init_minimap2(&ctx->minimap2, ctx->renderer, WINDOW_HEIGHT, WINDOW_WIDTH);
    initPlayer1(ctx);
    initPlayer2(ctx);
    enemyAtlas_load(&ctx->enemyAtlas, ctx->renderer);
    enemy_init(ctx);
    Mix_PlayMusic(ctx->musicLevel1, -1);
    ctx->cutsceneL2Timer = 0;
    ctx->cutsceneL2Alpha = 0;

    ctx->currentState     = STATE_CUTSCENE;
    ctx->cutsceneTimer    = 0;     // counts up each frame
    ctx->cutsceneAlpha    = 0;     // 0-255 for fade in/out

    ctx->font = TTF_OpenFont("assets/fonts/pixelFont.ttf", 24);
    if (!ctx->font)
        printf("Font failed: %s\n", TTF_GetError());

    ctx->sm.menuBoard          = loadTexture("assets/subMenu/menuBoard.png",                    ctx->renderer);
    ctx->sm.resumeBtn.tex      = loadTexture("assets/subMenu/buttons/resume.png",               ctx->renderer);
    ctx->sm.saveBtn.tex        = loadTexture("assets/subMenu/buttons/save.png",                 ctx->renderer);
    ctx->sm.loadBtn.tex        = loadTexture("assets/subMenu/buttons/load.png",                 ctx->renderer);
    ctx->sm.playerBtn.tex      = loadTexture("assets/subMenu/buttons/players.png",              ctx->renderer);
    ctx->sm.scoreBtn.tex       = loadTexture("assets/subMenu/buttons/scores.png",               ctx->renderer);
    ctx->sm.quitBtn.tex        = loadTexture("assets/subMenu/buttons/quit.png",                 ctx->renderer);
    ctx->sm.charSelectBtn.tex  = loadTexture("assets/subMenu/buttons/charSelect.png",           ctx->renderer);
    ctx->sm.outfitsBtn.tex     = loadTexture("assets/subMenu/buttons/outfits.png",              ctx->renderer);
    ctx->sm.buttonsBtn.tex     = loadTexture("assets/subMenu/buttons/buttons.png",              ctx->renderer);

    SDL_QueryTexture(ctx->sm.menuBoard, NULL, NULL,
                     &ctx->sm.menuBoardRect.w, &ctx->sm.menuBoardRect.h);
    ctx->sm.menuBoardRect = (SDL_Rect){20, 250,
                                       ctx->sm.menuBoardRect.w + 100,
                                       ctx->sm.menuBoardRect.h + 140};

    ctx->paused        = 0;
    ctx->pauseSwitched = 0;
    ctx->running       = 1;
    ctx->isCameraPanning  = 0;
    ctx->cameraFocusTimer = 0;


    ctx->sm.resumeBtn.rect  = (SDL_Rect){60, 320, BUTTON_W, BUTTON_H};
    ctx->sm.saveBtn.rect    = (SDL_Rect){60, 365, BUTTON_W, BUTTON_H};
    ctx->sm.loadBtn.rect    = (SDL_Rect){60, 410, BUTTON_W, BUTTON_H};
    ctx->sm.playerBtn.rect  = (SDL_Rect){60, 455, BUTTON_W, BUTTON_H};
    ctx->sm.scoreBtn.rect   = (SDL_Rect){60, 500, BUTTON_W, BUTTON_H};
    ctx->sm.quitBtn.rect    = (SDL_Rect){60, 545, BUTTON_W, BUTTON_H};

    ctx->sm.resumeBtn.hovered = 0;
    ctx->sm.saveBtn.hovered   = 0;
    ctx->sm.loadBtn.hovered   = 0;
    ctx->sm.playerBtn.hovered = 0;
    ctx->sm.scoreBtn.hovered  = 0;
    ctx->sm.quitBtn.hovered   = 0;

    ctx->sm.hoverSound = Mix_LoadWAV("assets/sounds/buttonHover.wav");

    ctx->sm.charSelectBtn.rect = (SDL_Rect){60, 320, BUTTON_W, BUTTON_H};
    ctx->sm.outfitsBtn.rect    = (SDL_Rect){60, 365, BUTTON_W, BUTTON_H};
    ctx->sm.buttonsBtn.rect    = (SDL_Rect){60, 410, BUTTON_W, BUTTON_H};
    ctx->sm.playerBtnSwitched  = 0;

    ctx->sm.playersBg    = loadTexture("assets/subMenu/backgrounds/playerSubMenuBg.jpg", ctx->renderer);
    ctx->sm.p1o1Btn.tex  = loadTexture("assets/subMenu/outfitFrames/p1o1.png",           ctx->renderer);
    ctx->sm.p1o1Btn.rect = (SDL_Rect){40, 220, 150, 200};
    ctx->sm.p1o1Btn.hovered = 0;

    ctx->sm.p1o2Btn.tex  = loadTexture("assets/subMenu/outfitFrames/p1o2.png", ctx->renderer);
    ctx->sm.p1o2Btn.rect = (SDL_Rect){210, 225, 150, 195};
    ctx->sm.p1o2Btn.hovered = 0;

    ctx->sm.okBtn.tex  = loadTexture("assets/subMenu/buttons/ok.png", ctx->renderer);
    ctx->sm.okBtn.rect = (SDL_Rect){(WINDOW_WIDTH - BUTTON_W) / 2, 540, BUTTON_W, BUTTON_H};
    ctx->sm.okBtn.hovered = 0;

    ctx->sm.backBtn.tex  = loadTexture("assets/subMenu/buttons/back.png", ctx->renderer);
    ctx->sm.backBtn.rect = (SDL_Rect){60, 455, BUTTON_W, BUTTON_H};
    ctx->sm.backBtn.hovered = 0;

    ctx->sm.p2o1Btn.tex  = loadTexture("assets/subMenu/outfitFrames/p2o1.png", ctx->renderer);
    ctx->sm.p2o1Btn.rect = (SDL_Rect){WINDOW_WIDTH - 380, 220, 150, 200};
    ctx->sm.p2o1Btn.hovered = 0;

    ctx->sm.p2o2Btn.tex  = loadTexture("assets/subMenu/outfitFrames/p2o2.png", ctx->renderer);
    ctx->sm.p2o2Btn.rect = (SDL_Rect){WINDOW_WIDTH - 210, 225, 150, 195};
    ctx->sm.p2o2Btn.hovered = 0;

    ctx->sm.charSelectBg = loadTexture("assets/subMenu/backgrounds/playerSubMenuBg.jpg", ctx->renderer);
    ctx->sm.p1preview    = loadTexture("assets/subMenu/charSelect/p1preview.png",         ctx->renderer);
    ctx->sm.p2preview    = loadTexture("assets/subMenu/charSelect/p2preview.png",         ctx->renderer);

    ctx->sm.p1SwapBtn.tex  = loadTexture("assets/subMenu/buttons/swap.png", ctx->renderer);
    ctx->sm.p1SwapBtn.rect = (SDL_Rect){150 + (180 - BUTTON_W) / 2, 490, BUTTON_W, BUTTON_H};
    ctx->sm.p1SwapBtn.hovered = 0;

    ctx->sm.p2SwapBtn.tex  = loadTexture("assets/subMenu/buttons/swap.png", ctx->renderer);
    ctx->sm.p2SwapBtn.rect = (SDL_Rect){650 + (180 - BUTTON_W) / 2, 490, BUTTON_W, BUTTON_H};
    ctx->sm.p2SwapBtn.hovered = 0;

    return ctx;
}

int hasIntersection(SDL_Rect r1, SDL_Rect r2)
{
    return (r1.x + r1.w >= r2.x &&
            r1.x        <= r2.x + r2.w &&
            r1.y + r1.h >= r2.y &&
            r1.y        <= r2.y + r2.h);
}

void playerMechanics(GameContext *ctx)
{
    printf("x: %d, y= %d\n", ctx->player1.rect.x, ctx->player1.rect.y);

    SDL_Rect *obs       = (ctx->map.level == LEVEL_1) ? ctx->map.obs1       : ctx->map.obs2;
    int       obs_cnt   = (ctx->map.level == LEVEL_1) ? ctx->map.obs1_cnt   : ctx->map.obs2_cnt;
    MapDoor  *doors     = (ctx->map.level == LEVEL_1) ? ctx->map.doors1     : ctx->map.doors2;
    int       dc        = (ctx->map.level == LEVEL_1) ? ctx->map.doors1_cnt : ctx->map.doors2_cnt;
    int      *door_open = (ctx->map.level == LEVEL_1) ? ctx->map.door1_open : ctx->map.door2_open;
    

    if (ctx->paused) return;

    ctx->player1.moving = 0;
    ctx->player2.moving = 0;

    /* ───────────────────────── PLAYER 1 ───────────────────────── */
    if (ctx->player1.alive)
    {
        /* running */
        if (ctx->keys[SDL_SCANCODE_LSHIFT]) {
            ctx->player1.speed      = 5;
            ctx->player1.frameDelay = 7;
            if (!ctx->player1.walkToRun) {
                Mix_HaltChannel(CH_P1_WALK);
                ctx->player1.walkToRun = 1;
            }
            if (!Mix_Playing(CH_P1_WALK))
                Mix_PlayChannel(CH_P1_WALK, ctx->player1.runningSound, -1);
        } else {
            ctx->player1.speed      = WALKING_SPEED;
            ctx->player1.frameDelay = WALKING_FRAME_DELAY;
            ctx->player1.walkToRun  = 0;
        }

        /* OPTION B MOVE: compute intended dx/dy, test once, move only if NOT blocked */
        int dx = 0, dy = 0;
        if (ctx->keys[SDL_SCANCODE_D]) dx += ctx->player1.speed;
        if (ctx->keys[SDL_SCANCODE_A]) dx -= ctx->player1.speed;
        if (ctx->keys[SDL_SCANCODE_S]) dy += ctx->player1.speed;
        if (ctx->keys[SDL_SCANCODE_W]) dy -= ctx->player1.speed;

        if (dx != 0 || dy != 0) {
          int moved = 0;

          SDL_Rect testX = ctx->player1.rect;
          testX.x += dx;
          if (!is_blocked(testX, obs, obs_cnt, doors, dc, door_open)) {
              ctx->player1.rect.x += dx;
              moved = 1;
          } else {
              minimap_trigger_shake(&ctx->minimap);
          }

          SDL_Rect testY = ctx->player1.rect;
          testY.y += dy;
          if (!is_blocked(testY, obs, obs_cnt, doors, dc, door_open)) {
              ctx->player1.rect.y += dy;
              moved = 1;
          } else {
              minimap_trigger_shake(&ctx->minimap);
          }

          if (moved && !ctx->player1.attacking) {
    if (abs(dx) >= abs(dy) && dx != 0) {
        if (dx > 0) {
            ctx->player1.lastDir  = SDL_SCANCODE_D;
            ctx->player1.lastHDir = SDL_SCANCODE_D;
            ctx->player1.currentState = ctx->player1.walkRight[ctx->player1.frame % 5];
        } else {
            ctx->player1.lastDir  = SDL_SCANCODE_A;
            ctx->player1.lastHDir = SDL_SCANCODE_A;
            ctx->player1.currentState = ctx->player1.walkLeft[ctx->player1.frame % 5];
        }
    } else if (dy != 0) {
        if (dy > 0) {
            ctx->player1.lastDir = SDL_SCANCODE_S;
            ctx->player1.currentState = ctx->player1.walkDown[ctx->player1.frame % 2];
        } else {
            ctx->player1.lastDir = SDL_SCANCODE_W;
            ctx->player1.currentState = ctx->player1.walkUp[ctx->player1.frame % 2];
        }
    }
}
ctx->player1.moving = moved;
if (!moved) Mix_HaltChannel(CH_P1_WALK);

          ctx->player1.moving = moved;
if (moved) {
    if (!Mix_Playing(CH_P1_WALK) && !ctx->player1.walkToRun)
        Mix_PlayChannel(CH_P1_WALK, ctx->player1.walkingSound, -1);
} else {
    Mix_HaltChannel(CH_P1_WALK);
}
        }

        /* jump */
        if (ctx->keys[SDL_SCANCODE_SPACE] && !ctx->player1.jumping) {
            ctx->player1.jumping   = 1;
            ctx->player1.jumpTimer = 0;
            Mix_HaltChannel(CH_P1_WALK);
            if (!Mix_Playing(CH_P1_JUMP))
                Mix_PlayChannel(CH_P1_JUMP, ctx->player1.jumpingSound, 0);
        }

        /* attack */
        if (ctx->keys[SDL_SCANCODE_B] && !ctx->player1.attacking) {
            ctx->player1.attacking   = 1;
            ctx->player1.attackTimer = 0;
            ctx->player1.attackFrame = 0;
            ctx->player1.baseX       = ctx->player1.rect.x;
            ctx->player1.currentState =
                (ctx->player1.lastHDir == SDL_SCANCODE_A) ? ctx->player1.attackLeft[0]
                                                         : ctx->player1.attackRight[0];
            if (!Mix_Playing(CH_P1_ATTACK))
                Mix_PlayChannel(CH_P1_ATTACK, ctx->player1.attackingSound, 0);
        }

        /* attack animation */
        if (ctx->player1.attacking) {
            ctx->player1.attackTimer++;
            if (ctx->player1.attackTimer >= 4) {
                ctx->player1.attackTimer = 0;
                ctx->player1.attackFrame++;

                if (ctx->player1.attackFrame == 3) {
                    ctx->player1.rect.w += 20;
                    if (hasIntersection(ctx->player1.rect, ctx->player2.rect) && ctx->player2.alive) {
                        ctx->player2.knockbackX      = (ctx->player1.lastHDir == SDL_SCANCODE_D) ? 8 : -8;
                        ctx->player2.knockbackXTimer = 15;

                        if (ctx->player2.healthStatus < 6) {
                            ctx->player2.healthStatus++;
                            if (ctx->player2.healthStatus == 6) {
                                ctx->player2.alive        = 0;
                                ctx->player2.healthStatus = 7;
                                if (!Mix_Playing(CH_P2_DEATH))
                                    Mix_PlayChannel(CH_P2_DEATH, ctx->player2.deathSound, 0);
                                ctx->player1.score -= 99;
                            }
                        }
                        if (!Mix_Playing(CH_P2_GETHIT))
                            Mix_PlayChannel(CH_P2_GETHIT, ctx->player2.gettingHitSound, 0);
                    }
                } else {
                    ctx->player1.rect.w = PLAYER1_W;
                }

                if (ctx->map.level == LEVEL_2 && ctx->enemy.alive &&
                  hasIntersection(ctx->player1.rect, ctx->enemy.rect)) {
                  ctx->enemy.healthStatus++;
if (ctx->enemy.healthStatus >= ctx->enemy.maxHealth)
    ctx->enemy.healthStatus++;
ctx->enemy.knockbackX     = (ctx->player1.lastHDir == SDL_SCANCODE_D) ? 120.0f : -120.0f;
ctx->enemy.knockbackY     = 0;
ctx->enemy.knockbackTimer = 12;
if (ctx->enemy.healthStatus >= ctx->enemy.maxHealth)
    ctx->enemy.alive = 0;
              }

                if (ctx->player1.attackFrame >= 6) {
                    ctx->player1.attacking   = 0;
                    ctx->player1.attackFrame = 0;
                } else {
                    ctx->player1.currentState =
                        (ctx->player1.lastHDir == SDL_SCANCODE_A) ? ctx->player1.attackLeft[ctx->player1.attackFrame]
                                                                 : ctx->player1.attackRight[ctx->player1.attackFrame];
                }
            }
        }

        /* knockback (collision-safe so x doesn't keep changing into walls) */
        if (ctx->player1.knockbackXTimer > 0) {
            SDL_Rect test = ctx->player1.rect;
            test.x += ctx->player1.knockbackX;

            if (!is_blocked(test, obs, obs_cnt, doors, dc, door_open)) {
                ctx->player1.rect.x += ctx->player1.knockbackX;
            } else {
                ctx->player1.knockbackX = 0;
                ctx->player1.knockbackXTimer = 0;
            }

            ctx->player1.knockbackX = (int)(ctx->player1.knockbackX * 0.85f);
            ctx->player1.knockbackXTimer--;
        }

        /* idle / walk animation */
        if (!ctx->player1.attacking) {
            if (!ctx->player1.moving) {
                Mix_HaltChannel(CH_P1_WALK);
                if      (ctx->player1.lastDir == SDL_SCANCODE_D) ctx->player1.currentState = ctx->player1.walkRight[0];
                else if (ctx->player1.lastDir == SDL_SCANCODE_A) ctx->player1.currentState = ctx->player1.walkLeft[0];
                else if (ctx->player1.lastDir == SDL_SCANCODE_W) ctx->player1.currentState = ctx->player1.walkUp[0];
                else if (ctx->player1.lastDir == SDL_SCANCODE_S) ctx->player1.currentState = ctx->player1.walkDown[0];
                else                                             ctx->player1.currentState = ctx->player1.idle;
                ctx->player1.frame      = 0;
                ctx->player1.frameTimer = 0;
            } else {
                ctx->player1.frameTimer++;
                if (ctx->player1.frameTimer >= ctx->player1.frameDelay) {
                    ctx->player1.frameTimer = 0;
                    ctx->player1.frame++;
                }
            }
        }

        /* jump arc */
        if (ctx->player1.jumping) {
            ctx->player1.jumpTimer++;
            float t = (float)ctx->player1.jumpTimer / 30.0f;
            ctx->player1.jumpOffset = (int)(sinf(t * 3.14f) * 40);
            if (ctx->player1.jumpTimer >= 30) {
                ctx->player1.jumping    = 0;
                ctx->player1.jumpTimer  = 0;
                ctx->player1.jumpOffset = 0;
            }
        }
    }

    /* ───────────────────────── PLAYER 2 ───────────────────────── */
    if (ctx->player2.alive)
    {
        /* running */
        if (ctx->keys[SDL_SCANCODE_M]) {
            ctx->player2.speed      = 5;
            ctx->player2.frameDelay = 7;
            if (!ctx->player2.walkToRun) {
                Mix_HaltChannel(CH_P2_WALK);
                ctx->player2.walkToRun = 1;
            }
            if (!Mix_Playing(CH_P2_WALK))
                Mix_PlayChannel(CH_P2_WALK, ctx->player2.runningSound, -1);
        } else {
            ctx->player2.speed      = WALKING_SPEED;
            ctx->player2.frameDelay = WALKING_FRAME_DELAY;
            ctx->player2.walkToRun  = 0;
        }

        /* OPTION B MOVE: compute intended dx/dy, test once, move only if NOT blocked */
        int dx2 = 0, dy2 = 0;
        if (ctx->keys[SDL_SCANCODE_RIGHT]) dx2 += ctx->player2.speed;
        if (ctx->keys[SDL_SCANCODE_LEFT])  dx2 -= ctx->player2.speed;
        if (ctx->keys[SDL_SCANCODE_DOWN])  dy2 += ctx->player2.speed;
        if (ctx->keys[SDL_SCANCODE_UP])    dy2 -= ctx->player2.speed;

        if (dx2 != 0 || dy2 != 0) {
            int moved2 = 0;

SDL_Rect testX = ctx->player2.rect;
testX.x += dx2;
if (!is_blocked(testX, obs, obs_cnt, doors, dc, door_open)) {
    ctx->player2.rect.x += dx2;
    moved2 = 1;
} else {
    minimap_trigger_shake(&ctx->minimap2);
}

SDL_Rect testY = ctx->player2.rect;
testY.y += dy2;
if (!is_blocked(testY, obs, obs_cnt, doors, dc, door_open)) {
    ctx->player2.rect.y += dy2;
    moved2 = 1;
} else {
    minimap_trigger_shake(&ctx->minimap2);
}

if (moved2 && !ctx->player2.attacking) {
    if (abs(dx2) >= abs(dy2) && dx2 != 0) {
        if (dx2 > 0) {
            ctx->player2.lastDir  = SDL_SCANCODE_RIGHT;
            ctx->player2.lastHDir = SDL_SCANCODE_RIGHT;
            ctx->player2.currentState = ctx->player2.walkRight[ctx->player2.frame % 5];
        } else {
            ctx->player2.lastDir  = SDL_SCANCODE_LEFT;
            ctx->player2.lastHDir = SDL_SCANCODE_LEFT;
            ctx->player2.currentState = ctx->player2.walkLeft[ctx->player2.frame % 5];
        }
    } else if (dy2 != 0) {
        if (dy2 > 0) {
            ctx->player2.lastDir = SDL_SCANCODE_DOWN;
            ctx->player2.currentState = ctx->player2.walkDown[ctx->player2.frame % 2];
        } else {
            ctx->player2.lastDir = SDL_SCANCODE_UP;
            ctx->player2.currentState = ctx->player2.walkUp[ctx->player2.frame % 2];
        }
    }
}
ctx->player2.moving = moved2;
if (moved2) {
    if (!Mix_Playing(CH_P2_WALK) && !ctx->player2.walkToRun)
        Mix_PlayChannel(CH_P2_WALK, ctx->player2.walkingSound, -1);
} else {
    Mix_HaltChannel(CH_P2_WALK);
}
        }

        /* jump */
        if (ctx->keys[SDL_SCANCODE_RSHIFT] && !ctx->player2.jumping) {
            ctx->player2.jumping   = 1;
            ctx->player2.jumpTimer = 0;
            if (!Mix_Playing(CH_P2_JUMP))
                Mix_PlayChannel(CH_P2_JUMP, ctx->player2.jumpingSound, 0);
        }

        /* attack */
        if (ctx->keys[SDL_SCANCODE_RETURN] && !ctx->player2.attacking) {
            ctx->player2.attacking   = 1;
            ctx->player2.attackTimer = 0;
            ctx->player2.attackFrame = 0;
            ctx->player2.baseX       = ctx->player2.rect.x;
            ctx->player2.currentState =
                (ctx->player2.lastHDir == SDL_SCANCODE_LEFT) ? ctx->player2.attackLeft[0]
                                                            : ctx->player2.attackRight[0];
            if (!Mix_Playing(CH_P2_ATTACK))
                Mix_PlayChannel(CH_P2_ATTACK, ctx->player2.attackingSound, 0);
        }

        /* knockback (collision-safe) */
        if (ctx->player2.knockbackXTimer > 0) {
            SDL_Rect test = ctx->player2.rect;
            test.x += ctx->player2.knockbackX;

            if (!is_blocked(test, obs, obs_cnt, doors, dc, door_open)) {
                ctx->player2.rect.x += ctx->player2.knockbackX;
            } else {
                ctx->player2.knockbackX = 0;
                ctx->player2.knockbackXTimer = 0;
            }

            ctx->player2.knockbackX = (int)(ctx->player2.knockbackX * 0.85f);
            ctx->player2.knockbackXTimer--;
        }

        /* idle / walk animation */
        if (!ctx->player2.attacking) {
            if (!ctx->player2.moving) {
                Mix_HaltChannel(CH_P2_WALK);
                if      (ctx->player2.lastDir == SDL_SCANCODE_RIGHT) ctx->player2.currentState = ctx->player2.walkRight[0];
                else if (ctx->player2.lastDir == SDL_SCANCODE_LEFT)  ctx->player2.currentState = ctx->player2.walkLeft[0];
                else if (ctx->player2.lastDir == SDL_SCANCODE_UP)    ctx->player2.currentState = ctx->player2.walkUp[0];
                else if (ctx->player2.lastDir == SDL_SCANCODE_DOWN)  ctx->player2.currentState = ctx->player2.walkDown[0];
                else                                                 ctx->player2.currentState = ctx->player2.idle;
                ctx->player2.frame      = 0;
                ctx->player2.frameTimer = 0;
            } else {
                ctx->player2.frameTimer++;
                if (ctx->player2.frameTimer >= ctx->player2.frameDelay) {
                    ctx->player2.frameTimer = 0;
                    ctx->player2.frame++;
                }
            }
        }

        /* jump arc */
        if (ctx->player2.jumping) {
            ctx->player2.jumpTimer++;
            float t = (float)ctx->player2.jumpTimer / 30.0f;
            ctx->player2.jumpOffset = (int)(sinf(t * 3.14f) * 40);
            if (ctx->player2.jumpTimer >= 30) {
                ctx->player2.jumping    = 0;
                ctx->player2.jumpTimer  = 0;
                ctx->player2.jumpOffset = 0;
            }
        }

        /* attack animation — only extend hitbox on frame 3 */
        if (ctx->player2.attacking) {
            ctx->player2.attackTimer++;
            if (ctx->player2.attackTimer >= 4) {
                ctx->player2.attackTimer = 0;
                ctx->player2.attackFrame++;

                if (ctx->player2.attackFrame == 3) {
                    ctx->player2.rect.w += 10;
                    if (hasIntersection(ctx->player1.rect, ctx->player2.rect) && ctx->player1.alive) {
                        ctx->player1.knockbackX      = (ctx->player2.lastHDir == SDL_SCANCODE_RIGHT) ? 8 : -8;
                        ctx->player1.knockbackXTimer = 15;

                        if (ctx->player1.healthStatus < 6) {
                            ctx->player1.healthStatus++;
                            if (ctx->player1.healthStatus == 6) {
                                ctx->player1.alive        = 0;
                                ctx->player1.healthStatus = 7;
                                if (!Mix_Playing(CH_P1_DEATH))
                                    Mix_PlayChannel(CH_P1_DEATH, ctx->player1.deathSound, 0);
                                ctx->player2.score -= 99;
                            }
                        }
                        if (!Mix_Playing(CH_P1_GETHIT))
                            Mix_PlayChannel(CH_P1_GETHIT, ctx->player1.gettingHitSound, 0);
                    }
                } else {
                    ctx->player2.rect.w = PLAYER2_W;
                }
                if (ctx->map.level == LEVEL_2 && ctx->enemy.alive &&
                  hasIntersection(ctx->player2.rect, ctx->enemy.rect)) {
                  ctx->enemy.healthStatus++;
if (ctx->enemy.healthStatus >= ctx->enemy.maxHealth)
    ctx->enemy.healthStatus++;
ctx->enemy.knockbackX = (ctx->player2.lastHDir == SDL_SCANCODE_RIGHT) ? 120.0f : -120.0f;
ctx->enemy.knockbackY     = 0;
ctx->enemy.knockbackTimer = 12;
if (ctx->enemy.healthStatus >= ctx->enemy.maxHealth)
    ctx->enemy.alive = 0;
              }

                if (ctx->player2.attackFrame >= 6) {
                    ctx->player2.attacking   = 0;
                    ctx->player2.attackFrame = 0;
                } else {
                    ctx->player2.currentState =
                        (ctx->player2.lastHDir == SDL_SCANCODE_LEFT) ? ctx->player2.attackLeft[ctx->player2.attackFrame]
                                                                    : ctx->player2.attackRight[ctx->player2.attackFrame];
                }
            }
        }
    }

    /* ── Key pickup ── */
Key *keys     = (ctx->map.level == LEVEL_1) ? ctx->map.keys1     : ctx->map.keys2;
int  keys_cnt = (ctx->map.level == LEVEL_1) ? ctx->map.keys1_cnt : ctx->map.keys2_cnt;

for (int i = 0; i < keys_cnt; i++) {
    if (!keys[i].collected && keys[i].visible) {
        if (map_rects_overlap(ctx->player1.rect, keys[i].rect) ||
            map_rects_overlap(ctx->player2.rect, keys[i].rect)) {
            
            keys[i].collected = 1;
            keys[i].visible   = 0;

            if (ctx->map.level == LEVEL_1 && i == 0) {
    ctx->isCameraPanning  = 1;
    ctx->cameraFocusTimer = 120;
    ctx->cameraTarget.x   = doors[0].rect.x + doors[0].rect.w / 2;
    ctx->cameraTarget.y   = doors[0].rect.y + doors[0].rect.h / 2;
}
        }
    }
}

/* ── Door triggers ── */
for (int i = 0; i < dc; i++) {
    int p1_at = hasIntersection(ctx->player1.rect, doors[i].rect);
    int p2_at = hasIntersection(ctx->player2.rect, doors[i].rect);

    if (doors[i].locked) {
        int kid = doors[i].key_id;
        if (kid >= 0 && kid < keys_cnt && keys[kid].collected) {
            door_open[i]    = 1;
            doors[i].locked = 0;
        }
    } else {
        if ((p1_at || p2_at) && doors[i].to_level == LEVEL_2) {
          ctx->map.level = LEVEL_2;
          setup_level2(&ctx->map);
          ctx->minimap.num_level  = 2;  
          ctx->minimap2.num_level = 2;   
          ctx->player1.rect.x = 200; ctx->player1.rect.y = 550;
          ctx->player2.rect.x = 230; ctx->player2.rect.y = 550;
          Mix_HaltMusic();
          Mix_PlayMusic(ctx->musicLevel2, -1);
          ctx->currentState    = STATE_CUTSCENE_L2_INTRO;
          ctx->cutsceneL2Timer = 0;
          ctx->cutsceneL2Alpha = 0;
          ctx->paused          = 0;
          return;
    }
    if ((p1_at || p2_at) && doors[i].to_level == LEVEL_1) {
        ctx->map.level = LEVEL_1;
        setup_level1(&ctx->map);
        ctx->player1.rect.x = 250; ctx->player1.rect.y = 650;
        ctx->player2.rect.x = 300; ctx->player2.rect.y = 650;
        return;
    }
    }
}



}

SDL_Rect scale_rect(SDL_Rect rect, float scale)
{
    float w = rect.w, h = rect.h;
    float sw = w * scale, sh = h * scale;
    return (SDL_Rect){
        rect.x + (int)((w - sw) / 2),
        rect.y + (int)((h - sh) / 2),
        (int)sw, (int)sh
    };
}

int point_in_rect(int x, int y, SDL_Rect *rect)
{
    return x >= rect->x && x <= rect->x + rect->w &&
           y >= rect->y && y <= rect->y + rect->h;
}

void subMenuFn(GameContext *ctx)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    Button *btns[] = {
        &ctx->sm.resumeBtn, &ctx->sm.saveBtn, &ctx->sm.loadBtn,
        &ctx->sm.playerBtn, &ctx->sm.scoreBtn, &ctx->sm.quitBtn
    };
    for (int i = 0; i < 6; i++) {
        SDL_Rect draw = point_in_rect(mx, my, &btns[i]->rect)
            ? scale_rect(btns[i]->rect, 1.1f) : btns[i]->rect;
        SDL_RenderCopy(ctx->renderer, btns[i]->tex, NULL, &draw);
    }
    for (int i = 0; i < 6; i++) {
        int over = point_in_rect(mx, my, &btns[i]->rect);
        if (over && !btns[i]->hovered) { Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0); btns[i]->hovered = 1; }
        else if (!over) btns[i]->hovered = 0;
    }
}

void playersMenuFn(GameContext *ctx)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    Button *btns[] = { &ctx->sm.charSelectBtn, &ctx->sm.outfitsBtn, &ctx->sm.buttonsBtn };
    for (int i = 0; i < 3; i++) {
        SDL_Rect draw = point_in_rect(mx, my, &btns[i]->rect)
            ? scale_rect(btns[i]->rect, 1.1f) : btns[i]->rect;
        SDL_RenderCopy(ctx->renderer, btns[i]->tex, NULL, &draw);
    }
    for (int i = 0; i < 3; i++) {
        int over = point_in_rect(mx, my, &btns[i]->rect);
        if (over && !btns[i]->hovered) { Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0); btns[i]->hovered = 1; }
        else if (!over) btns[i]->hovered = 0;
    }

    SDL_Rect drawBack = point_in_rect(mx, my, &ctx->sm.backBtn.rect)
        ? scale_rect(ctx->sm.backBtn.rect, 1.1f) : ctx->sm.backBtn.rect;
    SDL_RenderCopy(ctx->renderer, ctx->sm.backBtn.tex, NULL, &drawBack);
    int overBack = point_in_rect(mx, my, &ctx->sm.backBtn.rect);
    if (overBack && !ctx->sm.backBtn.hovered) { Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0); ctx->sm.backBtn.hovered = 1; }
    else if (!overBack) ctx->sm.backBtn.hovered = 0;
}

void changeOutfitsFn(GameContext *ctx)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    SDL_RenderCopy(ctx->renderer, ctx->sm.playersBg, NULL, NULL);

    SDL_Color white = {255,255,255,255}, gold = {180,150,80,255};

    /* P1 outfit buttons */
    Button *btns[] = { &ctx->sm.p1o1Btn, &ctx->sm.p1o2Btn };
    for (int i = 0; i < 2; i++) {
        SDL_Rect draw = point_in_rect(mx,my,&btns[i]->rect) ? scale_rect(btns[i]->rect,1.1f) : btns[i]->rect;
        SDL_RenderCopy(ctx->renderer, btns[i]->tex, NULL, &draw);
    }
    for (int i = 0; i < 2; i++) {
        int over = point_in_rect(mx,my,&btns[i]->rect);
        if (over && !btns[i]->hovered) { Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0); btns[i]->hovered=1; }
        else if (!over) btns[i]->hovered=0;
    }

    /* ok button */
    SDL_Rect draw = point_in_rect(mx,my,&ctx->sm.okBtn.rect) ? scale_rect(ctx->sm.okBtn.rect,1.1f) : ctx->sm.okBtn.rect;
    SDL_RenderCopy(ctx->renderer, ctx->sm.okBtn.tex, NULL, &draw);
    int over = point_in_rect(mx,my,&ctx->sm.okBtn.rect);
    if (over && !ctx->sm.okBtn.hovered) { Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0); ctx->sm.okBtn.hovered=1; }
    else if (!over) ctx->sm.okBtn.hovered=0;

    /* P1 labels */
    const char *label1 = ctx->player1.selectedOutfit==1 ? "Outfit 1 Selected" : "Outfit 1";
    const char *label2 = ctx->player1.selectedOutfit==2 ? "Outfit 2 Selected" : "Outfit 2";
    SDL_Surface *s; SDL_Texture *t; SDL_Rect d;

    s = TTF_RenderText_Blended(ctx->font, label1, ctx->player1.selectedOutfit==1 ? gold : white);
    t = SDL_CreateTextureFromSurface(ctx->renderer, s); SDL_FreeSurface(s);
    SDL_QueryTexture(t,NULL,NULL,&d.w,&d.h);
    d.x = ctx->sm.p1o1Btn.rect.x + (ctx->sm.p1o1Btn.rect.w - d.w)/2;
    d.y = ctx->sm.p1o1Btn.rect.y + ctx->sm.p1o1Btn.rect.h + 5;
    SDL_RenderCopy(ctx->renderer,t,NULL,&d); SDL_DestroyTexture(t);

    s = TTF_RenderText_Blended(ctx->font, label2, ctx->player1.selectedOutfit==2 ? gold : white);
    t = SDL_CreateTextureFromSurface(ctx->renderer, s); SDL_FreeSurface(s);
    SDL_QueryTexture(t,NULL,NULL,&d.w,&d.h);
    d.x = ctx->sm.p1o2Btn.rect.x + (ctx->sm.p1o2Btn.rect.w - d.w)/2;
    d.y = ctx->sm.p1o2Btn.rect.y + ctx->sm.p1o2Btn.rect.h + 5;
    SDL_RenderCopy(ctx->renderer,t,NULL,&d); SDL_DestroyTexture(t);

    /* P2 outfit buttons */
    Button *btns2[] = { &ctx->sm.p2o1Btn, &ctx->sm.p2o2Btn };
    for (int i = 0; i < 2; i++) {
        SDL_Rect draw2 = point_in_rect(mx,my,&btns2[i]->rect) ? scale_rect(btns2[i]->rect,1.1f) : btns2[i]->rect;
        SDL_RenderCopy(ctx->renderer, btns2[i]->tex, NULL, &draw2);
    }
    for (int i = 0; i < 2; i++) {
        int over2 = point_in_rect(mx,my,&btns2[i]->rect);
        if (over2 && !btns2[i]->hovered) { Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0); btns2[i]->hovered=1; }
        else if (!over2) btns2[i]->hovered=0;
    }

    /* P2 labels */
    const char *label3 = ctx->player2.selectedOutfit==1 ? "Outfit 1 Selected" : "Outfit 1";
    const char *label4 = ctx->player2.selectedOutfit==2 ? "Outfit 2 Selected" : "Outfit 2";

    s = TTF_RenderText_Blended(ctx->font, label3, ctx->player2.selectedOutfit==1 ? gold : white);
    t = SDL_CreateTextureFromSurface(ctx->renderer, s); SDL_FreeSurface(s);
    SDL_QueryTexture(t,NULL,NULL,&d.w,&d.h);
    d.x = ctx->sm.p2o1Btn.rect.x + (ctx->sm.p2o1Btn.rect.w - d.w)/2;
    d.y = ctx->sm.p2o1Btn.rect.y + ctx->sm.p2o1Btn.rect.h + 5;
    SDL_RenderCopy(ctx->renderer,t,NULL,&d); SDL_DestroyTexture(t);

    s = TTF_RenderText_Blended(ctx->font, label4, ctx->player2.selectedOutfit==2 ? gold : white);
    t = SDL_CreateTextureFromSurface(ctx->renderer, s); SDL_FreeSurface(s);
    SDL_QueryTexture(t,NULL,NULL,&d.w,&d.h);
    d.x = ctx->sm.p2o2Btn.rect.x + (ctx->sm.p2o2Btn.rect.w - d.w)/2;
    d.y = ctx->sm.p2o2Btn.rect.y + ctx->sm.p2o2Btn.rect.h + 5;
    SDL_RenderCopy(ctx->renderer,t,NULL,&d); SDL_DestroyTexture(t);
}

void charSelectFn(GameContext *ctx)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    SDL_RenderCopy(ctx->renderer, ctx->sm.charSelectBg, NULL, NULL);

    SDL_Rect p1card = {130,180,220,270}, p2card = {630,180,220,270};
    SDL_Texture *p1tex = (ctx->player1.selectedChar==1) ? ctx->sm.p1preview : ctx->sm.p2preview;
    SDL_Texture *p2tex = (ctx->player2.selectedChar==2) ? ctx->sm.p2preview : ctx->sm.p1preview;
    SDL_RenderCopy(ctx->renderer, p1tex, NULL, &p1card);
    SDL_RenderCopy(ctx->renderer, p2tex, NULL, &p2card);

    SDL_Color gold = {180,150,80,255};
    SDL_Surface *s; SDL_Texture *t; SDL_Rect d;

    const char *p1label = (ctx->player1.selectedChar==1) ? "P1: Character 1" : "P1: Character 2";
    s = TTF_RenderText_Blended(ctx->font, p1label, gold);
    t = SDL_CreateTextureFromSurface(ctx->renderer,s); SDL_FreeSurface(s);
    SDL_QueryTexture(t,NULL,NULL,&d.w,&d.h);
    d.x = 150+(180-d.w)/2; d.y = 430;
    SDL_RenderCopy(ctx->renderer,t,NULL,&d); SDL_DestroyTexture(t);

    const char *p2label = (ctx->player2.selectedChar==2) ? "P2: Character 2" : "P2: Character 1";
    s = TTF_RenderText_Blended(ctx->font, p2label, gold);
    t = SDL_CreateTextureFromSurface(ctx->renderer,s); SDL_FreeSurface(s);
    SDL_QueryTexture(t,NULL,NULL,&d.w,&d.h);
    d.x = 650+(180-d.w)/2; d.y = 430;
    SDL_RenderCopy(ctx->renderer,t,NULL,&d); SDL_DestroyTexture(t);

    Button *swapBtns[] = { &ctx->sm.p1SwapBtn, &ctx->sm.p2SwapBtn };
    for (int i = 0; i < 2; i++) {
        SDL_Rect draw2 = point_in_rect(mx,my,&swapBtns[i]->rect) ? scale_rect(swapBtns[i]->rect,1.1f) : swapBtns[i]->rect;
        SDL_RenderCopy(ctx->renderer, swapBtns[i]->tex, NULL, &draw2);
        int over2 = point_in_rect(mx,my,&swapBtns[i]->rect);
        if (over2 && !swapBtns[i]->hovered) { Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0); swapBtns[i]->hovered=1; }
        else if (!over2) swapBtns[i]->hovered=0;
    }

    SDL_Rect draw = point_in_rect(mx,my,&ctx->sm.okBtn.rect) ? scale_rect(ctx->sm.okBtn.rect,1.1f) : ctx->sm.okBtn.rect;
    SDL_RenderCopy(ctx->renderer, ctx->sm.okBtn.tex, NULL, &draw);
}

void game_update(GameContext *ctx)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    static Uint32 lastTick = 0;
    Uint32 now = SDL_GetTicks();
    float dt = lastTick == 0 ? 0.016f : (now - lastTick) / 1000.0f;
    lastTick = now;

    while (SDL_PollEvent(&ctx->event)) {
        if (ctx->event.type == SDL_QUIT) { ctx->running = 0; }
        else if (ctx->event.type == SDL_KEYDOWN)
            ctx->keys[ctx->event.key.keysym.scancode] = 1;
        else if (ctx->event.type == SDL_KEYUP)
            ctx->keys[ctx->event.key.keysym.scancode] = 0;
        else if (ctx->event.type == SDL_MOUSEBUTTONUP)
            ctx->sm.playerBtnSwitched = 0;
        else if (ctx->event.type == SDL_MOUSEBUTTONDOWN && ctx->paused) {
            if (ctx->currentState == STATE_PAUSED_MAIN) {
                if (point_in_rect(mx,my,&ctx->sm.resumeBtn.rect)) {
                    ctx->paused = 0; ctx->currentState = STATE_PLAYING;
                    Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0);
                } else if (point_in_rect(mx,my,&ctx->sm.playerBtn.rect) && !ctx->sm.playerBtnSwitched) {
                    ctx->currentState = STATE_PAUSED_PLAYERS;
                    Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0);
                    ctx->sm.playerBtnSwitched = 1;
                }
            } else if (ctx->currentState == STATE_PAUSED_PLAYERS) {
                if (point_in_rect(mx,my,&ctx->sm.outfitsBtn.rect)) {
                    ctx->currentState = STATE_PAUSED_OUTFITS;
                    Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0);
                }
                SDL_Rect backBtn = {60,455,BUTTON_W,BUTTON_H};
                if (point_in_rect(mx,my,&backBtn)) {
                    ctx->currentState = STATE_PAUSED_MAIN;
                    Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0);
                }
                if (point_in_rect(mx,my,&ctx->sm.charSelectBtn.rect)) {
                    ctx->currentState = STATE_PAUSED_CHARSELECT;
                    Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0);
                }
            } else if (ctx->currentState == STATE_PAUSED_OUTFITS) {
                if (point_in_rect(mx,my,&ctx->sm.p1o1Btn.rect)) {
                    ctx->player1.selectedOutfit = 1; Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0);
                } else if (point_in_rect(mx,my,&ctx->sm.p1o2Btn.rect)) {
                    ctx->player1.selectedOutfit = 2; Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0);
                } else if (point_in_rect(mx,my,&ctx->sm.p2o1Btn.rect)) {
                    ctx->player2.selectedOutfit = 1; Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0);
                } else if (point_in_rect(mx,my,&ctx->sm.p2o2Btn.rect)) {
                    ctx->player2.selectedOutfit = 2; Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0);
                } else if (point_in_rect(mx,my,&ctx->sm.okBtn.rect)) {
                    initOutfit(ctx,1,ctx->player1.selectedOutfit,ctx->player1.selectedChar);
                    initOutfit(ctx,2,ctx->player2.selectedOutfit,ctx->player2.selectedChar);
                    ctx->currentState = STATE_PAUSED_PLAYERS;
                    Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0);
                }
                SDL_Rect backBtn = {60,455,BUTTON_W,BUTTON_H};
                if (point_in_rect(mx,my,&backBtn)) {
                    ctx->currentState = STATE_PAUSED_PLAYERS;
                    Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0);
                }
            } else if (ctx->currentState == STATE_PAUSED_CHARSELECT) {
                if (point_in_rect(mx,my,&ctx->sm.p1SwapBtn.rect) ||
                    point_in_rect(mx,my,&ctx->sm.p2SwapBtn.rect)) {
                    int tmp = ctx->player1.selectedChar;
                    ctx->player1.selectedChar = ctx->player2.selectedChar;
                    ctx->player2.selectedChar = tmp;
                    Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0);
                } else if (point_in_rect(mx,my,&ctx->sm.okBtn.rect)) {
                    initOutfit(ctx,1,ctx->player1.selectedOutfit,ctx->player1.selectedChar);
                    initOutfit(ctx,2,ctx->player2.selectedOutfit,ctx->player2.selectedChar);
                    ctx->currentState = STATE_PAUSED_PLAYERS;
                    Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0);
                }
                SDL_Rect backBtn = {60,455,BUTTON_W,BUTTON_H};
                if (point_in_rect(mx,my,&backBtn)) {
                    ctx->currentState = STATE_PAUSED_PLAYERS;
                    Mix_PlayChannel(CH_BUTTONS,ctx->sm.hoverSound,0);
                }
            }
        }
    }

    if (ctx->keys[SDL_SCANCODE_ESCAPE] && !ctx->pauseSwitched) {
        ctx->paused = !ctx->paused;
        if (ctx->paused) Mix_HaltChannel(-1);
        ctx->currentState  = ctx->paused ? STATE_PAUSED_MAIN : STATE_PLAYING;
        ctx->pauseSwitched = 1;
    }
    if (!ctx->keys[SDL_SCANCODE_ESCAPE]) ctx->pauseSwitched = 0;

    if (ctx->currentState == STATE_CUTSCENE) {
    ctx->cutsceneTimer++;
    // fade in: 0-60, hold: 60-300, fade out: 300-360
    if      (ctx->cutsceneTimer <= 60)  ctx->cutsceneAlpha = (int)(ctx->cutsceneTimer * 255.0f / 60.0f);
    else if (ctx->cutsceneTimer <= 300) ctx->cutsceneAlpha = 255;
    else if (ctx->cutsceneTimer <= 360) ctx->cutsceneAlpha = (int)((360 - ctx->cutsceneTimer) * 255.0f / 60.0f);
    else {
        ctx->currentState  = STATE_PLAYING;
        ctx->cutsceneTimer = 0;
    }
    return;  // skip playerMechanics while cutscene plays
}
if (ctx->currentState == STATE_CUTSCENE_L2_INTRO) {
    ctx->cutsceneL2Timer++;
    if      (ctx->cutsceneL2Timer <= 60)  ctx->cutsceneL2Alpha = (int)(ctx->cutsceneL2Timer * 255.0f / 60.0f);
    else if (ctx->cutsceneL2Timer <= 340) ctx->cutsceneL2Alpha = 255;
    else if (ctx->cutsceneL2Timer <= 400) ctx->cutsceneL2Alpha = (int)((400 - ctx->cutsceneL2Timer) * 255.0f / 60.0f);
    else {
        ctx->currentState    = STATE_PLAYING;
        ctx->cutsceneL2Timer = 0;
    }
    return;
}

if (ctx->currentState == STATE_CUTSCENE_L2_ENDING) {
    ctx->cutsceneL2Timer++;
    if      (ctx->cutsceneL2Timer <= 60)  ctx->cutsceneL2Alpha = (int)(ctx->cutsceneL2Timer * 255.0f / 60.0f);
    else if (ctx->cutsceneL2Timer <= 400) ctx->cutsceneL2Alpha = 255;
    else if (ctx->cutsceneL2Timer <= 460) ctx->cutsceneL2Alpha = (int)((460 - ctx->cutsceneL2Timer) * 255.0f / 60.0f);
    else {
        ctx->running = 0; /* or go to a credits screen */
    }
    return;
}
    playerMechanics(ctx);
    if (ctx->map.level == LEVEL_2)
        enemy_update(ctx, dt);
        if (ctx->map.level == LEVEL_2)
            enemy_update(ctx, dt);

            /* ── Level 2 ending cutscene trigger ── */
            if (ctx->map.level == LEVEL_2 && !ctx->enemy.alive &&
                ctx->currentState == STATE_PLAYING) {
                ctx->currentState    = STATE_CUTSCENE_L2_ENDING;
                ctx->cutsceneL2Timer = 0;
                ctx->cutsceneL2Alpha = 0;
            }
    if (ctx->map.level == LEVEL_2)
    printf("enemy pos: %.1f %.1f alive:%d\n", ctx->enemy.x, ctx->enemy.y, ctx->enemy.alive);
    printf("atlas check: %p %p %p %p\n",
    ctx->enemyAtlas.walkRight[0],
    ctx->enemyAtlas.walkDown[0],
    ctx->enemyAtlas.walkUp[0],
    ctx->enemyAtlas.attack[0]);
    
    if (ctx->isCameraPanning && !ctx->paused) {
    ctx->cameraFocusTimer--;
    if (ctx->cameraFocusTimer <= 0) {
        ctx->isCameraPanning  = 0;
        ctx->cameraFocusTimer = 0;
    }
}


    if (ctx->map.level == LEVEL_1)
        update_falling_box(&ctx->map,
            ctx->player1.rect.x, ctx->player1.rect.y,
            ctx->player2.rect.x, ctx->player2.rect.y);

    MAJ_minimap(&ctx->minimap,  ctx->player1.rect.x, ctx->player1.rect.y,
                                 ctx->player2.rect.x, ctx->player2.rect.y);
    MAJ_minimap(&ctx->minimap2, ctx->player1.rect.x, ctx->player1.rect.y,
                                 ctx->player2.rect.x, ctx->player2.rect.y);

    minimap_update_shake(&ctx->minimap);
    minimap_update_shake(&ctx->minimap2);
}

void game_cleanup(GameContext *ctx)
{
    if (!ctx) return;

    Mix_FreeMusic(ctx->musicLevel1);
    Mix_FreeMusic(ctx->musicLevel2);

    for (int i=0;i<MAX_WALK_RIGHT;   i++) SDL_DestroyTexture(ctx->enemyAtlas.walkRight[i]);
    for (int i=0;i<MAX_WALK_UP;      i++) SDL_DestroyTexture(ctx->enemyAtlas.walkUp[i]);
    for (int i=0;i<MAX_WALK_DOWN;    i++) SDL_DestroyTexture(ctx->enemyAtlas.walkDown[i]);
    for (int i=0;i<MAX_ATTACK_FRAMES;i++) SDL_DestroyTexture(ctx->enemyAtlas.attack[i]);

    SDL_DestroyTexture(ctx->player1.idle);
    for (int i=0;i<5;i++) SDL_DestroyTexture(ctx->player1.walkRight[i]);
    for (int i=0;i<5;i++) SDL_DestroyTexture(ctx->player1.walkLeft[i]);
    for (int i=0;i<2;i++) SDL_DestroyTexture(ctx->player1.walkUp[i]);
    for (int i=0;i<2;i++) SDL_DestroyTexture(ctx->player1.walkDown[i]);
    for (int i=0;i<8;i++) SDL_DestroyTexture(ctx->player1.hpBar[i]);
    for (int i=0;i<6;i++) SDL_DestroyTexture(ctx->player1.attackRight[i]);
    for (int i=0;i<6;i++) SDL_DestroyTexture(ctx->player1.attackLeft[i]);

    SDL_DestroyTexture(ctx->player2.idle);
    for (int i=0;i<5;i++) SDL_DestroyTexture(ctx->player2.walkRight[i]);
    for (int i=0;i<5;i++) SDL_DestroyTexture(ctx->player2.walkLeft[i]);
    for (int i=0;i<2;i++) SDL_DestroyTexture(ctx->player2.walkUp[i]);
    for (int i=0;i<2;i++) SDL_DestroyTexture(ctx->player2.walkDown[i]);
    for (int i=0;i<8;i++) SDL_DestroyTexture(ctx->player2.hpBar[i]);
    for (int i=0;i<6;i++) SDL_DestroyTexture(ctx->player2.attackRight[i]);
    for (int i=0;i<6;i++) SDL_DestroyTexture(ctx->player2.attackLeft[i]);

    Mix_FreeChunk(ctx->player1.walkingSound);
    Mix_FreeChunk(ctx->player1.runningSound);
    Mix_FreeChunk(ctx->player1.jumpingSound);
    Mix_FreeChunk(ctx->player1.attackingSound);
    Mix_FreeChunk(ctx->player1.gettingHitSound);
    Mix_FreeChunk(ctx->player1.deathSound);
    Mix_FreeChunk(ctx->player2.walkingSound);
    Mix_FreeChunk(ctx->player2.runningSound);
    Mix_FreeChunk(ctx->player2.jumpingSound);
    Mix_FreeChunk(ctx->player2.attackingSound);
    Mix_FreeChunk(ctx->player2.gettingHitSound);
    Mix_FreeChunk(ctx->player2.deathSound);

    map_cleanup(&ctx->map);
    liberer_minimap(&ctx->minimap);
    liberer_minimap(&ctx->minimap2);

    if (ctx->font)     TTF_CloseFont(ctx->font);
    if (ctx->renderer) SDL_DestroyRenderer(ctx->renderer);
    if (ctx->window)   SDL_DestroyWindow(ctx->window);
    Mix_CloseAudio(); Mix_Quit(); IMG_Quit(); TTF_Quit(); SDL_Quit();
    free(ctx);
}

void game_render(GameContext *ctx)
{
    const int halfW = WINDOW_WIDTH  / 2;   /* 500 */
    const int fullH = WINDOW_HEIGHT;        /* 650 */
 
    /* How many world pixels fit in each viewport at ZOOM_FACTOR */
    const int viewW = (int)(halfW / ZOOM_FACTOR);   /* 250 */
    const int viewH = (int)(fullH / ZOOM_FACTOR);   /* 325 */

    if (ctx->currentState == STATE_CUTSCENE) {
    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ctx->renderer);

    // split into two lines for readability
    const char *line1 = "Londres, 1984.";
    const char *line2 = "Un crime odieux vient d'ebranler la ville :";
    const char *line3 = "un meurtre dans un manoir huppe.";
    const char *line4 = "Sherlock Holmes et son fidele compagnon";
    const char *line5 = "n'ont qu'une mission... elucider l'affaire.";

    const char *lines[] = {line1, line2, line3, line4, line5};
    int lineCount = 5;
    int lineH = 40;
    int startY = WINDOW_HEIGHT / 2 - (lineCount * lineH) / 2;

    for (int i = 0; i < lineCount; i++) {
        SDL_Surface *surf = TTF_RenderText_Blended(ctx->font, lines[i],
            (SDL_Color){255, 255, 255, (Uint8)ctx->cutsceneAlpha});
        SDL_Texture *tex = SDL_CreateTextureFromSurface(ctx->renderer, surf);
        SDL_FreeSurface(surf);
        SDL_SetTextureAlphaMod(tex, (Uint8)ctx->cutsceneAlpha);
        int tw, th;
        SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
        SDL_Rect dst = { (WINDOW_WIDTH - tw) / 2, startY + i * lineH, tw, th };
        SDL_RenderCopy(ctx->renderer, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }

    SDL_RenderPresent(ctx->renderer);
    return;  // skip the rest of game_render
}

/* ── Level 2 intro cutscene ── */
if (ctx->currentState == STATE_CUTSCENE_L2_INTRO) {
    SDL_SetRenderDrawColor(ctx->renderer, 5, 0, 10, 255);
    SDL_RenderClear(ctx->renderer);

    const char *lines[] = {
        "Les indices convergaient tous...",
        "vers un seul et meme coupable.",
        " ",
        "Sherlock Holmes lui-meme.",
        " ",
        "Au fond des cachots obscurs,",
        "il etait temps d'affronter la verite —",
        "et de se battre contre ses propres demons."
    };
    int lineCount = 8;
    int lineH     = 38;
    int startY    = WINDOW_HEIGHT / 2 - (lineCount * lineH) / 2;
    Uint8 alpha   = (Uint8)ctx->cutsceneL2Alpha;

    for (int i = 0; i < lineCount; i++) {
        if (lines[i][0] == ' ') continue; /* blank spacer line */
        SDL_Color col;
        if (i == 3) /* "Sherlock Holmes lui-meme" — highlight in blood red */
            col = (SDL_Color){200, 30, 30, alpha};
        else if (i == 0 || i == 1)
            col = (SDL_Color){180, 160, 220, alpha}; /* cold purple-white */
        else
            col = (SDL_Color){210, 200, 230, alpha}; /* pale mysterious */

        SDL_Surface *surf = TTF_RenderText_Blended(ctx->font, lines[i], col);
        SDL_Texture *tex  = SDL_CreateTextureFromSurface(ctx->renderer, surf);
        SDL_FreeSurface(surf);
        SDL_SetTextureAlphaMod(tex, alpha);
        int tw, th;
        SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
        SDL_Rect dst = { (WINDOW_WIDTH - tw) / 2, startY + i * lineH, tw, th };
        SDL_RenderCopy(ctx->renderer, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }

    SDL_RenderPresent(ctx->renderer);
    return;
}

/* ── Level 2 ending cutscene ── */
if (ctx->currentState == STATE_CUTSCENE_L2_ENDING) {
    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ctx->renderer);

    const char *lines[] = {
        "— FIN —",
        " ",
        "Vaincu et brise,",
        "Sherlock Holmes s'est rendu a la justice.",
        " ",
        "Les enqueteurs ont decouvert la verite :",
        "c'etait lui, le coupable.",
        " ",
        "Atteint de schizophrenie,",
        "il n'avait jamais su...",
        "que le monstre qu'il traquait",
        "n'etait autre que lui-meme."
    };
    int lineCount = 12;
    int lineH     = 36;
    int startY    = WINDOW_HEIGHT / 2 - (lineCount * lineH) / 2;
    Uint8 alpha   = (Uint8)ctx->cutsceneL2Alpha;

    for (int i = 0; i < lineCount; i++) {
        if (lines[i][0] == ' ') continue;
        SDL_Color col;
        if (i == 0) /* "— FIN —" in gold */
            col = (SDL_Color){200, 170, 60, alpha};
        else if (i == 8 || i == 9 || i == 10 || i == 11) /* schizophrenia reveal — cold blue */
            col = (SDL_Color){120, 160, 220, alpha};
        else
            col = (SDL_Color){220, 215, 225, alpha};

        SDL_Surface *surf = TTF_RenderText_Blended(ctx->font, lines[i], col);
        SDL_Texture *tex  = SDL_CreateTextureFromSurface(ctx->renderer, surf);
        SDL_FreeSurface(surf);
        SDL_SetTextureAlphaMod(tex, alpha);
        int tw, th;
        SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
        SDL_Rect dst = { (WINDOW_WIDTH - tw) / 2, startY + i * lineH, tw, th };
        SDL_RenderCopy(ctx->renderer, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }

    SDL_RenderPresent(ctx->renderer);
    return;
}
 
    SDL_RenderSetViewport(ctx->renderer, NULL);
    SDL_SetRenderDrawColor(ctx->renderer, 20, 20, 20, 255);
    SDL_RenderClear(ctx->renderer);
 
    /* ── Per-player camera: snap instantly, then clamp ── */
    /* Camera top-left in WORLD space */
    int cam[2][2]; /* cam[side][0]=x, cam[side][1]=y */
 
    /* Player 1 camera */
    cam[0][0] = (ctx->player1.rect.x + ctx->player1.rect.w/2) - viewW/2;
    cam[0][1] = (ctx->player1.rect.y + ctx->player1.rect.h/2) - viewH/2;
 
    /* Player 2 camera */
    cam[1][0] = (ctx->player2.rect.x + ctx->player2.rect.w/2) - viewW/2;
    cam[1][1] = (ctx->player2.rect.y + ctx->player2.rect.h/2) - viewH/2;
    
    if (ctx->isCameraPanning) {
    int doorCamX = ctx->cameraTarget.x - viewW / 2;
    int doorCamY = ctx->cameraTarget.y - viewH / 2;
    int t = ctx->cameraFocusTimer;
    float lerp;
    if      (t > 80) lerp = (float)(120 - t) / 40.0f;
    else if (t > 40) lerp = 1.0f;
    else             lerp = (float)t / 40.0f;
    for (int s = 0; s < 2; s++) {
        cam[s][0] = (int)(cam[s][0] * (1.0f - lerp) + doorCamX * lerp);
        cam[s][1] = (int)(cam[s][1] * (1.0f - lerp) + doorCamY * lerp);
    }
}

 
    /* Clamp both cameras so we never go outside the map */
    for (int s = 0; s < 2; s++) {
        if (cam[s][0] < 0)              cam[s][0] = 0;
        if (cam[s][1] < 0)              cam[s][1] = 0;
        if (cam[s][0] > MAP_W - viewW)  cam[s][0] = MAP_W - viewW;
        if (cam[s][1] > MAP_H - viewH)  cam[s][1] = MAP_H - viewH;
    }
 
    SDL_Rect viewports[2] = {
        {0,     0, halfW, fullH},
        {halfW, 0, halfW, fullH}
    };
 
    for (int side = 0; side < 2; side++) {
        int camX = cam[side][0];
        int camY = cam[side][1];
 
        SDL_RenderSetViewport(ctx->renderer, &viewports[side]);
 
        /* ── Map ── */
        SDL_Texture *tex_map = (ctx->map.level == LEVEL_1)
                               ? ctx->map.tex_map1 : ctx->map.tex_map2;
 
        /* Source: a viewW×viewH window into the world */
        SDL_Rect map_src = { camX, camY, viewW, viewH };
        /* Dest: fill the entire viewport (this is the 2× zoom stretch) */
        SDL_Rect map_dst = { 0, 0, halfW, fullH };
        SDL_RenderCopy(ctx->renderer, tex_map, &map_src, &map_dst);

       
        /* ── Falling box (level 1 only) ── */
        if (ctx->map.level == LEVEL_1) {
            FallingBox *fb = &ctx->map.fbox;
            /* Convert world pos → screen pos inside this viewport */
            SDL_Rect box_dst = {
                (int)((fb->rect.x - camX) * ZOOM_FACTOR),
                (int)((fb->rect.y - camY) * ZOOM_FACTOR),
                (int)(fb->rect.w * ZOOM_FACTOR),
                (int)(fb->rect.h * ZOOM_FACTOR)
            };
            SDL_Texture *btex = (fb->state == BOX_BROKEN)
                                ? ctx->map.tex_broken_box : ctx->map.tex_box;
            SDL_RenderCopy(ctx->renderer, btex, NULL, &box_dst);
        }
        
        /* ── Keys ── */
        {
            Key *keys     = (ctx->map.level == LEVEL_1) ? ctx->map.keys1     : ctx->map.keys2;
            int  keys_cnt = (ctx->map.level == LEVEL_1) ? ctx->map.keys1_cnt : ctx->map.keys2_cnt;

            SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(ctx->renderer, 255, 215, 0, 255);
            for (int i = 0; i < keys_cnt; i++) {
                if (keys[i].visible && !keys[i].collected) {
                    SDL_Rect kr = {
                        (int)((keys[i].rect.x - camX) * ZOOM_FACTOR),
                        (int)((keys[i].rect.y - camY) * ZOOM_FACTOR),
                        (int)(keys[i].rect.w  * ZOOM_FACTOR),
                        (int)(keys[i].rect.h  * ZOOM_FACTOR)
                    };
                    SDL_RenderFillRect(ctx->renderer, &kr);
                }
            }
        }

        /* ── Shadows ── */
        SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 80);
 
        if (ctx->player1.alive) {
            SDL_Rect sh = {
                (int)((ctx->player1.rect.x - camX) * ZOOM_FACTOR) + 5,
                (int)((ctx->player1.rect.y - camY) * ZOOM_FACTOR)
                    + (int)(ctx->player1.rect.h * ZOOM_FACTOR) - 5,
                (int)(ctx->player1.rect.w * ZOOM_FACTOR) - 10,
                8
            };
            SDL_RenderFillRect(ctx->renderer, &sh);
        }
        if (ctx->player2.alive) {
            SDL_Rect sh = {
                (int)((ctx->player2.rect.x - camX) * ZOOM_FACTOR) + 5,
                (int)((ctx->player2.rect.y - camY) * ZOOM_FACTOR)
                    + (int)(ctx->player2.rect.h * ZOOM_FACTOR) - 5,
                (int)(ctx->player2.rect.w * ZOOM_FACTOR) - 10,
                8
            };
            SDL_RenderFillRect(ctx->renderer, &sh);
        }
 
        /* ── Players ── */
        if (ctx->player1.alive) {
            SDL_Rect r = {
                (int)((ctx->player1.rect.x - camX) * ZOOM_FACTOR),
                (int)((ctx->player1.rect.y - camY) * ZOOM_FACTOR)
                    - ctx->player1.jumpOffset,
                (int)(ctx->player1.rect.w * ZOOM_FACTOR),
                (int)(ctx->player1.rect.h * ZOOM_FACTOR)
            };
            SDL_RenderCopy(ctx->renderer, ctx->player1.currentState, NULL, &r);
        }
        if (ctx->player2.alive) {
            SDL_Rect r = {
                (int)((ctx->player2.rect.x - camX) * ZOOM_FACTOR),
                (int)((ctx->player2.rect.y - camY) * ZOOM_FACTOR)
                    - ctx->player2.jumpOffset,
                (int)(ctx->player2.rect.w * ZOOM_FACTOR),
                (int)(ctx->player2.rect.h * ZOOM_FACTOR)
            };
            SDL_RenderCopy(ctx->renderer, ctx->player2.currentState, NULL, &r);
        }

        if (ctx->map.level == LEVEL_2)
            enemy_render(ctx, camX, camY);
 
        /* ── HUD (score + HP bar — fixed screen positions) ── */
        char scoreText[32];
        snprintf(scoreText, sizeof(scoreText),
                 side == 0 ? "P1 Score: %d" : "P2 Score: %d",
                 side == 0 ? ctx->player1.score : ctx->player2.score);
 
        SDL_Surface *surf = TTF_RenderText_Blended(
            ctx->font, scoreText, (SDL_Color){255, 255, 255, 255});
        SDL_Texture *stx = SDL_CreateTextureFromSurface(ctx->renderer, surf);
        SDL_FreeSurface(surf);
        SDL_Rect sd; SDL_QueryTexture(stx, NULL, NULL, &sd.w, &sd.h);
        sd.x = 10; sd.y = 70;
        SDL_RenderCopy(ctx->renderer, stx, NULL, &sd);
        SDL_DestroyTexture(stx);
 
        SDL_Rect hp = {10, 20, PLAYER1HP_W, PLAYER1HP_H};
        SDL_RenderCopy(ctx->renderer,
            side == 0 ? ctx->player1.hpBar[ctx->player1.healthStatus]
                      : ctx->player2.hpBar[ctx->player2.healthStatus],
            NULL, &hp);
    }
 
    /* ── Minimap + divider line (drawn over both viewports) ── */
    SDL_RenderSetViewport(ctx->renderer, NULL);
    afficher_minimap(ctx->minimap,   ctx->renderer);
    afficher_minimap2(ctx->minimap2, ctx->renderer);
 
    SDL_SetRenderDrawColor(ctx->renderer, 80, 80, 80, 255);
    SDL_RenderDrawLine(ctx->renderer, halfW, 0, halfW, fullH);
 
    /* ── Pause overlay ── */
    if (ctx->paused) {
        SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 160);
        SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRect(ctx->renderer, &overlay);
        SDL_RenderCopy(ctx->renderer, ctx->sm.menuBoard, NULL, &ctx->sm.menuBoardRect);
        if      (ctx->currentState == STATE_PAUSED_MAIN)       subMenuFn(ctx);
        else if (ctx->currentState == STATE_PAUSED_PLAYERS)    playersMenuFn(ctx);
        else if (ctx->currentState == STATE_PAUSED_OUTFITS)    changeOutfitsFn(ctx);
        else if (ctx->currentState == STATE_PAUSED_CHARSELECT) charSelectFn(ctx);
    }
 
    SDL_RenderPresent(ctx->renderer);
}

void game_run(GameContext *ctx)
{
    while (ctx->running) {
        game_update(ctx);
        game_render(ctx);
    }
}
