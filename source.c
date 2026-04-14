#include "players.h"

SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, path);
    if (!texture) {
        printf("Failed to load texture: %s\n", IMG_GetError());
    }
    return texture;
}

void initOutfit(GameContext *ctx, int playerNum, int outfitNum, int charNum){
    char fullPath[64];
    Player *p;
    if (playerNum == 1)
    {
      p=&ctx->player1;
    }
    else 
    {
      p=&ctx->player2;
    }

    // idle
    sprintf(fullPath, "assets/player%d/outfit%d/idle.png", charNum, outfitNum);
    p->idle = loadTexture(fullPath, ctx->renderer);
    p->currentState = p->idle;

    // walk right / left
    for (int i = 0; i < 5; i++) {
        sprintf(fullPath, "assets/player%d/outfit%d/right%d.png", charNum, outfitNum, i+1);
        p->walkRight[i] = loadTexture(fullPath, ctx->renderer);

        sprintf(fullPath, "assets/player%d/outfit%d/left%d.png", charNum, outfitNum, i+1);
        p->walkLeft[i] = loadTexture(fullPath, ctx->renderer);
    }

    // walk up / down
    for (int i = 0; i < 2; i++) {
        sprintf(fullPath, "assets/player%d/outfit%d/up%d.png", charNum, outfitNum, i+1);
        p->walkUp[i] = loadTexture(fullPath, ctx->renderer);

        sprintf(fullPath, "assets/player%d/outfit%d/down%d.png", charNum, outfitNum, i+1);
        p->walkDown[i] = loadTexture(fullPath, ctx->renderer);
    }

    // attack right / left
    for (int i = 0; i < 6; i++) {
        sprintf(fullPath, "assets/player%d/outfit%d/attackRight%d.png", charNum, outfitNum, i+1);
        p->attackRight[i] = loadTexture(fullPath, ctx->renderer);

        sprintf(fullPath, "assets/player%d/outfit%d/attackLeft%d.png", charNum, outfitNum, i+1);
        p->attackLeft[i] = loadTexture(fullPath, ctx->renderer);
    }
}

void initPlayer1(GameContext *ctx)
{
    ctx->player1.outfitNum = 1;
    ctx->player1.outfitSwitched = 0;
    initOutfit(ctx, 1, 1, 1);
    
    ctx->player1.hpBar[0]  = loadTexture("assets/hpBar/hpBar1.png",  ctx->renderer);
    ctx->player1.hpBar[1]  = loadTexture("assets/hpBar/hpBar2.png",  ctx->renderer);
    ctx->player1.hpBar[2]  = loadTexture("assets/hpBar/hpBar3.png",  ctx->renderer);
    ctx->player1.hpBar[3]  = loadTexture("assets/hpBar/hpBar4.png",  ctx->renderer);
    ctx->player1.hpBar[4]  = loadTexture("assets/hpBar/hpBar5.png",  ctx->renderer);
    ctx->player1.hpBar[5]  = loadTexture("assets/hpBar/hpBar6.png",  ctx->renderer);
    ctx->player1.hpBar[6]  = loadTexture("assets/hpBar/hpBar7.png",  ctx->renderer);
    ctx->player1.hpBar[7]  = loadTexture("assets/hpBar/hpBar8.png",  ctx->renderer);

    ctx->player1.attacking   = 0;
    ctx->player1.attackTimer = 0;
    ctx->player1.attackFrame = 0;
    ctx->player1.frame       = 0;
    ctx->player1.frameTimer  = 0;
    ctx->player1.frameDelay  = WALKING_FRAME_DELAY;
    ctx->player1.lastDir     = -1;
    ctx->player1.lastHDir    = SDL_SCANCODE_D;
    ctx->player1.jumping     = 0;
    ctx->player1.jumpTimer   = 0;
    ctx->player1.jumpOffset  = 0;

    ctx->player1.rect.x = PLAYER1_X;
    ctx->player1.rect.y = PLAYER1_Y;
    ctx->player1.rect.w = PLAYER1_W;
    ctx->player1.rect.h = PLAYER1_H;

    ctx->player1.healthRect.x = PLAYER1HP_X;
    ctx->player1.healthRect.y = PLAYER1HP_Y;
    ctx->player1.healthRect.w = PLAYER1HP_W;
    ctx->player1.healthRect.h = PLAYER1HP_H;

    ctx->player1.walkingSound   = Mix_LoadWAV("assets/sounds/walking.mp3");
    ctx->player1.runningSound   = Mix_LoadWAV("assets/sounds/running.mp3");
    ctx->player1.jumpingSound   = Mix_LoadWAV("assets/sounds/jump.mp3");
    ctx->player1.attackingSound = Mix_LoadWAV("assets/sounds/attack.mp3");
    ctx->player1.gettingHitSound = Mix_LoadWAV("assets/sounds/hitCharacter.mp3");
    ctx->player1.deathSound = Mix_LoadWAV("assets/sounds/death.mp3");

    ctx->player1.healthStatus = 0;
    ctx->player1.alive = 1;

    ctx->player1.speed = WALKING_SPEED;
    ctx->player1.walkToRun = 0;
    ctx->player1.knockbackX = 0;
    ctx->player1.knockbackXTimer = 0;

    ctx->player1.score=0;
    ctx->player1.moving=0;
    ctx->player1.selectedOutfit = 1;
    ctx->player1.selectedChar = 1;
}

