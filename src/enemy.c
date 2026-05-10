/*  game.c  –  full implementation
    Compile:
        gcc game.c -o game \
            $(sdl2-config --cflags --libs) \
            -lSDL2_image -lm
*/

#include "enemy.h"

/* ═══════════════════════════════════════════════
   UTILITIES
═══════════════════════════════════════════════ */

SDL_Texture *LoadTexture(const char *path, SDL_Renderer *renderer)
{
    SDL_Surface *s = IMG_Load(path);
    if (!s) {
        printf("[WARN] LoadTexture: %s – %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
    SDL_FreeSurface(s);
    return t;
}

float Vec2Length(float dx, float dy)
{
    return sqrtf(dx * dx + dy * dy);
}

void Vec2Normalize(float *dx, float *dy)
{
    float len = Vec2Length(*dx, *dy);
    if (len > 0.0f) { *dx /= len; *dy /= len; }
}


float Distance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

void Enemy_UpdateState(Enemy *e, Player *p)
{
    float d = Distance(e->x, e->y, p->x, p->y);

    switch (e->state)
    {
        case ENEMY_WAITING:
            if (d <= e->detectionRange && d > e->attackRange)
                e->state = ENEMY_FOLLOWING;
        break;

        case ENEMY_FOLLOWING:
            if (d <= e->attackRange)
                e->state = ENEMY_ATTACKING;
            else if (d > e->detectionRange)
                e->state = ENEMY_WAITING;
        break;

        case ENEMY_ATTACKING:
            if (d > e->attackRange)
                e->state = ENEMY_FOLLOWING;
        break;
    }
}

void Enemy_MoveTowardPlayer(Enemy *e, Player *p, float dt)
{
    float dx = p->x - e->x;
    float dy = p->y - e->y;

    float length = sqrtf(dx * dx + dy * dy);

    if (length != 0)
    {
        dx /= length;
        dy /= length;
    }

    e->x += dx * e->speed * dt;
    e->y += dy * e->speed * dt;

    e->rect.x = (int)e->x;
    e->rect.y = (int)e->y;

    Animation_Update(&e->anim, dx, dy, e->atlas);
}

/* ═══════════════════════════════════════════════
   ATLAS  (resource loading)
═══════════════════════════════════════════════ */

int Atlas_Load(SpriteAtlas *a, SDL_Renderer *renderer)
{
    const char *rightFiles[MAX_WALK_RIGHT] = {
        "right/f1.png","right/f2.png","right/f3.png","right/f4.png"
    };
    const char *upFiles[MAX_WALK_UP] = {
        "up/f1u.png","up/f2u.png","up/f3u.png","up/f4u.png"
    };
    const char *downFiles[MAX_WALK_DOWN] = {
        "down/f1d.png","down/f2d.png","down/f3d.png","down/f4d.png"
    };
    const char *attackFiles[MAX_ATTACK_FRAMES] = {
        "attacks/att1.png","attacks/att2.png","attacks/att3.png"
    };

    for (int i = 0; i < MAX_WALK_RIGHT;   i++) a->walkRight[i] = LoadTexture(rightFiles[i],  renderer);
    for (int i = 0; i < MAX_WALK_UP;      i++) a->walkUp[i]    = LoadTexture(upFiles[i],     renderer);
    for (int i = 0; i < MAX_WALK_DOWN;    i++) a->walkDown[i]  = LoadTexture(downFiles[i],   renderer);
    for (int i = 0; i < MAX_ATTACK_FRAMES;i++) a->attack[i]    = LoadTexture(attackFiles[i], renderer);

    for (int i = 0; i < MAX_HP_BARS; i++) {
        char buf[32];
        sprintf(buf, "hp/hpBar%d.png", i + 1);
        a->hpBars[i] = LoadTexture(buf, renderer);
    }
    return 1;
}

void Atlas_Destroy(SpriteAtlas *a)
{
    for (int i = 0; i < MAX_WALK_RIGHT;   i++) if (a->walkRight[i]) SDL_DestroyTexture(a->walkRight[i]);
    for (int i = 0; i < MAX_WALK_UP;      i++) if (a->walkUp[i])    SDL_DestroyTexture(a->walkUp[i]);
    for (int i = 0; i < MAX_WALK_DOWN;    i++) if (a->walkDown[i])  SDL_DestroyTexture(a->walkDown[i]);
    for (int i = 0; i < MAX_ATTACK_FRAMES;i++) if (a->attack[i])    SDL_DestroyTexture(a->attack[i]);
    for (int i = 0; i < MAX_HP_BARS;      i++) if (a->hpBars[i])    SDL_DestroyTexture(a->hpBars[i]);
}

/* ═══════════════════════════════════════════════
   ANIMATION
═══════════════════════════════════════════════ */

void Animation_Init(Animation *a)
{
    a->state        = ANIM_IDLE;
    a->dir          = DIR_DOWN;
    a->currentFrame = 0;
    a->frameCounter = 0;
    a->maxFrames    = MAX_WALK_DOWN;
    a->flip         = SDL_FLIP_NONE;
    a->attackFrame  = 0;
    a->attackCounter= 0;
}

void Animation_Update(Animation *a, float dx, float dy, SpriteAtlas *atlas)
{
    (void)atlas; /* reserved for future per-character atlases */

    if (a->state == ANIM_ATTACK) return; /* attack manages itself */

    /* choose direction & frame set */
    if (fabs(dx) > fabs(dy)) {
        if (dx > 0) {
            a->dir = DIR_RIGHT;
            a->maxFrames = MAX_WALK_RIGHT;
            a->flip = SDL_FLIP_NONE;
        } else if (dx < 0) {
            a->dir = DIR_LEFT;
            a->maxFrames = MAX_WALK_RIGHT;
            a->flip = SDL_FLIP_HORIZONTAL;
        }
    } else {
        if (dy < 0) {
            a->dir = DIR_UP;
            a->maxFrames = MAX_WALK_UP;
            a->flip = SDL_FLIP_NONE;
        } else if (dy > 0) {
            a->dir = DIR_DOWN;
            a->maxFrames = MAX_WALK_DOWN;
            a->flip = SDL_FLIP_NONE;
        }
    }

    if (dx != 0 || dy != 0) {
        a->state = ANIM_WALK;
        a->frameCounter++;
        if (a->frameCounter >= FRAME_DELAY) {
            a->currentFrame = (a->currentFrame + 1) % a->maxFrames;
            a->frameCounter = 0;
        }
    } else {
        a->state        = ANIM_IDLE;
        a->currentFrame = 0;
    }
}

void Animation_Render(SDL_Renderer *r, Animation *a, SpriteAtlas *atlas, SDL_Rect *dst)
{
    if (a->state == ANIM_ATTACK) {
        SDL_Texture *frame = atlas->attack[a->attackFrame];
        if (frame) SDL_RenderCopyEx(r, frame, NULL, dst, 0, NULL, a->flip);

        a->attackCounter++;
        if (a->attackCounter >= ATTACK_DELAY) {
            a->attackFrame++;
            a->attackCounter = 0;
            if (a->attackFrame >= MAX_ATTACK_FRAMES) {
                a->state        = ANIM_IDLE;
                a->attackFrame  = 0;
            }
        }
        return;
    }

    SDL_Texture *frame = NULL;
    switch (a->dir) {
        case DIR_UP:    frame = atlas->walkUp[a->currentFrame];    break;
        case DIR_DOWN:  frame = atlas->walkDown[a->currentFrame];  break;
        case DIR_RIGHT:
        case DIR_LEFT:  frame = atlas->walkRight[a->currentFrame]; break;
    }
    if (frame) SDL_RenderCopyEx(r, frame, NULL, dst, 0, NULL, a->flip);
}

/* ═══════════════════════════════════════════════
   HEALTH
═══════════════════════════════════════════════ */

void Health_Init(Health *h, int maxHP)
{
    h->max     = maxHP;
    h->current = 0; /* 0 = full (index into hpBars) */
}

void Health_Damage(Health *h, int amount)
{
    h->current += amount;
    if (h->current > h->max) h->current = h->max;
}

int Health_IsDead(Health *h)
{
    return h->current >= h->max;
}

void Health_Render(Game *g)
{
    SDL_Texture *bar = g->atlas.hpBars[g->health.current];
    if (!bar) return;
    SDL_Rect hpRect = { g->screenW - 450, 20, 400, 40 };
    SDL_RenderCopy(g->renderer, bar, NULL, &hpRect);
}

/* ═══════════════════════════════════════════════
   ATTACK / TRANSMISSION
═══════════════════════════════════════════════ */

/*  Attack_Trigger – spawn a hit-box in a given direction from the origin.
    'damage' is how many HP bar steps to subtract.
    'lifetime' gives the trigger a short window to detect a hit.         */
void Attack_Trigger(AttackTrigger *a, SDL_Rect *origin, Direction dir, int damage)
{
    if (a->active) return; /* only one trigger at a time */

    a->active   = 1;
    a->damage   = damage;
    a->dir      = dir;
    a->lifetime = 0.15f; /* 150 ms window */

    /* position hitbox just outside the sprite */
    int reach = 40;
    a->hitbox = (SDL_Rect){ origin->x, origin->y, origin->w, origin->h };
    switch (dir) {
        case DIR_UP:    a->hitbox.y -= reach; break;
        case DIR_DOWN:  a->hitbox.y += reach; break;
        case DIR_RIGHT: a->hitbox.x += reach; break;
        case DIR_LEFT:  a->hitbox.x -= reach; break;
    }
}

void Attack_Update(AttackTrigger *a, float dt)
{
    if (!a->active) return;
    a->lifetime -= dt;
    if (a->lifetime <= 0.0f) a->active = 0;
}

int Attack_CheckHit(AttackTrigger *a, SDL_Rect *target)
{
    if (!a->active) return 0;
    return SDL_HasIntersection(&a->hitbox, target);
}

void Attack_Render(Game *g)
{
    if (!g->attack.active) return;
    SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g->renderer, 255, 200, 0, 120);
    SDL_RenderFillRect(g->renderer, &g->attack.hitbox);
}

/* ═══════════════════════════════════════════════
   COLLISION
═══════════════════════════════════════════════ */

int Collision_Check(SDL_Rect *a, SDL_Rect *b)
{
    return SDL_HasIntersection(a, b);
}

/*  Resolve: if enemy and player overlap, revert both to their previous
    positions, start enemy attack animation and reduce player HP.        */
void Collision_ResolveEnemyPlayer(Game *g, float oldEX, float oldEY,
                                  float oldPX, float oldPY)
{
    g->enemy.x = oldEX;
    g->enemy.y = oldEY;
    g->player.x = oldPX;
    g->player.y = oldPY;

    /* Sync rects */
    g->enemy.rect.x  = (int)g->enemy.x;
    g->enemy.rect.y  = (int)g->enemy.y;
    g->player.rect.x = (int)g->player.x;
    g->player.rect.y = (int)g->player.y;

    /* Start enemy attack if not already attacking */
    if (g->enemy.anim.state != ANIM_ATTACK) {
        g->enemy.anim.state        = ANIM_ATTACK;
        g->enemy.anim.attackFrame  = 0;
        g->enemy.anim.attackCounter= 0;

        /* Face player */
        float dxFace = g->player.rect.x - g->enemy.rect.x;
        float dyFace = g->player.rect.y - g->enemy.rect.y;
        if (fabsf(dxFace) > fabsf(dyFace)) {
            g->enemy.anim.dir  = (dxFace > 0) ? DIR_RIGHT : DIR_LEFT;
            g->enemy.anim.flip = (dxFace > 0) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
        } else {
            g->enemy.anim.dir  = (dyFace < 0) ? DIR_UP : DIR_DOWN;
            g->enemy.anim.flip = SDL_FLIP_NONE;
        }

        /* Deal damage */
        if (!Health_IsDead(&g->health))
            Health_Damage(&g->health, 1);

        /* Spawn an attack trigger in the facing direction */
        Attack_Trigger(&g->attack, &g->enemy.rect, g->enemy.anim.dir, 1);
    }
}

/* ═══════════════════════════════════════════════
   ENEMY
═══════════════════════════════════════════════ */

void Enemy_ChooseNewTarget(Enemy *e, int screenW, int screenH)
{
    /* Axis-aligned movement: randomly pick a horizontal or vertical target */
    if (rand() % 2)
        e->targetX = (float)(rand() % (screenW - e->w)), e->targetY = e->y;
    else
        e->targetY = (float)(rand() % (screenH - e->h)), e->targetX = e->x;
}

void Enemy_Init(Game *g)
{
    Enemy *e  = &g->enemy;
    e->w      = 64;
    e->h      = 64;
    e->x      = (float)(g->screenW / 2);
    e->y      = (float)(g->screenH / 2);
    e->speed  = PLAYER_SPEED;
    e->atlas  = &g->atlas;
    Enemy_ChooseNewTarget(e, g->screenW, g->screenH);
    Animation_Init(&e->anim);
    e->rect = (SDL_Rect){ (int)e->x, (int)e->y, e->w, e->h };

    e->state = ENEMY_WAITING;
    e->detectionRange = 300.0f;
    e->attackRange = 80.0f;

    Enemy *e2 = &g->enemy2;
    e2->w = 64;
    e2->h = 64;
    e2->x = 700;
    e2->y = 400;
    e2->speed = 120.0f;
    e2->atlas = &g->atlas;
    e2->state = ENEMY_WAITING;
    e2->detectionRange = 300.0f;
    e2->attackRange = 80.0f;
    Animation_Init(&e2->anim);
    e2->rect = (SDL_Rect){ (int)e2->x, (int)e2->y, e2->w, e2->h };
}

void Enemy_Update(Enemy *e, float dt, int screenW, int screenH)
{
    if (e->anim.state == ANIM_ATTACK) return; /* frozen during attack */

    float dx = e->targetX - e->x;
    float dy = e->targetY - e->y;

    /* Axis-lock: move only horizontally or vertically */
    if (fabsf(dx) > fabsf(dy)) dy = 0.0f; else dx = 0.0f;

    float dist = Vec2Length(dx, dy);
    if (dist > 2.0f) {
        Vec2Normalize(&dx, &dy);
        e->x += dx * e->speed * dt;
        e->y += dy * e->speed * dt;
    } else {
        Enemy_ChooseNewTarget(e, screenW, screenH);
        dx = dy = 0.0f;
    }

    e->rect.x = (int)e->x;
    e->rect.y = (int)e->y;

    Animation_Update(&e->anim, dx, dy, e->atlas);
}

void Enemy_Render(Game *g)
{
    SDL_Rect dst = { (int)g->enemy.x, (int)g->enemy.y, g->enemy.w, g->enemy.h };
    Animation_Render(g->renderer, &g->enemy.anim, g->enemy.atlas, &dst);

    SDL_Rect dst2 = { (int)g->enemy2.x, (int)g->enemy2.y, g->enemy2.w, g->enemy2.h };
    Animation_Render(g->renderer, &g->enemy2.anim, g->enemy2.atlas, &dst2);
}

/* ═══════════════════════════════════════════════
   PLAYER  (ZQSD-controlled red block)
═══════════════════════════════════════════════ */

void Player_Init(Game *g)
{
    Player *p  = &g->player;
    p->w       = 40;
    p->h       = 40;
    p->x       = 100.0f;
    p->y       = 100.0f;
    p->speed   = RECT_SPEED;
    p->color   = (SDL_Color){ 0, 0, 255, 255 };
    p->rect    = (SDL_Rect){ (int)p->x, (int)p->y, p->w, p->h };
    p->up = p->down = p->left = p->right = 0;
}

void Player_Update(Game *g, float dt)
{
    Player *p = &g->player;
    if (p->up)    p->y -= p->speed * dt;
    if (p->down)  p->y += p->speed * dt;
    if (p->left)  p->x -= p->speed * dt;
    if (p->right) p->x += p->speed * dt;
    p->rect.x = (int)p->x;
    p->rect.y = (int)p->y;
}

void Player_Render(Game *g)
{
    SDL_SetRenderDrawColor(g->renderer,
        g->player.color.r, g->player.color.g,
        g->player.color.b, g->player.color.a);
    SDL_RenderFillRect(g->renderer, &g->player.rect);
}

/* ═══════════════════════════════════════════════
   MENU
═══════════════════════════════════════════════ */

void Menu_Init(Game *g)
{
    Menu *m = &g->menu;
    m->visible      = 0;
    m->boardTexture = LoadTexture("menuBoard.png", g->renderer);

    const char *btnPaths[MAX_BUTTONS] = {
        "bottons/resume.png",  "bottons/save.png",   "bottons/load.png",
        "bottons/players.png", "bottons/options.png","bottons/scores.png",
        "bottons/quit.png"
    };
    for (int i = 0; i < MAX_BUTTONS; i++)
        m->buttons[i].texture = LoadTexture(btnPaths[i], g->renderer);
}

void Menu_ComputeLayout(Menu *m, int screenW, int screenH)
{
    if (!m->boardTexture) return;

    int texW, texH;
    SDL_QueryTexture(m->boardTexture, NULL, NULL, &texW, &texH);

    m->boardRect.w = (int)(texW * BOARD_SCALE);
    m->boardRect.h = (int)(texH * BOARD_SCALE);
    m->boardRect.x = 20;
    m->boardRect.y = screenH - m->boardRect.h - 20;

    int bw      = (int)(m->boardRect.w * 0.40f);
    int bh      = (int)(m->boardRect.h * 0.10f);
    int padLeft = (int)(m->boardRect.w * 0.04f);
    int padTop  = (int)(m->boardRect.h * 0.16f);
    int gap     = 10;

    for (int i = 0; i < MAX_BUTTONS; i++) {
        m->buttons[i].rect.x = m->boardRect.x + padLeft;
        m->buttons[i].rect.y = m->boardRect.y + padTop + i * (bh + gap);
        m->buttons[i].rect.w = bw;
        m->buttons[i].rect.h = bh;
    }
}

void Menu_HandleClick(Game *g, int mx, int my)
{
    Menu *m = &g->menu;
    if (!m->visible) return;

    SDL_Point p = { mx, my };
    for (int i = 0; i < MAX_BUTTONS; i++) {
        if (SDL_PointInRect(&p, &m->buttons[i].rect)) {
            switch (i) {
                case 0: /* Resume */
                    m->visible = 0;
                    g->paused  = 0;
                    break;
                case 6: /* Quit */
                    g->running = 0;
                    break;
                /* cases 1-5: save / load / players / options / scores – stub */
                default:
                    printf("[Menu] Button %d pressed (not yet implemented)\n", i);
                    break;
            }
            break;
        }
    }
}

void Menu_Render(Game *g)
{
    Menu *m = &g->menu;
    if (!m->visible) return;

    /* Dim overlay */
    SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 120);
    SDL_Rect overlay = { 0, 0, g->screenW, g->screenH };
    SDL_RenderFillRect(g->renderer, &overlay);

    if (m->boardTexture)
        SDL_RenderCopy(g->renderer, m->boardTexture, NULL, &m->boardRect);

    for (int i = 0; i < MAX_BUTTONS; i++)
        if (m->buttons[i].texture)
            SDL_RenderCopy(g->renderer, m->buttons[i].texture, NULL, &m->buttons[i].rect);
}