void initPlayer2(GameContext *ctx)
{
    ctx->player2.outfitNum = 1;
    ctx->player2.outfitSwitched = 0;
    initOutfit(ctx, 2, 1, 2);
    
    ctx->player2.hpBar[0]  = loadTexture("assets/hpBar/hpBar1.png",  ctx->renderer);
    ctx->player2.hpBar[1]  = loadTexture("assets/hpBar/hpBar2.png",  ctx->renderer);
    ctx->player2.hpBar[2]  = loadTexture("assets/hpBar/hpBar3.png",  ctx->renderer);
    ctx->player2.hpBar[3]  = loadTexture("assets/hpBar/hpBar4.png",  ctx->renderer);
    ctx->player2.hpBar[4]  = loadTexture("assets/hpBar/hpBar5.png",  ctx->renderer);
    ctx->player2.hpBar[5]  = loadTexture("assets/hpBar/hpBar6.png",  ctx->renderer);
    ctx->player2.hpBar[6]  = loadTexture("assets/hpBar/hpBar7.png",  ctx->renderer);
    ctx->player2.hpBar[7]  = loadTexture("assets/hpBar/hpBar8.png",  ctx->renderer);

    ctx->player2.attacking   = 0;
    ctx->player2.attackTimer = 0;
    ctx->player2.attackFrame = 0;
    ctx->player2.frame       = 0;
    ctx->player2.frameTimer  = 0;
    ctx->player2.frameDelay  = 10;
    ctx->player2.lastDir     = -1;
    ctx->player2.lastHDir    = SDL_SCANCODE_RIGHT;
    ctx->player2.jumping     = 0;
    ctx->player2.jumpTimer   = 0;
    ctx->player2.jumpOffset  = 0;

    ctx->player2.rect.x = PLAYER2_X;
    ctx->player2.rect.y = PLAYER2_Y;
    ctx->player2.rect.w = PLAYER2_W;
    ctx->player2.rect.h = PLAYER2_H;

    ctx->player2.healthRect.x = PLAYER2HP_X;
    ctx->player2.healthRect.y = PLAYER2HP_Y;
    ctx->player2.healthRect.w = PLAYER2HP_W;
    ctx->player2.healthRect.h = PLAYER2HP_H;

    ctx->player2.walkingSound   = Mix_LoadWAV("assets/sounds/walking.mp3");
    ctx->player2.runningSound   = Mix_LoadWAV("assets/sounds/running.mp3");
    ctx->player2.jumpingSound   = Mix_LoadWAV("assets/sounds/jump.mp3");
    ctx->player2.attackingSound = Mix_LoadWAV("assets/sounds/attack.mp3");
    ctx->player2.gettingHitSound = Mix_LoadWAV("assets/sounds/hitCharacter.mp3");
    ctx->player2.deathSound = Mix_LoadWAV("assets/sounds/death.mp3");

    ctx->player2.healthStatus = 0;
    ctx->player2.alive = 1;

    ctx->player2.walkToRun = 0;
    ctx->player2.knockbackX = 0;
    ctx->player2.knockbackXTimer = 0;
    ctx->player2.speed = WALKING_SPEED;
    
    ctx->player2.score=0;
    ctx->player2.moving=0;
    ctx->player2.selectedOutfit = 1;
    ctx->player2.selectedChar = 2;
}

GameContext* game_init(void)
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
    if (!ctx->window) { fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); free(ctx); return NULL; }

    ctx->renderer = SDL_CreateRenderer(ctx->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ctx->renderer) { fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError()); SDL_DestroyWindow(ctx->window); free(ctx); return NULL; }

    memset(ctx->keys, 0, sizeof(ctx->keys));

    initPlayer1(ctx);
    initPlayer2(ctx);

    ctx->font = TTF_OpenFont("assets/fonts/pixelFont.ttf", 24);
    if (!ctx->font) {
        printf("Font failed: %s\n", TTF_GetError());
    }

    ctx->sm.menuBoard = loadTexture("assets/subMenu/menuBoard.png", ctx->renderer);
    ctx->sm.resumeBtn.tex = loadTexture("assets/subMenu/buttons/resume.png", ctx->renderer);
    ctx->sm.saveBtn.tex = loadTexture("assets/subMenu/buttons/save.png", ctx->renderer);
    ctx->sm.loadBtn.tex = loadTexture("assets/subMenu/buttons/load.png", ctx->renderer);
    ctx->sm.playerBtn.tex = loadTexture("assets/subMenu/buttons/players.png", ctx->renderer);
    ctx->sm.scoreBtn.tex = loadTexture("assets/subMenu/buttons/scores.png", ctx->renderer);
    ctx->sm.quitBtn.tex = loadTexture("assets/subMenu/buttons/quit.png", ctx->renderer);
    ctx->sm.charSelectBtn.tex = loadTexture("assets/subMenu/buttons/charSelect.png", ctx->renderer);
    ctx->sm.outfitsBtn.tex = loadTexture("assets/subMenu/buttons/outfits.png", ctx->renderer);
    ctx->sm.buttonsBtn.tex = loadTexture("assets/subMenu/buttons/buttons.png", ctx->renderer);

    SDL_QueryTexture(ctx->sm.menuBoard, NULL, NULL, &ctx->sm.menuBoardRect.w, &ctx->sm.menuBoardRect.h);
    ctx->sm.menuBoardRect=(SDL_Rect){20,250,ctx->sm.menuBoardRect.w + 100,ctx->sm.menuBoardRect.h + 140};
    ctx->paused = 0;
    ctx->pauseSwitched = 0;
    ctx->running = 1;

    ctx->sm.resumeBtn.rect = (SDL_Rect) {60,320, BUTTON_W, BUTTON_H};
    ctx->sm.saveBtn.rect = (SDL_Rect) {60,365, BUTTON_W, BUTTON_H};
    ctx->sm.loadBtn.rect = (SDL_Rect) {60,410, BUTTON_W, BUTTON_H};
    ctx->sm.playerBtn.rect = (SDL_Rect) {60,455, BUTTON_W, BUTTON_H};
    ctx->sm.scoreBtn.rect = (SDL_Rect) {60,500, BUTTON_W, BUTTON_H};
    ctx->sm.quitBtn.rect = (SDL_Rect) {60,545, BUTTON_W, BUTTON_H};

    ctx->sm.resumeBtn.hovered = 0;
    ctx->sm.saveBtn.hovered   = 0;
    ctx->sm.loadBtn.hovered   = 0;
    ctx->sm.playerBtn.hovered = 0;
    ctx->sm.scoreBtn.hovered  = 0;
    ctx->sm.quitBtn.hovered   = 0;

    ctx->sm.hoverSound = Mix_LoadWAV("assets/sounds/buttonHover.wav");;

    ctx->currentState = STATE_PLAYING;

    ctx->sm.charSelectBtn.rect = (SDL_Rect){60, 320, BUTTON_W, BUTTON_H};
    ctx->sm.outfitsBtn.rect    = (SDL_Rect){60, 365, BUTTON_W, BUTTON_H};
    ctx->sm.buttonsBtn.rect    = (SDL_Rect){60, 410, BUTTON_W, BUTTON_H};
    ctx->sm.playerBtnSwitched = 0;

    ctx->sm.playersBg = loadTexture("assets/subMenu/backgrounds/playerSubMenuBg.jpg", ctx->renderer);
    ctx->sm.p1o1Btn.tex    = loadTexture("assets/subMenu/outfitFrames/p1o1.png", ctx->renderer);
    ctx->sm.p1o1Btn.rect   = (SDL_Rect){40, 220, 150, 200};
    ctx->sm.p1o1Btn.hovered = 0;

    ctx->sm.p1o2Btn.tex    = loadTexture("assets/subMenu/outfitFrames/p1o2.png", ctx->renderer);
    ctx->sm.p1o2Btn.rect   = (SDL_Rect){210, 225, 150, 195};
    ctx->sm.p1o2Btn.hovered = 0;

    ctx->sm.okBtn.tex = loadTexture("assets/subMenu/buttons/ok.png", ctx->renderer);
    ctx->sm.okBtn.rect = (SDL_Rect){(WINDOW_WIDTH - BUTTON_W) / 2, 540, BUTTON_W, BUTTON_H};
    ctx->sm.okBtn.hovered = 0;

    ctx->sm.backBtn.tex  = loadTexture("assets/subMenu/buttons/back.png", ctx->renderer);
    ctx->sm.backBtn.rect = (SDL_Rect){60, 455, BUTTON_W, BUTTON_H};
    ctx->sm.backBtn.hovered = 0;

    ctx->sm.p2o1Btn.tex    = loadTexture("assets/subMenu/outfitFrames/p2o1.png", ctx->renderer);
    ctx->sm.p2o1Btn.rect   = (SDL_Rect){WINDOW_WIDTH-380, 220, 150, 200};
    ctx->sm.p2o1Btn.hovered = 0;

    ctx->sm.p2o2Btn.tex    = loadTexture("assets/subMenu/outfitFrames/p2o2.png", ctx->renderer);
    ctx->sm.p2o2Btn.rect   = (SDL_Rect){WINDOW_WIDTH-210, 225, 150, 195};
    ctx->sm.p2o2Btn.hovered = 0;

    ctx->sm.charSelectBg = loadTexture("assets/subMenu/backgrounds/playerSubMenuBg.jpg", ctx->renderer);
    ctx->sm.p1preview = loadTexture("assets/subMenu/charSelect/p1preview.png", ctx->renderer);
    ctx->sm.p2preview = loadTexture("assets/subMenu/charSelect/p2preview.png", ctx->renderer);

    ctx->sm.p1SwapBtn.tex     = loadTexture("assets/subMenu/buttons/swap.png", ctx->renderer);
    ctx->sm.p1SwapBtn.rect = (SDL_Rect){150 + (180 - BUTTON_W) / 2, 490, BUTTON_W, BUTTON_H};

    ctx->sm.p1SwapBtn.hovered = 0;

    ctx->sm.p2SwapBtn.tex     = loadTexture("assets/subMenu/buttons/swap.png", ctx->renderer);
    ctx->sm.p2SwapBtn.rect = (SDL_Rect){650 + (180 - BUTTON_W) / 2, 490, BUTTON_W, BUTTON_H};
    ctx->sm.p2SwapBtn.hovered = 0;

    return ctx;
}

int hasIntersection(SDL_Rect r1, SDL_Rect r2)
{
    return (r1.x + r1.w >= r2.x &&
            r1.x <= r2.x + r2.w &&
            r1.y + r1.h >= r2.y &&
            r1.y <= r2.y + r2.h);
}

void playerMechanics(GameContext *ctx)
{
  int speed = WALKING_SPEED;
  if(!ctx->paused)
  {
    ctx->player1.moving = 0;
    ctx->player2.moving = 0;

    // player 1 movement
    if (ctx->player1.alive)
    {
        //runnning
        if (ctx->keys[SDL_SCANCODE_LSHIFT]) {
          ctx->player1.speed = 5;
          ctx->player1.frameDelay = 7;
          if(!ctx->player1.walkToRun) {Mix_HaltChannel(CH_P1_WALK); ctx->player1.walkToRun=1;}
          if (!Mix_Playing(CH_P1_WALK))Mix_PlayChannel(CH_P1_WALK, ctx->player1.runningSound, -1);
        }
        else {
          ctx->player1.speed = WALKING_SPEED;
          ctx->player1.frameDelay=WALKING_FRAME_DELAY;
          ctx->player1.walkToRun=0;
        }

        //walk right 
        if (ctx->keys[SDL_SCANCODE_D]) {
            ctx->player1.rect.x += ctx->player1.speed;
            if (!ctx->player1.attacking) ctx->player1.currentState = ctx->player1.walkRight[ctx->player1.frame % 5];
            ctx->player1.lastDir  = SDL_SCANCODE_D;
            ctx->player1.lastHDir = SDL_SCANCODE_D;
            ctx->player1.moving = 1;
            if (!Mix_Playing(CH_P1_WALK)) Mix_PlayChannel(CH_P1_WALK, ctx->player1.walkingSound, -1);
        }
        //walk left
        if (ctx->keys[SDL_SCANCODE_A]) {
            ctx->player1.rect.x -= ctx->player1.speed;
            if (!ctx->player1.attacking) ctx->player1.currentState = ctx->player1.walkLeft[ctx->player1.frame % 5];
            ctx->player1.lastDir  = SDL_SCANCODE_A;
            ctx->player1.lastHDir = SDL_SCANCODE_A;
            ctx->player1.moving = 1;
            if (!Mix_Playing(CH_P1_WALK)) Mix_PlayChannel(CH_P1_WALK, ctx->player1.walkingSound, -1);
        }
        //walk up
        if (ctx->keys[SDL_SCANCODE_W]) {
            ctx->player1.rect.y -= ctx->player1.speed;
            if (!ctx->player1.attacking) ctx->player1.currentState = ctx->player1.walkUp[ctx->player1.frame % 2];
            ctx->player1.lastDir = SDL_SCANCODE_W;
            ctx->player1.moving = 1;
            if (!Mix_Playing(CH_P1_WALK)) Mix_PlayChannel(CH_P1_WALK, ctx->player1.walkingSound, -1);
        }
        //walk down
        if (ctx->keys[SDL_SCANCODE_S]) {
            ctx->player1.rect.y += ctx->player1.speed;
            if (!ctx->player1.attacking) ctx->player1.currentState = ctx->player1.walkDown[ctx->player1.frame % 2];
            ctx->player1.lastDir = SDL_SCANCODE_S;
            ctx->player1.moving = 1;
            if (!Mix_Playing(CH_P1_WALK)) Mix_PlayChannel(CH_P1_WALK, ctx->player1.walkingSound, -1);
        }
        //jump
        if (ctx->keys[SDL_SCANCODE_SPACE] && !ctx->player1.jumping) {
            ctx->player1.jumping   = 1;
            ctx->player1.jumpTimer = 0;
            Mix_HaltChannel(CH_P1_WALK);
            if (!Mix_Playing(CH_P1_JUMP)) Mix_PlayChannel(CH_P1_JUMP, ctx->player1.jumpingSound, 0);
        }
        //attack
        if (ctx->keys[SDL_SCANCODE_B] && !ctx->player1.attacking) {
            ctx->player1.attacking   = 1;
            ctx->player1.attackTimer = 0;
            ctx->player1.attackFrame = 0;
            ctx->player1.baseX = ctx->player1.rect.x;
            if (ctx->player1.lastHDir == SDL_SCANCODE_A)
                ctx->player1.currentState = ctx->player1.attackLeft[0];
            else
                ctx->player1.currentState = ctx->player1.attackRight[0];
            if (!Mix_Playing(CH_P1_ATTACK)) Mix_PlayChannel(CH_P1_ATTACK, ctx->player1.attackingSound, 0);
        }
        
        // player 1 attack animation
        if (ctx->player1.attacking) {
            ctx->player1.attackTimer++;
            if (ctx->player1.attackTimer >= 4) {
                ctx->player1.attackTimer = 0;
                ctx->player1.attackFrame++;
                if (ctx->player1.attackFrame == 3) {
                    ctx->player1.rect.w += 20;
                    if (hasIntersection(ctx->player1.rect, ctx->player2.rect) && ctx->player2.alive) 
                    {
                        //knockback 
                        if(ctx->player1.lastHDir == SDL_SCANCODE_D)
                        {
                          ctx->player2.knockbackX = 8;
                        }
                        else 
                        {
                          ctx->player2.knockbackX = -8;
                        }
                        ctx->player2.knockbackXTimer = 15;



                        //health management
                        if (ctx->player2.healthStatus < 7) {
                            ctx->player2.healthStatus++;
                            if (ctx->player2.healthStatus == 6) {
                                ctx->player2.alive = 0;
                                ctx->player2.healthStatus = 7;
                                if (!Mix_Playing(CH_P2_DEATH)) Mix_PlayChannel(CH_P2_DEATH, ctx->player2.deathSound, 0);
                                ctx->player1.score -= 99;
                            }
                        }
                        //sound
                        if (!Mix_Playing(CH_P2_GETHIT)) Mix_PlayChannel(CH_P2_GETHIT, ctx->player2.gettingHitSound, 0);

                    }
                } else {
                    ctx->player1.rect.w = PLAYER1_W;
                }
                if (ctx->player1.attackFrame >= 6) {
                    ctx->player1.attacking   = 0;
                    ctx->player1.attackFrame = 0;
                } else {
                    if (ctx->player1.lastHDir == SDL_SCANCODE_A)
                        ctx->player1.currentState = ctx->player1.attackLeft[ctx->player1.attackFrame];
                    else
                        ctx->player1.currentState = ctx->player1.attackRight[ctx->player1.attackFrame];
                }
            }
        }

        // player 1 animation
        if (ctx->player1.knockbackXTimer > 0) {
            ctx->player1.rect.x += ctx->player1.knockbackX;
            ctx->player1.knockbackX = (int)(ctx->player1.knockbackX * 0.85f);
            ctx->player1.knockbackXTimer--;
        }

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

        // player 1 jump
        if (ctx->player1.jumping) {
            ctx->player1.jumpTimer++;
            float t = (float)ctx->player1.jumpTimer / 30.0f;
            ctx->player1.jumpOffset = (int)(sin(t * 3.14f) * 40);
            if (ctx->player1.jumpTimer >= 30) {
                ctx->player1.jumping    = 0;
                ctx->player1.jumpTimer  = 0;
                ctx->player1.jumpOffset = 0;
            }
        }
    }

    // player 2 movement

    if (ctx->player2.alive)
    {
      //running
      if (ctx->keys[SDL_SCANCODE_M]) {
            ctx->player2.speed = 5;
            ctx->player2.frameDelay = 7;
            if(!ctx->player2.walkToRun) {Mix_HaltChannel(CH_P2_WALK); ctx->player2.walkToRun=1;}
            if (!Mix_Playing(CH_P2_WALK))Mix_PlayChannel(CH_P2_WALK, ctx->player2.runningSound, -1);
      }
      else {
            ctx->player2.speed = WALKING_SPEED;
            ctx->player2.frameDelay=WALKING_FRAME_DELAY;
            ctx->player2.walkToRun=0;
          }


      if (ctx->player2.alive)
      {
          //walk right
          if (ctx->keys[SDL_SCANCODE_RIGHT]) {
              ctx->player2.rect.x += ctx->player2.speed;
              if (!ctx->player2.attacking) ctx->player2.currentState = ctx->player2.walkRight[ctx->player2.frame % 5];
              ctx->player2.lastDir  = SDL_SCANCODE_RIGHT;
              ctx->player2.lastHDir = SDL_SCANCODE_RIGHT;
              ctx->player2.moving = 1;
              if (!Mix_Playing(CH_P2_WALK)) Mix_PlayChannel(CH_P2_WALK, ctx->player2.walkingSound, -1);
          }
          //walk left
          if (ctx->keys[SDL_SCANCODE_LEFT]) {
              ctx->player2.rect.x -= ctx->player2.speed;
              if (!ctx->player2.attacking) ctx->player2.currentState = ctx->player2.walkLeft[ctx->player2.frame % 5];
              ctx->player2.lastDir  = SDL_SCANCODE_LEFT;
              ctx->player2.lastHDir = SDL_SCANCODE_LEFT;
              ctx->player2.moving = 1;
              if (!Mix_Playing(CH_P2_WALK)) Mix_PlayChannel(CH_P2_WALK, ctx->player2.walkingSound, -1);
          }
          //walk up
          if (ctx->keys[SDL_SCANCODE_UP]) {
              ctx->player2.rect.y -= ctx->player2.speed;
              if (!ctx->player2.attacking) ctx->player2.currentState = ctx->player2.walkUp[ctx->player2.frame % 2];
              ctx->player2.lastDir = SDL_SCANCODE_UP;
              ctx->player2.moving = 1;
              if (!Mix_Playing(CH_P2_WALK)) Mix_PlayChannel(CH_P2_WALK, ctx->player2.walkingSound, -1);
          }
          //walk down
          if (ctx->keys[SDL_SCANCODE_DOWN]) {
              ctx->player2.rect.y += ctx->player2.speed;
              if (!ctx->player2.attacking) ctx->player2.currentState = ctx->player2.walkDown[ctx->player2.frame % 2];
              ctx->player2.lastDir = SDL_SCANCODE_DOWN;
              ctx->player2.moving = 1;
              if (!Mix_Playing(CH_P2_WALK)) Mix_PlayChannel(CH_P2_WALK, ctx->player2.walkingSound, -1);
          }
          //run
          if (ctx->keys[SDL_SCANCODE_RSHIFT] && !ctx->player2.jumping) {
              ctx->player2.jumping   = 1;
              ctx->player2.jumpTimer = 0;
              if (!Mix_Playing(CH_P2_JUMP)) Mix_PlayChannel(CH_P2_JUMP, ctx->player2.jumpingSound, 0);
          }
          //attack
          if (ctx->keys[SDL_SCANCODE_RETURN] && !ctx->player2.attacking) {
              ctx->player2.attacking   = 1;
              ctx->player2.attackTimer = 0;
              ctx->player2.attackFrame = 0;
              ctx->player2.baseX = ctx->player2.rect.x;
              if (!Mix_Playing(CH_P2_ATTACK)) Mix_PlayChannel(CH_P2_ATTACK, ctx->player2.attackingSound, 0);
              if (ctx->player2.lastHDir == SDL_SCANCODE_LEFT)
                  ctx->player2.currentState = ctx->player2.attackLeft[0];
              else
                  ctx->player2.currentState = ctx->player2.attackRight[0];
          }

          // player 2 animation
          if (ctx->player2.knockbackXTimer > 0) {
              ctx->player2.rect.x += ctx->player2.knockbackX;
              ctx->player2.knockbackX = (int)(ctx->player2.knockbackX * 0.85f);
              ctx->player2.knockbackXTimer--;
          }
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

          // player 2 jump
          if (ctx->player2.jumping) {
              ctx->player2.jumpTimer++;
              float t = (float)ctx->player2.jumpTimer / 30.0f;
              ctx->player2.jumpOffset = (int)(sin(t * 3.14f) * 40);
              if (ctx->player2.jumpTimer >= 30) {
                  ctx->player2.jumping    = 0;
                  ctx->player2.jumpTimer  = 0;
                  ctx->player2.jumpOffset = 0;
              }
          }

          // player 2 attack animation
          if (ctx->player2.attacking) {
              ctx->player2.attackTimer++;
              if (ctx->player2.attackTimer >= 4) {
                  ctx->player2.attackTimer = 0;
                  ctx->player2.attackFrame++;
                  if (ctx->player2.attackFrame == 3 || ctx->player2.attackFrame == 2) {
                      ctx->player2.rect.w += 10;
                      if (ctx->player2.attackFrame == 3) {
                          if (hasIntersection(ctx->player1.rect, ctx->player2.rect) && ctx->player1.alive) 
                          {
                              //knockback 
                              if(ctx->player2.lastHDir == SDL_SCANCODE_RIGHT)
                              {
                                ctx->player1.knockbackX = 8;
                              }
                              else 
                              {
                                ctx->player1.knockbackX = -8;
                              }
                              ctx->player1.knockbackXTimer = 15;

                              //health management
                              if (ctx->player1.healthStatus < 7) {
                                  ctx->player1.healthStatus++;
                                  if (ctx->player1.healthStatus == 6) {
                                      ctx->player1.alive = 0;
                                      ctx->player1.healthStatus = 7;
                                      if (!Mix_Playing(CH_P1_DEATH)) Mix_PlayChannel(CH_P1_DEATH, ctx->player1.deathSound, 0);
                                      ctx->player2.score -= 99;
                                  }
                              }
                              //sound
                              if (!Mix_Playing(CH_P1_GETHIT)) Mix_PlayChannel(CH_P1_GETHIT, ctx->player1.gettingHitSound, 0);
                          }

                      }
                  } else {
                      ctx->player2.rect.w = PLAYER2_W;
                  }
                  if (ctx->player2.attackFrame >= 6) {
                      ctx->player2.attacking   = 0;
                      ctx->player2.attackFrame = 0;
                  } else {
                      if (ctx->player2.lastHDir == SDL_SCANCODE_LEFT)
                          ctx->player2.currentState = ctx->player2.attackLeft[ctx->player2.attackFrame];
                      else
                          ctx->player2.currentState = ctx->player2.attackRight[ctx->player2.attackFrame];
                  }
              }
          }
      }
    }
  }
}