/* ═══════════════════════════════════════════════
   INPUT
═══════════════════════════════════════════════ */

void Input_Poll(Game *g)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            g->running = 0;
        }

        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
            Menu_HandleClick(g, e.button.x, e.button.y);

        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    g->paused = !g->paused;
                    g->menu.visible = g->paused;
                    if (g->paused)
                        Menu_ComputeLayout(&g->menu, g->screenW, g->screenH);
                    break;
                case SDLK_z: g->player.up    = 1; break;
                case SDLK_s: g->player.down  = 1; break;
                case SDLK_q: g->player.left  = 1; break;
                case SDLK_d: g->player.right = 1; break;
            }
        }

        if (e.type == SDL_KEYUP) {
            switch (e.key.keysym.sym) {
                case SDLK_z: g->player.up    = 0; break;
                case SDLK_s: g->player.down  = 0; break;
                case SDLK_q: g->player.left  = 0; break;
                case SDLK_d: g->player.right = 0; break;
            }
        }
    }
}

/* ═══════════════════════════════════════════════
   STAR  COLLECTIBLE
═══════════════════════════════════════════════ */

/*  Star_Respawn – move the star to a new random position on screen.
    Keeps a margin so the star never spawns outside visible area.       */
void Star_Respawn(Game *g)
{
    int margin = STAR_SIZE * 2;
    g->star.rect.x = margin + rand() % (g->screenW - STAR_SIZE - margin * 2);
    g->star.rect.y = margin + rand() % (g->screenH - STAR_SIZE - margin * 2);
}