SDL_Rect scale_rect(SDL_Rect rect, float scale)
{
    SDL_Rect scaled;
    float w = rect.w, h = rect.h;
    float sw = w * scale, sh = h * scale;
    scaled.w = (int)sw;
    scaled.h = (int)sh;
    scaled.x = rect.x + (int)((w - sw) / 2);
    scaled.y = rect.y + (int)((h - sh) / 2);
    return scaled;
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
            ? scale_rect(btns[i]->rect, 1.1f)
            : btns[i]->rect;
        SDL_RenderCopy(ctx->renderer, btns[i]->tex, NULL, &draw);
    }

    for (int i = 0; i < 6; i++) {
        int over = point_in_rect(mx, my, &btns[i]->rect);
        if (over && !btns[i]->hovered) {
            Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
            btns[i]->hovered = 1;
        } else if (!over) {
            btns[i]->hovered = 0;
        }
    }
}

void playersMenuFn(GameContext *ctx)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    Button *btns[] = {
        &ctx->sm.charSelectBtn,
        &ctx->sm.outfitsBtn,
        &ctx->sm.buttonsBtn
    };

    for (int i = 0; i < 3; i++) {
        SDL_Rect draw = point_in_rect(mx, my, &btns[i]->rect)
            ? scale_rect(btns[i]->rect, 1.1f)
            : btns[i]->rect;
        SDL_RenderCopy(ctx->renderer, btns[i]->tex, NULL, &draw);
    }

    for (int i = 0; i < 3; i++) {
        int over = point_in_rect(mx, my, &btns[i]->rect);
        if (over && !btns[i]->hovered) {
            Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
            btns[i]->hovered = 1;
        } else if (!over) {
            btns[i]->hovered = 0;
        }
    }
  SDL_Rect drawBack = point_in_rect(mx, my, &ctx->sm.backBtn.rect)
          ? scale_rect(ctx->sm.backBtn.rect, 1.1f)
          : ctx->sm.backBtn.rect;
      SDL_RenderCopy(ctx->renderer, ctx->sm.backBtn.tex, NULL, &drawBack);

      int overBack = point_in_rect(mx, my, &ctx->sm.backBtn.rect);
      if (overBack && !ctx->sm.backBtn.hovered) {
          Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
          ctx->sm.backBtn.hovered = 1;
      } else if (!overBack) {
          ctx->sm.backBtn.hovered = 0;
      }

}

void changeOutfitsFn(GameContext *ctx)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    SDL_RenderCopy(ctx->renderer, ctx->sm.playersBg, NULL, NULL);

    Button *btns[] = { &ctx->sm.p1o1Btn, &ctx->sm.p1o2Btn };

    for (int i = 0; i < 2; i++) {
        SDL_Rect draw = point_in_rect(mx, my, &btns[i]->rect)
            ? scale_rect(btns[i]->rect, 1.1f)
            : btns[i]->rect;
        SDL_RenderCopy(ctx->renderer, btns[i]->tex, NULL, &draw);
    }

    for (int i = 0; i < 2; i++) {
        int over = point_in_rect(mx, my, &btns[i]->rect);
        if (over && !btns[i]->hovered) {
            Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
            btns[i]->hovered = 1;
        } else if (!over) {
            btns[i]->hovered = 0;
        }
    }

    // ok button
    SDL_Rect draw = point_in_rect(mx, my, &ctx->sm.okBtn.rect)
        ? scale_rect(ctx->sm.okBtn.rect, 1.1f)
        : ctx->sm.okBtn.rect;
    SDL_RenderCopy(ctx->renderer, ctx->sm.okBtn.tex, NULL, &draw);

    int over = point_in_rect(mx, my, &ctx->sm.okBtn.rect);
    if (over && !ctx->sm.okBtn.hovered) {
        Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
        ctx->sm.okBtn.hovered = 1;
    } else if (!over) {
        ctx->sm.okBtn.hovered = 0;
    }
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gold  = {180, 150,  80, 255};

    const char *label1 = ctx->player1.selectedOutfit == 1 ? "Outfit 1 Selected" : "Outfit 1";
    const char *label2 = ctx->player1.selectedOutfit == 2 ? "Outfit 2 Selected" : "Outfit 2";

    SDL_Surface *s;
    SDL_Texture *t;
    SDL_Rect d;

    s = TTF_RenderText_Blended(ctx->font, label1, ctx->player1.selectedOutfit == 1 ? gold : white);
    t = SDL_CreateTextureFromSurface(ctx->renderer, s);
    SDL_FreeSurface(s);
    SDL_QueryTexture(t, NULL, NULL, &d.w, &d.h);
    d.x = ctx->sm.p1o1Btn.rect.x + (ctx->sm.p1o1Btn.rect.w - d.w) / 2;
    d.y = ctx->sm.p1o1Btn.rect.y + ctx->sm.p1o1Btn.rect.h + 5;
    SDL_RenderCopy(ctx->renderer, t, NULL, &d);
    SDL_DestroyTexture(t);

    s = TTF_RenderText_Blended(ctx->font, label2, ctx->player1.selectedOutfit == 2 ? gold : white);
    t = SDL_CreateTextureFromSurface(ctx->renderer, s);
    SDL_FreeSurface(s);
    SDL_QueryTexture(t, NULL, NULL, &d.w, &d.h);
    d.x = ctx->sm.p1o2Btn.rect.x + (ctx->sm.p1o2Btn.rect.w - d.w) / 2;
    d.y = ctx->sm.p1o2Btn.rect.y + ctx->sm.p1o2Btn.rect.h + 5;
    SDL_RenderCopy(ctx->renderer, t, NULL, &d);
    SDL_DestroyTexture(t);
    // p2 outfit buttons
    Button *btns2[] = { &ctx->sm.p2o1Btn, &ctx->sm.p2o2Btn };

    for (int i = 0; i < 2; i++) {
        SDL_Rect draw2 = point_in_rect(mx, my, &btns2[i]->rect)
            ? scale_rect(btns2[i]->rect, 1.1f)
            : btns2[i]->rect;
        SDL_RenderCopy(ctx->renderer, btns2[i]->tex, NULL, &draw2);
    }

    for (int i = 0; i < 2; i++) {
        int over2 = point_in_rect(mx, my, &btns2[i]->rect);
        if (over2 && !btns2[i]->hovered) {
            Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
            btns2[i]->hovered = 1;
        } else if (!over2) {
            btns2[i]->hovered = 0;
        }
    }

    // p2 labels
    const char *label3 = ctx->player2.selectedOutfit == 1 ? "Outfit 1 Selected" : "Outfit 1";
    const char *label4 = ctx->player2.selectedOutfit == 2 ? "Outfit 2 Selected" : "Outfit 2";

    s = TTF_RenderText_Blended(ctx->font, label3, ctx->player2.selectedOutfit == 1 ? gold : white);
    t = SDL_CreateTextureFromSurface(ctx->renderer, s);
    SDL_FreeSurface(s);
    SDL_QueryTexture(t, NULL, NULL, &d.w, &d.h);
    d.x = ctx->sm.p2o1Btn.rect.x + (ctx->sm.p2o1Btn.rect.w - d.w) / 2;
    d.y = ctx->sm.p2o1Btn.rect.y + ctx->sm.p2o1Btn.rect.h + 5;
    SDL_RenderCopy(ctx->renderer, t, NULL, &d);
    SDL_DestroyTexture(t);

    s = TTF_RenderText_Blended(ctx->font, label4, ctx->player2.selectedOutfit == 2 ? gold : white);
    t = SDL_CreateTextureFromSurface(ctx->renderer, s);
    SDL_FreeSurface(s);
    SDL_QueryTexture(t, NULL, NULL, &d.w, &d.h);
    d.x = ctx->sm.p2o2Btn.rect.x + (ctx->sm.p2o2Btn.rect.w - d.w) / 2;
    d.y = ctx->sm.p2o2Btn.rect.y + ctx->sm.p2o2Btn.rect.h + 5;
    SDL_RenderCopy(ctx->renderer, t, NULL, &d);
    SDL_DestroyTexture(t);

}