void Star_Init(Game *g)
{
    g->star.rect.w   = STAR_SIZE;
    g->star.rect.h   = STAR_SIZE;
    g->star.score    = 0;
    /* Try to load an image; fall back to drawing a yellow square if absent */
    g->star.texture  = LoadTexture("star.png", g->renderer);
    Star_Respawn(g);
}

/*  Star_Update – check if the player rectangle overlaps the star.
    If so, increment score and teleport the star to a new location.     */
void Star_Update(Game *g)
{
    if (Collision_Check(&g->player.rect, &g->star.rect)) {
        g->star.score++;
        printf("[Star] Collected! Total: %d\n", g->star.score);
        Star_Respawn(g);
    }
}

/*  Star_Render – draw the star texture or a simple yellow polygon.
    The polygon is a 5-point star drawn with SDL lines as fallback.     */
void DrawStarShape(SDL_Renderer *r, int cx, int cy, int radius)
{
    /* 5-pointed star: alternate outer (72°) and inner (36°) points */
    float points[10][2];
    float innerR = radius * 0.45f;
    for (int i = 0; i < 10; i++) {
        float angle = (float)i * 3.14159f / 5.0f - 3.14159f / 2.0f;
        float rad   = (i % 2 == 0) ? (float)radius : innerR;
        points[i][0] = cx + rad * cosf(angle);
        points[i][1] = cy + rad * sinf(angle);
    }
    SDL_SetRenderDrawColor(r, 255, 220, 0, 255);
    for (int i = 0; i < 10; i++) {
        int next = (i + 1) % 10;
        SDL_RenderDrawLine(r,
            (int)points[i][0], (int)points[i][1],
            (int)points[next][0], (int)points[next][1]);
    }
    /* Filled approximation: draw from center to each edge */
    SDL_SetRenderDrawColor(r, 255, 200, 0, 200);
    for (int i = 0; i < 10; i++) {
        int next = (i + 1) % 10;
        SDL_RenderDrawLine(r, cx, cy, (int)points[i][0],    (int)points[i][1]);
        SDL_RenderDrawLine(r, cx, cy, (int)points[next][0], (int)points[next][1]);
    }
}