void charSelectFn(GameContext *ctx)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    SDL_RenderCopy(ctx->renderer, ctx->sm.charSelectBg, NULL, NULL);

    SDL_Rect p1card = {130, 180, 220, 270};
    SDL_Rect p2card = {630, 180, 220, 270};

    SDL_Texture *p1tex = ctx->player1.selectedChar == 1 ? ctx->sm.p1preview : ctx->sm.p2preview;
    SDL_Texture *p2tex = ctx->player2.selectedChar == 2 ? ctx->sm.p2preview : ctx->sm.p1preview;

    SDL_RenderCopy(ctx->renderer, p1tex, NULL, &p1card);
    SDL_RenderCopy(ctx->renderer, p2tex, NULL, &p2card);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gold  = {180, 150,  80, 255};

    SDL_Surface *s; SDL_Texture *t; SDL_Rect d;

    const char *p1label = ctx->player1.selectedChar == 1 ? "P1: Character 1" : "P1: Character 2";
    s = TTF_RenderText_Blended(ctx->font, p1label, gold);
    t = SDL_CreateTextureFromSurface(ctx->renderer, s);
    SDL_FreeSurface(s);
    SDL_QueryTexture(t, NULL, NULL, &d.w, &d.h);
    d.x = 150 + (180 - d.w) / 2;
    d.y = 430;
    SDL_RenderCopy(ctx->renderer, t, NULL, &d);
    SDL_DestroyTexture(t);

    const char *p2label = ctx->player2.selectedChar == 2 ? "P2: Character 2" : "P2: Character 1";
    s = TTF_RenderText_Blended(ctx->font, p2label, gold);
    t = SDL_CreateTextureFromSurface(ctx->renderer, s);
    SDL_FreeSurface(s);
    SDL_QueryTexture(t, NULL, NULL, &d.w, &d.h);
    d.x = 650 + (180 - d.w) / 2;
    d.y = 430;
    SDL_RenderCopy(ctx->renderer, t, NULL, &d);
    SDL_DestroyTexture(t);

    // swap buttons
    Button *swapBtns[] = { &ctx->sm.p1SwapBtn, &ctx->sm.p2SwapBtn };
    for (int i = 0; i < 2; i++) {
        SDL_Rect draw2 = point_in_rect(mx, my, &swapBtns[i]->rect)
            ? scale_rect(swapBtns[i]->rect, 1.1f)
            : swapBtns[i]->rect;
        SDL_RenderCopy(ctx->renderer, swapBtns[i]->tex, NULL, &draw2);

        int over2 = point_in_rect(mx, my, &swapBtns[i]->rect);
        if (over2 && !swapBtns[i]->hovered) {
            Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
            swapBtns[i]->hovered = 1;
        } else if (!over2) {
            swapBtns[i]->hovered = 0;
        }
    }

    // ok button
    SDL_Rect draw = point_in_rect(mx, my, &ctx->sm.okBtn.rect)
        ? scale_rect(ctx->sm.okBtn.rect, 1.1f)
        : ctx->sm.okBtn.rect;
    SDL_RenderCopy(ctx->renderer, ctx->sm.okBtn.tex, NULL, &draw);
}

void game_update(GameContext *ctx)
{
    int windowWidth, windowHeight;
    SDL_GetWindowSize(ctx->window, &windowWidth, &windowHeight);
    int mx, my;
    SDL_GetMouseState(&mx, &my);


    while (SDL_PollEvent(&ctx->event))
    {
        if (ctx->event.type == SDL_QUIT) { ctx->running = 0; }
        else if (ctx->event.type == SDL_KEYDOWN)
            ctx->keys[ctx->event.key.keysym.scancode] = 1;
        else if (ctx->event.type == SDL_KEYUP)
            ctx->keys[ctx->event.key.keysym.scancode] = 0;
        else if (ctx->event.type == SDL_MOUSEBUTTONUP)
            ctx->sm.playerBtnSwitched = 0;
        else if (ctx->event.type == SDL_MOUSEBUTTONDOWN && ctx->paused) {
            if (ctx->currentState == STATE_PAUSED_MAIN) {
                if (point_in_rect(mx, my, &ctx->sm.resumeBtn.rect)) {
                    ctx->paused = 0;
                    ctx->currentState = STATE_PLAYING;
                    Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
                }
                else if (point_in_rect(mx, my, &ctx->sm.playerBtn.rect) && !ctx->sm.playerBtnSwitched) {
                    ctx->currentState = STATE_PAUSED_PLAYERS;
                    Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
                    ctx->sm.playerBtnSwitched = 1;
                }
            }
            else if (ctx->currentState == STATE_PAUSED_PLAYERS) {
                if (point_in_rect(mx, my, &ctx->sm.outfitsBtn.rect)) {
                    ctx->currentState = STATE_PAUSED_OUTFITS;
                    Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
                }
                SDL_Rect backBtn = {60, 455, BUTTON_W, BUTTON_H};
                if (point_in_rect(mx, my, &backBtn)) {
                    ctx->currentState = STATE_PAUSED_MAIN;
                    Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
                }
                if (point_in_rect(mx, my, &ctx->sm.charSelectBtn.rect)) {
                    ctx->currentState = STATE_PAUSED_CHARSELECT;
                    Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
                }
            }
            else if (ctx->currentState == STATE_PAUSED_OUTFITS) {
                if (point_in_rect(mx, my, &ctx->sm.p1o1Btn.rect)) {
                    ctx->player1.selectedOutfit = 1;
                    Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
                }
                else if (point_in_rect(mx, my, &ctx->sm.p1o2Btn.rect)) {
                    ctx->player1.selectedOutfit = 2;
                    Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
                }
                else if (point_in_rect(mx, my, &ctx->sm.okBtn.rect)) {
                    initOutfit(ctx, 1, ctx->player1.selectedOutfit, ctx->player1.selectedChar);  // reload p1 with their outfit
                    initOutfit(ctx, 2, ctx->player2.selectedOutfit, ctx->player2.selectedChar);  // reload p2 with their outfit
                    ctx->currentState = STATE_PAUSED_PLAYERS;
                    Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
                }
                else if (point_in_rect(mx, my, &ctx->sm.p2o1Btn.rect)) {
                    ctx->player2.selectedOutfit = 1;
                    Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
                }
                else if (point_in_rect(mx, my, &ctx->sm.p2o2Btn.rect)) {
                    ctx->player2.selectedOutfit = 2;
                    Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
                }
                SDL_Rect backBtn = {60, 455, BUTTON_W, BUTTON_H};
                if (point_in_rect(mx, my, &backBtn)) {
                    ctx->currentState = STATE_PAUSED_PLAYERS;
                    Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
                }
            }        
            else if (ctx->currentState == STATE_PAUSED_CHARSELECT) {
                if (point_in_rect(mx, my, &ctx->sm.p1SwapBtn.rect) ||
                    point_in_rect(mx, my, &ctx->sm.p2SwapBtn.rect)) {
                    int tmp = ctx->player1.selectedChar;
                    ctx->player1.selectedChar = ctx->player2.selectedChar;
                    ctx->player2.selectedChar = tmp;
                    Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
                }
                else if (point_in_rect(mx, my, &ctx->sm.okBtn.rect)) {
                    initOutfit(ctx, 1, ctx->player1.selectedOutfit, ctx->player1.selectedChar);
                    initOutfit(ctx, 2, ctx->player2.selectedOutfit, ctx->player2.selectedChar);
                    ctx->currentState = STATE_PAUSED_PLAYERS;
                    Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
                }
                SDL_Rect backBtn = {60, 455, BUTTON_W, BUTTON_H};
                if (point_in_rect(mx, my, &backBtn)) {
                    ctx->currentState = STATE_PAUSED_PLAYERS;
                    Mix_PlayChannel(CH_BUTTONS, ctx->sm.hoverSound, 0);
                }
            }                
        }
    }   
    if (ctx->keys[SDL_SCANCODE_ESCAPE] && !ctx->pauseSwitched)
    {
      ctx->paused = !ctx->paused;
      if (ctx->paused) Mix_HaltChannel(-1);
      ctx->currentState = ctx->paused ? STATE_PAUSED_MAIN : STATE_PLAYING;
      ctx->pauseSwitched = 1;
    }
    if (!ctx->keys[SDL_SCANCODE_ESCAPE]) {
        ctx->pauseSwitched = 0;
    }

    playerMechanics(ctx);

}