void Star_Render(Game *g)
{
    if (g->star.texture) {
        SDL_RenderCopy(g->renderer, g->star.texture, NULL, &g->star.rect);
    } else {
        int cx = g->star.rect.x + STAR_SIZE / 2;
        int cy = g->star.rect.y + STAR_SIZE / 2;
        DrawStarShape(g->renderer, cx, cy, STAR_SIZE / 2);
    }
}

/* ═══════════════════════════════════════════════
   GAME LIFECYCLE
═══════════════════════════════════════════════ */

int Game_Init(Game *g)
{
    srand((unsigned)time(NULL));

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);
    g->screenW = dm.w;
    g->screenH = dm.h;

    g->window = SDL_CreateWindow("Game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        g->screenW, g->screenH,
        SDL_WINDOW_FULLSCREEN_DESKTOP);

    g->renderer = SDL_CreateRenderer(g->window, -1, SDL_RENDERER_ACCELERATED);

    if (!g->window || !g->renderer) return 0;

    Atlas_Load(&g->atlas, g->renderer);

    Enemy_Init(g);
    Player_Init(g);
    Health_Init(&g->health, MAX_HP_BARS - 1);
    Menu_Init(g);
    Star_Init(g);

    g->attack.active   = 0;
    g->running         = 1;
    g->paused          = 0;

    return 1;
}