void game_cleanup(GameContext *ctx)
{
    if (!ctx) return;

    SDL_DestroyTexture(ctx->player1.idle);
    for (int i = 0; i < 5; i++) SDL_DestroyTexture(ctx->player1.walkRight[i]);
    for (int i = 0; i < 5; i++) SDL_DestroyTexture(ctx->player1.walkLeft[i]);
    for (int i = 0; i < 2; i++) SDL_DestroyTexture(ctx->player1.walkUp[i]);
    for (int i = 0; i < 2; i++) SDL_DestroyTexture(ctx->player1.walkDown[i]);
    for (int i = 0; i < 8; i++) SDL_DestroyTexture(ctx->player1.hpBar[i]);
    for (int i = 0; i < 6; i++) SDL_DestroyTexture(ctx->player1.attackRight[i]);
    for (int i = 0; i < 6; i++) SDL_DestroyTexture(ctx->player1.attackLeft[i]);

    SDL_DestroyTexture(ctx->player2.idle);
    for (int i = 0; i < 5; i++) SDL_DestroyTexture(ctx->player2.walkRight[i]);
    for (int i = 0; i < 5; i++) SDL_DestroyTexture(ctx->player2.walkLeft[i]);
    for (int i = 0; i < 2; i++) SDL_DestroyTexture(ctx->player2.walkUp[i]);
    for (int i = 0; i < 2; i++) SDL_DestroyTexture(ctx->player2.walkDown[i]);
    for (int i = 0; i < 8; i++) SDL_DestroyTexture(ctx->player2.hpBar[i]);
    for (int i = 0; i < 6; i++) SDL_DestroyTexture(ctx->player2.attackRight[i]);
    for (int i = 0; i < 6; i++) SDL_DestroyTexture(ctx->player2.attackLeft[i]);

    Mix_FreeChunk(ctx->player1.walkingSound);
    Mix_FreeChunk(ctx->player1.jumpingSound);
    Mix_FreeChunk(ctx->player1.attackingSound);
    Mix_FreeChunk(ctx->player2.walkingSound);
    Mix_FreeChunk(ctx->player2.jumpingSound);
    Mix_FreeChunk(ctx->player2.attackingSound);
    Mix_FreeChunk(ctx->player1.gettingHitSound);
    Mix_FreeChunk(ctx->player1.deathSound);
    Mix_FreeChunk(ctx->player2.gettingHitSound);
    Mix_FreeChunk(ctx->player2.deathSound);

    if (ctx->font) TTF_CloseFont(ctx->font);
    if (ctx->renderer) SDL_DestroyRenderer(ctx->renderer);
    if (ctx->window)   SDL_DestroyWindow(ctx->window);
    Mix_CloseAudio();
    Mix_Quit();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    free(ctx);
}

void game_render(GameContext *ctx)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    SDL_Rect drawRect1 = ctx->player1.rect;
    drawRect1.y -= ctx->player1.jumpOffset;

    SDL_Rect drawRect2 = ctx->player2.rect;
    drawRect2.y -= ctx->player2.jumpOffset;

    SDL_SetRenderDrawColor(ctx->renderer, 20, 20, 20, 255);
    SDL_RenderClear(ctx->renderer);

    // shadows
    if (ctx->player1.alive) {
        SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 80);
        SDL_Rect shadow1 = { ctx->player1.rect.x + 5, ctx->player1.rect.y + ctx->player1.rect.h - 5, ctx->player1.rect.w - 10, 8 };
        SDL_RenderFillRect(ctx->renderer, &shadow1);
    }
    if (ctx->player2.alive) {
        SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 80);
        SDL_Rect shadow2 = { ctx->player2.rect.x + 5, ctx->player2.rect.y + ctx->player2.rect.h - 5, ctx->player2.rect.w - 10, 8 };
        SDL_RenderFillRect(ctx->renderer, &shadow2);
    }

    SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 255, 255);

    // players
    if (ctx->player1.alive) SDL_RenderCopy(ctx->renderer, ctx->player1.currentState, NULL, &drawRect1);
    if (ctx->player2.alive) SDL_RenderCopy(ctx->renderer, ctx->player2.currentState, NULL, &drawRect2);

    // score text
    char scoreText[32];
    snprintf(scoreText, sizeof(scoreText), "Score: %d", ctx->player1.score);
    SDL_Surface* surf = TTF_RenderText_Blended(ctx->font, scoreText, (SDL_Color){255, 255, 255});
    SDL_Texture* score1Txt = SDL_CreateTextureFromSurface(ctx->renderer, surf);
    SDL_FreeSurface(surf);
    SDL_Rect dst;
    SDL_QueryTexture(score1Txt, NULL, NULL, &dst.w, &dst.h);
    dst.x = 20; dst.y = 70;
    SDL_RenderCopy(ctx->renderer, score1Txt, NULL, &dst);
    SDL_DestroyTexture(score1Txt);

    snprintf(scoreText, sizeof(scoreText), "Score: %d", ctx->player2.score);
    surf = TTF_RenderText_Blended(ctx->font, scoreText, (SDL_Color){255, 255, 255});
    SDL_Texture* score2Txt = SDL_CreateTextureFromSurface(ctx->renderer, surf);
    SDL_FreeSurface(surf);
    SDL_QueryTexture(score2Txt, NULL, NULL, &dst.w, &dst.h);
    dst.x = 790; dst.y = 70;
    SDL_RenderCopy(ctx->renderer, score2Txt, NULL, &dst);
    SDL_DestroyTexture(score2Txt);

    // health bars
    SDL_RenderCopy(ctx->renderer, ctx->player1.hpBar[ctx->player1.healthStatus], NULL, &ctx->player1.healthRect);
    SDL_RenderCopy(ctx->renderer, ctx->player2.hpBar[ctx->player2.healthStatus], NULL, &ctx->player2.healthRect);


    // pause sub-menu
    if (ctx->paused) {
      // dark semi-transparent overlay (the "blur" fake)
      SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);
      SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 160);
      SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
      SDL_RenderFillRect(ctx->renderer, &overlay);

      // menu board
      SDL_RenderCopy(ctx->renderer, ctx->sm.menuBoard, NULL, &ctx->sm.menuBoardRect);

      if (ctx->currentState == STATE_PAUSED_MAIN) {
        subMenuFn(ctx);  
      }
      else if (ctx->currentState == STATE_PAUSED_PLAYERS) {
        playersMenuFn(ctx);
      }
      else if (ctx->currentState == STATE_PAUSED_OUTFITS) {
        changeOutfitsFn(ctx);
      }
      else if (ctx->currentState == STATE_PAUSED_CHARSELECT) {
          charSelectFn(ctx);
      }
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