void Game_Shutdown(Game *g)
{
    Atlas_Destroy(&g->atlas);

    if (g->star.texture)    SDL_DestroyTexture(g->star.texture);
    if (g->menu.boardTexture) SDL_DestroyTexture(g->menu.boardTexture);
    for (int i = 0; i < MAX_BUTTONS; i++)
        if (g->menu.buttons[i].texture) SDL_DestroyTexture(g->menu.buttons[i].texture);

    SDL_DestroyRenderer(g->renderer);
    SDL_DestroyWindow(g->window);
    IMG_Quit();
    SDL_Quit();
}

void Game_Run(Game *g)
{
    Uint64 now  = SDL_GetPerformanceCounter();
    Uint64 last = 0;
    float  dt;

    while (g->running) {
        last = now;
        now  = SDL_GetPerformanceCounter();
        dt   = (float)(now - last) / (float)SDL_GetPerformanceFrequency();
        if (dt > 0.05f) dt = 0.05f; /* cap delta at 50 ms */

        Input_Poll(g);

        if (!g->paused) {
            /* Save old positions for collision revert */
            float oldEX = g->enemy.x,  oldEY = g->enemy.y;
            float oldE2X = g->enemy2.x, oldE2Y = g->enemy2.y;
            float oldPX = g->player.x, oldPY = g->player.y;

            Enemy_Update(&g->enemy, dt, g->screenW, g->screenH);

            Enemy_UpdateState(&g->enemy2, &g->player);

            switch (g->enemy2.state)
            {
                case ENEMY_WAITING:
                    Animation_Update(&g->enemy2.anim, 0, 0, g->enemy2.atlas);
                    break;

                case ENEMY_FOLLOWING:
                    Enemy_MoveTowardPlayer(&g->enemy2, &g->player, dt);
                    break;

                case ENEMY_ATTACKING:
                    g->enemy2.anim.state = ANIM_ATTACK;
                    break;
            }

            Player_Update(g, dt);
            Attack_Update(&g->attack, dt);
            Star_Update(g);

            /* Collision: first enemy vs player */
            if (Collision_Check(&g->enemy.rect, &g->player.rect))
                Collision_ResolveEnemyPlayer(g, oldEX, oldEY, oldPX, oldPY);

            /* Collision: second enemy vs player */
            if (Collision_Check(&g->enemy2.rect, &g->player.rect))
            {
                g->enemy2.x = oldE2X;
                g->enemy2.y = oldE2Y;

                g->enemy2.rect.x = (int)g->enemy2.x;
                g->enemy2.rect.y = (int)g->enemy2.y;

                g->player.x = oldPX;
                g->player.y = oldPY;

                g->player.rect.x = (int)g->player.x;
                g->player.rect.y = (int)g->player.y;

                g->enemy2.anim.state = ANIM_ATTACK;

                if (!Health_IsDead(&g->health))
                    Health_Damage(&g->health, 1);
            }
        }

        /* ── Render ── */
        SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 255);
        SDL_RenderClear(g->renderer);

        Enemy_Render(g);
        Player_Render(g);
        Star_Render(g);
        Attack_Render(g);
        Health_Render(g);
        Menu_Render(g);

        SDL_RenderPresent(g->renderer);
    }
}
