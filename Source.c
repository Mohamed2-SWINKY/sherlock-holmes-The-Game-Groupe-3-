/* source.c - All game logic and rendering functions */
#include "header.h"

/* -------------------------------------------------------
   Helper: check if two SDL_Rects overlap
   ------------------------------------------------------- */
int rects_overlap(SDL_Rect a, SDL_Rect b)
{
    return (a.x < b.x + b.w && a.x + a.w > b.x &&
            a.y < b.y + b.h && a.y + a.h > b.y);
}

/* Helper: filled colour rectangle */
void filled_rect(SDL_Renderer *r, SDL_Rect rect, SDL_Color col)
{
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(r, &rect);
}


 /* ── HUD (score + keys + HP bar — fixed screen positions) ── */
        char scoreText[64];
        snprintf(scoreText, sizeof(scoreText),
                 side == 0 ? "P1 Score: %d | Keys: %d" : "P2 Score: %d | Keys: %d",
                 side == 0 ? ctx->player1.score : ctx->player2.score,
                 side == 0 ? ctx->player1.keyCount : ctx->player2.keyCount);
 
        SDL_Surface *surf = TTF_RenderText_Blended(
            ctx->font, scoreText, (SDL_Color){255, 255, 255, 255});
        SDL_Texture *stx = SDL_CreateTextureFromSurface(ctx->renderer, surf);
        SDL_FreeSurface(surf);
        SDL_Rect sd; SDL_QueryTexture(stx, NULL, NULL, &sd.w, &sd.h);


/* Helper: render UTF-8 text at (x,y) */
void draw_text(Game *g, TTF_Font *fnt, const char *txt,
               int x, int y, SDL_Color col)
{
    if (!fnt) return;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(fnt, txt, col);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(g->renderer, surf);
    SDL_Rect dst = { x, y, surf->w, surf->h };
    SDL_FreeSurface(surf);
    if (!tex) return;
    SDL_RenderCopy(g->renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

/* -------------------------------------------------------
   game_init – SDL, window, renderer, textures, fonts
   ------------------------------------------------------- */
int game_init(Game *g)
{
    memset(g, 0, sizeof(Game));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 0;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "IMG_Init: %s\n", IMG_GetError());
        return 0;
    }
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        /* continue without fonts */
    }

    g->window = SDL_CreateWindow("Sherlock Holmes",
                                 SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED,
                                 WINDOW_W, WINDOW_H, 0);
    if (!g->window) return 0;

    g->renderer = SDL_CreateRenderer(g->window, -1,
                      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g->renderer) return 0;

    /* Load map textures
       Map 1: assets/Gemini_Generated_Image_lvee1dlvee1dlvee.png
       Map 2: assets/Gemini_Generated_Image_9hznjl9hznjl9hzn.png  */
    g->tex_map1 = IMG_LoadTexture(g->renderer,
        "assets/background/background1.png");
    g->tex_map2 = IMG_LoadTexture(g->renderer,
        "assets/background/background2.png");

    /* Load box textures */
    g->tex_box        = IMG_LoadTexture(g->renderer, "assets/box.png");
    g->tex_broken_box = IMG_LoadTexture(g->renderer, "assets/broken box.png");

    if (!g->tex_map1 || !g->tex_map2 || !g->tex_box || !g->tex_broken_box) {
        fprintf(stderr, "IMG_LoadTexture: %s\n", IMG_GetError());
        return 0;
    }

    /* Try to load a system font – place any .ttf in assets/ as font.ttf */
    g->font_lg = TTF_OpenFont("assets/font.ttf", 28);
    g->font_sm = TTF_OpenFont("assets/font.ttf", 16);
    if (!g->font_lg) {
        /* fallback to common system paths */
        g->font_lg = TTF_OpenFont(
            "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 28);
        g->font_sm = TTF_OpenFont(
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16);
    }
    if (!g->font_lg)
        fprintf(stderr, "Warning: no font found – text disabled.\n");

    g->state       = STATE_GUIDE1;
    g->mode        = MODE_MONO;
    g->timer_on    = 0;
    g->timer_start = 0;

    setup_level1(g);

    /* Initialise 4-direction scroll cameras */
    camera_init(&g->cam1, WINDOW_W, WINDOW_H);
    camera_init(&g->cam2, WINDOW_W / 2, WINDOW_H);

    return 1;
}

/* -------------------------------------------------------
   game_cleanup
   ------------------------------------------------------- */
void game_cleanup(Game *g)
{
    if (g->font_lg)       TTF_CloseFont(g->font_lg);
    if (g->font_sm)       TTF_CloseFont(g->font_sm);
    if (g->tex_map1)      SDL_DestroyTexture(g->tex_map1);
    if (g->tex_map2)      SDL_DestroyTexture(g->tex_map2);
    if (g->tex_box)       SDL_DestroyTexture(g->tex_box);
    if (g->tex_broken_box)SDL_DestroyTexture(g->tex_broken_box);
    if (g->renderer)      SDL_DestroyRenderer(g->renderer);
    if (g->window)        SDL_DestroyWindow(g->window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

/* -------------------------------------------------------
   Macro: add an obstacle rectangle
   ------------------------------------------------------- */
#define ADD_OBS1(X,Y,W,H) \
    g->obs1[g->obs1_cnt++] = (SDL_Rect){X,Y,W,H}
#define ADD_OBS2(X,Y,W,H) \
    g->obs2[g->obs2_cnt++] = (SDL_Rect){X,Y,W,H}

/* -------------------------------------------------------
   setup_level1 – obstacles, keys, doors, falling box
   Obstacles match furniture visible in Map 1 image
   ------------------------------------------------------- */
void setup_level1(Game *g)
{
    g->obs1_cnt   = 0;
    g->keys1_cnt  = 0;
    g->doors1_cnt = 0;
    memset(g->door1_open, 0, sizeof(g->door1_open));

    /* === OUTER PERIMETER === */
    /* House top wall (gap at 558-602 for balcony door) */
    ADD_OBS1(378, 218, 180, 12);  /* left of gap  */
    ADD_OBS1(602, 218, 326, 12);  /* right of gap */
    /* House left wall (gap at 378-452 for outdoor entry) */
    ADD_OBS1(378, 218, 12, 148);
    ADD_OBS1(378, 458, 12, 276);
    /* House right wall */
    ADD_OBS1(906, 218, 12, 510);
    /* House bottom wall (gap at 612-666 for exit door) */
    ADD_OBS1(378, 728, 234, 12);
    ADD_OBS1(666, 728, 252, 12);
    /* Hallway walls */
    ADD_OBS1(1050, 218, 12, 112);
    ADD_OBS1(1050, 412, 12, 316);
    /* Dungeon outer walls */
    ADD_OBS1(1062, 218, 326, 12);  /* dungeon top   */
    ADD_OBS1(1386, 218, 12, 510);  /* dungeon right  */
    ADD_OBS1(1062, 728, 324, 12);  /* dungeon bottom */

    /* === INDOOR VERTICAL WALL (left-room / right-room divider) ===
       Gap at y 328-392 for passage */
    ADD_OBS1(660, 218, 12, 110);
    ADD_OBS1(660, 392, 12, 348);

    /* === HORIZONTAL WALL dividing right rooms ===
       Gap at x 774-846 for passage */
    ADD_OBS1(660, 448, 114, 12);
    ADD_OBS1(844, 448, 74, 12);

    /* === LEFT ROOM FURNITURE (living room + lab) === */
    /* Bookshelf left */
    ADD_OBS1(392, 268, 72, 108);
    /* Bookshelf right */
    ADD_OBS1(478, 268, 72, 108);
    /* Fireplace centre-top */
    ADD_OBS1(482, 218, 80, 65);
    /* Green sofa */
    ADD_OBS1(434, 372, 160, 70);
    /* Small end table */
    ADD_OBS1(386, 356, 40, 50);
    /* Chemistry/lab table bottom-left */
    ADD_OBS1(386, 560, 98, 88);
    /* Bottles table */
    ADD_OBS1(408, 600, 166, 76);
    /* Stool */
    ADD_OBS1(430, 538, 32, 32);

    /* === RIGHT ROOMS FURNITURE === */
    /* Notice board / desk (top right room) */
    ADD_OBS1(688, 248, 160, 105);
    /* Middle desk */
    ADD_OBS1(672, 342, 170, 72);
    /* Filing cabinets right side */
    ADD_OBS1(854, 334, 50, 128);
    /* Bottom bookshelf */
    ADD_OBS1(674, 484, 150, 90);
    /* Bottom filing cabinets */
    ADD_OBS1(846, 486, 52, 90);

    /* === DUNGEON RIGHT AREA FURNITURE === */
    /* Barrels cluster top */
    ADD_OBS1(1208, 282, 122, 80);
    /* Static crates top-right (gap left for falling box at 1318,258) */
    ADD_OBS1(1314, 358, 76, 50);  /* crate base below falling box */
    /* Stairs centre */
    ADD_OBS1(1136, 432, 62, 126);
    /* Barrels bottom-left */
    ADD_OBS1(1206, 546, 116, 80);
    /* Small crate bottom */
    ADD_OBS1(1152, 598, 50, 50);

    /* === BALCONY (y: 0-218) obstacles === */
    /* Rooftop structure centre */
    ADD_OBS1(460, 8,  230, 120);
    /* Balcony right boxes (static) */
    ADD_OBS1(862, 60,  58,  80);
    ADD_OBS1(862, 130, 58,  70);
    /* Balcony chimney stacks */
    ADD_OBS1(528, 10,  28, 30);
    ADD_OBS1(640, 10,  28, 30);

    /* === OUTDOOR FENCE === */
    ADD_OBS1(10, 218,  20, 252);  /* top fence segment left  */
    ADD_OBS1(10, 478,  20, 250);  /* bottom fence segment    */
    ADD_OBS1(346, 218, 20, 252);
    ADD_OBS1(346, 478, 20, 250);

    /* ---- Keys ---- */
    /* Key 0: spawns from broken box (hidden until box breaks) */
    g->keys1[0].id      = 0;
    /* Key spawns just below where the box lands (BOX_LAND_Y + BOX_H) */
    g->keys1[0].rect    = (SDL_Rect){ BOX_START_X + 13, BOX_LAND_Y + BOX_H + 6, 22, 22 };
    g->keys1[0].visible = 0;   /* becomes visible when box breaks */
    g->keys1[0].collected = 0;
    /* Key 1: resting on balcony floor */
    g->keys1[1].id      = 1;
    g->keys1[1].rect    = (SDL_Rect){ 498, 148, 22, 22 };
    g->keys1[1].visible = 1;
    g->keys1[1].collected = 0;
    g->keys1_cnt = 2;

    /* ---- Doors ---- */
    /* Door 0: balcony door (top wall gap, needs key 0) */
    g->doors1[0].rect     = (SDL_Rect){ 558, 210, 44, 18 };
    g->doors1[0].locked   = 1;
    g->doors1[0].key_id   = 0;
    g->doors1[0].to_level = -1;
    /* Door 1: exit door (bottom wall, needs key 1) → Level 2 */
    g->doors1[1].rect     = (SDL_Rect){ 612, 724, 54, 16 };
    g->doors1[1].locked   = 1;
    g->doors1[1].key_id   = 1;
    g->doors1[1].to_level = LEVEL_2;
    g->doors1_cnt = 2;

    /* ---- Falling box ---- */
    g->fbox.rect   = (SDL_Rect){ BOX_START_X, BOX_START_Y, BOX_W, BOX_H };
    g->fbox.fy     = (float)BOX_START_Y;
    g->fbox.vel    = 0.0f;
    g->fbox.state  = BOX_IDLE;

    /* ---- Player start ---- */
    g->p1.rect = (SDL_Rect){ 500, 400, PLAYER_W, PLAYER_H };
    g->p2.rect = (SDL_Rect){ 540, 400, PLAYER_W, PLAYER_H };
    memset(g->p1.has_key, 0, sizeof(g->p1.has_key));
    memset(g->p2.has_key, 0, sizeof(g->p2.has_key));
}

/* -------------------------------------------------------
   setup_level2 – dungeon map obstacles, keys, doors
   Obstacles match furniture in Map 2 image
   ------------------------------------------------------- */
void setup_level2(Game *g)
{
    g->obs2_cnt   = 0;
    g->keys2_cnt  = 0;
    g->doors2_cnt = 0;
    memset(g->door2_open, 0, sizeof(g->door2_open));

    /* === OUTER WALLS === */
    ADD_OBS2(0,   0,   8,   742);
    ADD_OBS2(0,   0,   1400, 8);
    ADD_OBS2(1392, 0,  8,  742);
    ADD_OBS2(0,   734, 1400, 8);

    /* === TOP ROW ROOM WALLS === */
    /* Horizontal wall below top row (with gaps) */
    ADD_OBS2(8,   240,  384, 12);  /* left segment  */
    ADD_OBS2(680, 240,  714, 12);  /* right segment */
    /* Vertical walls top row */
    ADD_OBS2(388,  8,  12, 232);
    ADD_OBS2(682,  8,  12, 232);
    ADD_OBS2(950,  8,  12, 232);

    /* === TOP-LEFT ROOM (x:8-388, y:8-240) === */
    /* Barrel */
    ADD_OBS2(20, 112, 65, 75);
    /* Desk with skull */
    ADD_OBS2(118, 108, 160, 65);
    /* Table */
    ADD_OBS2(118, 188, 160, 42);

    /* === TOP-CENTRE STAIRS (x:388-682, y:8-240) === */
    ADD_OBS2(488, 88, 98, 130);   /* staircase structure */

    /* === TOP-RIGHT ROOM (x:950-1392, y:8-240) === */
    ADD_OBS2(1058, 38,  115, 78);  /* fireplace           */
    ADD_OBS2(1078, 145, 192, 88);  /* ritual altar table  */
    ADD_OBS2(958,  68,  24,  55);  /* candelabra left     */
    ADD_OBS2(1356, 68,  24,  55);  /* candelabra right    */

    /* === MIDDLE ROW WALLS === */
    /* Horizontal wall below middle row (with gaps) */
    ADD_OBS2(8,   500,  388, 12);
    ADD_OBS2(680, 500,  714, 12);
    /* Vertical walls mid-row */
    ADD_OBS2(388, 252, 12, 248);
    ADD_OBS2(680, 252, 12, 248);

    /* === BLACK PIT OBSTACLES (holes in the floor) === */
    ADD_OBS2(394, 250, 154, 70);
    ADD_OBS2(700, 250, 124, 70);
    ADD_OBS2(394, 432, 88,  68);
    ADD_OBS2(620, 432, 108, 68);

    /* === MIDDLE-LEFT ROOM (x:8-388, y:252-500) === */
    /* Portrait */
    ADD_OBS2(20,  302, 84,  76);
    /* Weapon rack / display case */
    ADD_OBS2(96,  302, 108, 76);
    /* Chest of drawers */
    ADD_OBS2(172, 300, 152, 72);

    /* === MIDDLE-RIGHT SECTION (x:680-1392) === */
    /* Bookshelf left */
    ADD_OBS2(696, 376, 182, 86);
    /* Notice board */
    ADD_OBS2(902, 346, 144, 86);
    /* Bookshelf right */
    ADD_OBS2(1058, 346, 292, 86);

    /* === BOTTOM ROW WALLS === */
    ADD_OBS2(388, 512, 12, 222);
    ADD_OBS2(680, 512, 12, 222);

    /* === BOTTOM-LEFT ROOM (x:8-388, y:512-734) === */
    /* Chain/bed frame */
    ADD_OBS2(16, 512, 156, 82);
    /* Hay beds */
    ADD_OBS2(184, 512, 148, 82);

    /* === BOTTOM CENTRE === */
    /* Rock pile */
    ADD_OBS2(454, 550, 96, 88);
    /* Safe / vault */
    ADD_OBS2(456, 594, 68, 68);

    /* === BOTTOM-RIGHT GARGOYLE HALL (x:680-1392) === */
    ADD_OBS2(694,  560, 46, 72);   /* gargoyle 1 */
    ADD_OBS2(780,  560, 46, 72);   /* gargoyle 2 */
    ADD_OBS2(1162, 560, 46, 72);   /* gargoyle 3 */
    ADD_OBS2(1246, 560, 46, 72);   /* gargoyle 4 */
    ADD_OBS2(1332, 560, 46, 72);   /* gargoyle 5 */
    /* Altar pedestal in front of door */
    ADD_OBS2(900, 610, 80, 48);

    /* ---- Key ---- */
    /* Hidden key under the desk in top-left room */
    g->keys2[0].id        = 0;
    g->keys2[0].rect      = (SDL_Rect){ 148, 145, 22, 22 };
    g->keys2[0].visible   = 1;
    g->keys2[0].collected = 0;
    g->keys2_cnt = 1;

    /* ---- Door ---- */
    /* Exit door – bottom of gargoyle hall (key 0, completes game) */
    g->doors2[0].rect     = (SDL_Rect){ 1108, 710, 90, 14 };
    g->doors2[0].locked   = 1;
    g->doors2[0].key_id   = 0;
    g->doors2[0].to_level = -2;  /* -2 = game complete */
    g->doors2_cnt = 1;

    /* ---- Player start (near top-centre staircase exit) ---- */
    g->p1.rect = (SDL_Rect){ 530, 150, PLAYER_W, PLAYER_H };
    g->p2.rect = (SDL_Rect){ 564, 150, PLAYER_W, PLAYER_H };
    memset(g->p1.has_key, 0, sizeof(g->p1.has_key));
    memset(g->p2.has_key, 0, sizeof(g->p2.has_key));
}

/* -------------------------------------------------------
   update_timer_str – fill buf with HH:MM:SS
   ------------------------------------------------------- */
void update_timer_str(Game *g, char *buf)
{
    if (!g->timer_on) {
        snprintf(buf, TIMER_BUF, "00:00:00");
        return;
    }
    Uint32 elapsed = SDL_GetTicks() - g->timer_start;
    int total_s = (int)(elapsed / 1000);
    int h = total_s / 3600;
    int m = (total_s % 3600) / 60;
    int s = total_s % 60;
    snprintf(buf, TIMER_BUF, "%02d:%02d:%02d", h, m, s);
}

/* -------------------------------------------------------
   update_falling_box – trigger + physics
   ------------------------------------------------------- */
void update_falling_box(Game *g)
{
    FallingBox *fb = &g->fbox;
    if (fb->state == BOX_BROKEN) return;

    /* Trigger when either player is close */
    if (fb->state == BOX_IDLE) {
        int dx1 = g->p1.rect.x - fb->rect.x;
        int dy1 = g->p1.rect.y - fb->rect.y;
        int dx2 = g->p2.rect.x - fb->rect.x;
        int dy2 = g->p2.rect.y - fb->rect.y;
        float d1 = sqrtf((float)(dx1*dx1 + dy1*dy1));
        float d2 = sqrtf((float)(dx2*dx2 + dy2*dy2));
        if (d1 < BOX_TRIGGER_DIST || d2 < BOX_TRIGGER_DIST) {
            fb->state = BOX_FALLING;
            fb->vel   = 0.0f;
        }
        return;
    }

    /* Falling */
    fb->vel += 0.5f;
    if (fb->vel > BOX_FALL_SPEED) fb->vel = BOX_FALL_SPEED;
    fb->fy += fb->vel;
    fb->rect.y = (int)fb->fy;

    if (fb->rect.y >= BOX_LAND_Y) {
        fb->rect.y = BOX_LAND_Y;
        fb->fy     = (float)BOX_LAND_Y;
        fb->state  = BOX_BROKEN;
        /* Reveal key 0 */
        g->keys1[0].visible = 1;
    }
}

/* -------------------------------------------------------
   update_player – move + collide + pick keys + use doors
   ------------------------------------------------------- */
void update_player(Game *g, Player *p, int up, int dn, int lt, int rt)
{
    SDL_Rect *obs;
    int       obs_cnt;
    Key      *kl;
    int       kc;
    Door     *dl;
    int       dc;
    int      *door_open;

    if (g->level == LEVEL_1) {
        obs = g->obs1; obs_cnt = g->obs1_cnt;
        kl  = g->keys1; kc = g->keys1_cnt;
        dl  = g->doors1; dc = g->doors1_cnt;
        door_open = g->door1_open;
    } else {
        obs = g->obs2; obs_cnt = g->obs2_cnt;
        kl  = g->keys2; kc = g->keys2_cnt;
        dl  = g->doors2; dc = g->doors2_cnt;
        door_open = g->door2_open;
    }

    SDL_Rect next = p->rect;

    /* --- Horizontal movement --- */
    if (lt) next.x -= PLAYER_SPEED;
    if (rt) next.x += PLAYER_SPEED;

    /* Clamp to map */
    if (next.x < 0)               next.x = 0;
    if (next.x + next.w > MAP_W)  next.x = MAP_W - next.w;

    /* Obstacle collisions (X) */
    int blocked_x = 0;
    for (int i = 0; i < obs_cnt; i++)
        if (rects_overlap(next, obs[i])) { blocked_x = 1; break; }

    /* Door collisions (X) – locked doors block */
    if (!blocked_x) {
        for (int i = 0; i < dc; i++)
            if (dl[i].locked && !door_open[i] && rects_overlap(next, dl[i].rect))
            { blocked_x = 1; break; }
    }
    if (blocked_x) next.x = p->rect.x;

    /* --- Vertical movement --- */
    if (up) next.y -= PLAYER_SPEED;
    if (dn) next.y += PLAYER_SPEED;

    if (next.y < 0)               next.y = 0;
    if (next.y + next.h > MAP_H)  next.y = MAP_H - next.h;

    int blocked_y = 0;
    for (int i = 0; i < obs_cnt; i++)
        if (rects_overlap(next, obs[i])) { blocked_y = 1; break; }

    if (!blocked_y) {
        for (int i = 0; i < dc; i++)
            if (dl[i].locked && !door_open[i] && rects_overlap(next, dl[i].rect))
            { blocked_y = 1; break; }
    }
    if (blocked_y) next.y = p->rect.y;

    p->rect = next;

    /* --- Pick up keys --- */
    for (int i = 0; i < kc; i++) {
        if (kl[i].visible && !kl[i].collected &&
            rects_overlap(p->rect, kl[i].rect)) {
            kl[i].collected  = 1;
            kl[i].visible    = 0;
            p->has_key[kl[i].id] = 1;
        }
    }

    /* --- Interact with doors (auto-unlock when touching with correct key) --- */
    for (int i = 0; i < dc; i++) {
        if (!door_open[i] && dl[i].locked &&
            rects_overlap(p->rect, dl[i].rect)) {
            int kid = dl[i].key_id;
            if (p->has_key[kid]) {
                door_open[i]  = 1;
                dl[i].locked  = 0;
                p->has_key[kid] = 0;
            }
        }
        /* Level transition through open door */
        if (door_open[i] && dl[i].to_level >= 0 &&
            rects_overlap(p->rect, dl[i].rect)) {
            g->level = dl[i].to_level;
            setup_level2(g);
        }
        /* Game complete */
        if (door_open[i] && dl[i].to_level == -2 &&
            rects_overlap(p->rect, dl[i].rect)) {
            /* Reset to guide for now as a simple "game complete" */
            g->state = STATE_GUIDE1;
            g->timer_on = 0;
        }
    }
}

/* -------------------------------------------------------
   input_guide – guide pages
   ------------------------------------------------------- */
void input_guide(Game *g, SDL_Event *e)
{
    if (e->type == SDL_KEYDOWN || e->type == SDL_MOUSEBUTTONDOWN) {
        if (g->state == STATE_GUIDE1) {
            g->state = STATE_GUIDE2;
        } else {
            /* Start the game */
            g->state       = STATE_PLAYING;
            g->timer_on    = 1;
            g->timer_start = SDL_GetTicks();
        }
    }
}

/* input_mode_menu – choose mono/multi */
void input_mode_menu(Game *g, SDL_Event *e)
{
    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_1) {
            g->mode  = MODE_MONO;
            g->state = STATE_PLAYING;
        } else if (e->key.keysym.sym == SDLK_2) {
            g->mode  = MODE_MULTI;
            g->state = STATE_PLAYING;
        } else if (e->key.keysym.sym == SDLK_ESCAPE) {
            g->state = STATE_PLAYING;
        }
    }
}

/* input_playing – SHIFT to mode menu, ESC to quit */
void input_playing(Game *g, SDL_Event *e)
{
    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_LSHIFT ||
            e->key.keysym.sym == SDLK_RSHIFT) {
            g->state = STATE_MODE_MENU;
        }
    }
}
// TACHE BLANCHE DU LOT2 
/*
 * tache_blanche.c
 * ─────────────────────────────────────────────────────────────
 * Sous-menu "Meilleurs Scores" – Lot 2 / tâche blanche
 *
 * Compile avec le reste du projet :
 *   gcc ... tache_blanche.c ... -lSDL2 -lSDL2_ttf -lSDL2_image \
 *           -lSDL2_mixer -lm -I/usr/include/SDL2
 *
 * Ce fichier ne touche à AUCUN autre fichier du projet.
 * ─────────────────────────────────────────────────────────────
 */

#include "tache_blanche.h"

/* ════════════════════════════════════════════════════════════
   COULEURS LOCALES (palette cohérente avec le jeu)
   ════════════════════════════════════════════════════════════ */
static const SDL_Color TB_NOIR       = {  10,  10,  20, 220 };
static const SDL_Color TB_OR         = { 255, 215,   0, 255 };
static const SDL_Color TB_BLANC      = { 230, 230, 230, 255 };
static const SDL_Color TB_GRIS       = { 140, 140, 140, 255 };
static const SDL_Color TB_ROUGE      = { 210,  55,  55, 255 };
static const SDL_Color TB_BLEU       = {  55, 120, 210, 255 };
static const SDL_Color TB_VERT       = {  55, 190,  90, 255 };
static const SDL_Color TB_ARGENT     = { 192, 192, 192, 255 };
static const SDL_Color TB_BRONZE     = { 205, 127,  50, 255 };

/* ════════════════════════════════════════════════════════════
   UTILITAIRES INTERNES
   ════════════════════════════════════════════════════════════ */

/* Dessine un rectangle plein semi-transparent */
static void tb_fillRect(SDL_Renderer *r, SDL_Rect rect, SDL_Color c)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r, &rect);
}

/* Dessine le contour d'un rectangle */
static void tb_drawRect(SDL_Renderer *r, SDL_Rect rect, SDL_Color c)
{
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderDrawRect(r, &rect);
}

/* Dessine une ligne horizontale décorative */
static void tb_hline(SDL_Renderer *r, int x1, int x2, int y, SDL_Color c)
{
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderDrawLine(r, x1, y, x2, y);
}

/* Affiche du texte avec ctx->font (la police du jeu) */
static void tb_texte(SDL_Renderer *r, TTF_Font *font,
                     const char *txt, SDL_Color c,
                     int x, int y, int centrerH)
{
    if (!font || !txt || !txt[0]) return;

    SDL_Surface *s = TTF_RenderUTF8_Blended(font, txt, c);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    if (!t) { SDL_FreeSurface(s); return; }

    SDL_Rect dst;
    dst.w = s->w;
    dst.h = s->h;
    dst.x = centrerH ? (WINDOW_WIDTH - dst.w) / 2 : x;
    dst.y = y;

    SDL_RenderCopy(r, t, NULL, &dst);
    SDL_DestroyTexture(t);
    SDL_FreeSurface(s);
}

/* Fond dégradé vertical sombre */
static void tb_fond(SDL_Renderer *r)
{
    for (int y = 0; y < WINDOW_HEIGHT; y++) {
        Uint8 rv = (Uint8)(10 + 20 * y / WINDOW_HEIGHT);
        Uint8 gv = (Uint8)(10 + 10 * y / WINDOW_HEIGHT);
        Uint8 bv = (Uint8)(20 + 40 * y / WINDOW_HEIGHT);
        SDL_SetRenderDrawColor(r, rv, gv, bv, 255);
        SDL_RenderDrawLine(r, 0, y, WINDOW_WIDTH, y);
    }
}

/* ════════════════════════════════════════════════════════════
   GESTION DU FICHIER score.txt
   Format d'une ligne : NOM SCORE JOUEUR\n
   ════════════════════════════════════════════════════════════ */

void tb_charger(ClassementTB *cl)
{
    cl->nb = 0;

    FILE *f = fopen(SCORE_FILE, "r");
    if (!f) {
        /* Fichier inexistant : on le crée vide */
        f = fopen(SCORE_FILE, "w");
        if (f) fclose(f);
        return;
    }

    while (cl->nb < MAX_SCORES_TB) {
        EntreeTB *e = &cl->entrees[cl->nb];
        if (fscanf(f, "%49s %d %d",
                   e->nom, &e->score, &e->joueur) != 3)
            break;
        cl->nb++;
    }

    fclose(f);
    tb_trier(cl);
}

void tb_sauvegarder(const ClassementTB *cl)
{
    FILE *f = fopen(SCORE_FILE, "w");
    if (!f) {
        fprintf(stderr,
            "[TACHE_BLANCHE] Impossible d'ecrire dans %s\n", SCORE_FILE);
        return;
    }

    for (int i = 0; i < cl->nb; i++)
        fprintf(f, "%s %d %d\n",
                cl->entrees[i].nom,
                cl->entrees[i].score,
                cl->entrees[i].joueur);

    fclose(f);
    printf("[TACHE_BLANCHE] %d score(s) sauvegardes dans %s\n",
           cl->nb, SCORE_FILE);
}

/* Tri par insertion décroissant */
void tb_trier(ClassementTB *cl)
{
    for (int i = 1; i < cl->nb; i++) {
        EntreeTB cle = cl->entrees[i];
        int j = i - 1;
        while (j >= 0 && cl->entrees[j].score < cle.score) {
            cl->entrees[j + 1] = cl->entrees[j];
            j--;
        }
        cl->entrees[j + 1] = cle;
    }
}

int tb_inserer(ClassementTB *cl, const char *nom, int score, int joueur)
{
    if (!nom || nom[0] == '\0') return 0;

    /* Si classement plein et score trop faible → pas de place */
    if (cl->nb >= MAX_SCORES_TB &&
        score <= cl->entrees[MAX_SCORES_TB - 1].score)
        return 0;

    int idx = (cl->nb < MAX_SCORES_TB) ? cl->nb : MAX_SCORES_TB - 1;
    strncpy(cl->entrees[idx].nom, nom, MAX_NOM_TB - 1);
    cl->entrees[idx].nom[MAX_NOM_TB - 1] = '\0';
    cl->entrees[idx].score  = score;
    cl->entrees[idx].joueur = joueur;

    if (cl->nb < MAX_SCORES_TB) cl->nb++;

    tb_trier(cl);
    return 1;
}

/* ════════════════════════════════════════════════════════════
   SOUS-MENU 1 : SAISIE DU NOM
   ════════════════════════════════════════════════════════════ */

int tb_saisir_nom(GameContext *ctx, char *nomSortie)
{
    char  buffer[MAX_NOM_TB] = "";
    int   longueur  = 0;
    int   valide    = 0;
    int   running   = 1;

    SDL_StartTextInput();

    Uint32 dernierCligno  = SDL_GetTicks();
    int    curseurVisible = 1;

    SDL_Renderer *r    = ctx->renderer;
    TTF_Font     *font = ctx->font;   /* police du jeu */

    SDL_Event ev;

    while (running) {

        /* ── Événements ── */
        while (SDL_PollEvent(&ev)) {

            if (ev.type == SDL_QUIT) {
                running = 0; valide = 0;
            }
            else if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym) {

                    case SDLK_ESCAPE:
                        running = 0; valide = 0;
                        break;

                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        if (longueur > 0) { running = 0; valide = 1; }
                        break;

                    case SDLK_BACKSPACE:
                        if (longueur > 0)
                            buffer[--longueur] = '\0';
                        break;

                    default: break;
                }
            }
            else if (ev.type == SDL_TEXTINPUT) {
                int ajout = (int)strlen(ev.text.text);
                if (longueur + ajout < MAX_NOM_TB - 1) {
                    strncat(buffer, ev.text.text,
                            (size_t)(MAX_NOM_TB - longueur - 1));
                    longueur += ajout;
                }
            }
            else if (ev.type == SDL_MOUSEBUTTONDOWN &&
                     ev.button.button == SDL_BUTTON_LEFT) {
                /* Clic sur bouton VALIDER */
                SDL_Rect btnOk = { (WINDOW_WIDTH - 200) / 2,
                                   380, 200, 50 };
                int mx = ev.button.x, my = ev.button.y;
                if (mx >= btnOk.x && mx <= btnOk.x + btnOk.w &&
                    my >= btnOk.y && my <= btnOk.y + btnOk.h &&
                    longueur > 0) {
                    running = 0; valide = 1;
                }
            }
        }

        /* ── Curseur clignotant (500 ms) ── */
        if (SDL_GetTicks() - dernierCligno >= 500) {
            curseurVisible  = !curseurVisible;
            dernierCligno   = SDL_GetTicks();
        }

        /* ── Rendu ── */
        tb_fond(r);

        /* Titre */
        tb_texte(r, font, "ENTREZ VOTRE NOM", TB_OR, 0, 80, 1);

        /* Ligne décorative */
        tb_hline(r, 150, WINDOW_WIDTH - 150, 120, TB_OR);

        /* Sous-titre */
        tb_texte(r, font,
                 "Votre score sera sauvegarde dans " SCORE_FILE,
                 TB_GRIS, 0, 140, 1);

        /* Zone de saisie */
        SDL_Rect boite = { (WINDOW_WIDTH - 400) / 2, 220, 400, 55 };
        tb_fillRect(r, boite, (SDL_Color){30, 30, 60, 200});
        tb_drawRect(r, boite, TB_BLEU);

        /* Texte + curseur */
        char affiche[MAX_NOM_TB + 2];
        snprintf(affiche, sizeof(affiche), "%s%s",
                 buffer, curseurVisible ? "|" : " ");
        tb_texte(r, font, affiche, TB_BLEU,
                 boite.x + 14, boite.y + 14, 0);

        /* Aide */
        tb_texte(r, font,
                 "Entree : valider   |   Echap : passer",
                 TB_GRIS, 0, 310, 1);

        /* Bouton VALIDER */
        int mx2, my2;
        SDL_GetMouseState(&mx2, &my2);
        SDL_Rect btnOk = { (WINDOW_WIDTH - 200) / 2, 380, 200, 50 };
        int survol = (mx2 >= btnOk.x && mx2 <= btnOk.x + btnOk.w &&
                      my2 >= btnOk.y && my2 <= btnOk.y + btnOk.h);
        SDL_Color cBtn = survol ? TB_VERT
                                : (SDL_Color){40, 150, 70, 255};
        tb_fillRect(r, btnOk, cBtn);
        tb_drawRect(r, btnOk, TB_BLANC);
        tb_texte(r, font, "VALIDER",
                 TB_BLANC, btnOk.x + 60, btnOk.y + 13, 0);

        SDL_RenderPresent(r);
        SDL_Delay(16);
    }

    SDL_StopTextInput();

    if (valide) {
        strncpy(nomSortie, buffer, MAX_NOM_TB - 1);
        nomSortie[MAX_NOM_TB - 1] = '\0';
    }
    return valide;
}

/* ════════════════════════════════════════════════════════════
   SOUS-MENU 2 : AFFICHAGE DU CLASSEMENT
   ════════════════════════════════════════════════════════════ */

void tb_afficher_classement(GameContext *ctx, const ClassementTB *cl)
{
    SDL_Renderer *r    = ctx->renderer;
    TTF_Font     *font = ctx->font;

    int      running = 1;
    SDL_Event ev;

    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT      ||
                ev.type == SDL_KEYDOWN   ||
                ev.type == SDL_MOUSEBUTTONDOWN)
                running = 0;
        }

        /* ── Fond ── */
        tb_fond(r);

        /* ── Titre ── */
        tb_texte(r, font, "MEILLEURS SCORES", TB_OR, 0, 28, 1);
        tb_hline(r, 80, WINDOW_WIDTH - 80, 68, TB_OR);

        /* ── En-têtes ── */
        tb_texte(r, font, "RANG",    TB_BLEU,  70,  80, 0);
        tb_texte(r, font, "NOM",     TB_BLEU, 170,  80, 0);
        tb_texte(r, font, "JOUEUR",  TB_BLEU, 560,  80, 0);
        tb_texte(r, font, "SCORE",   TB_BLEU, 720,  80, 0);
        tb_hline(r, 60, WINDOW_WIDTH - 60, 106, TB_GRIS);

        /* ── Lignes du classement ── */
        if (cl->nb == 0) {
            tb_texte(r, font,
                     "Aucun score enregistre.", TB_GRIS, 0, 200, 1);
        }

        for (int i = 0; i < cl->nb; i++) {
            int y = 115 + i * 44;

            /* Fond alterné */
            if (i % 2 == 0) {
                SDL_Rect fond = { 55, y - 4, WINDOW_WIDTH - 110, 40 };
                tb_fillRect(r, fond, (SDL_Color){255, 255, 255, 12});
            }

            /* Couleur selon rang */
            SDL_Color cLigne;
            if      (i == 0) cLigne = TB_OR;
            else if (i == 1) cLigne = TB_ARGENT;
            else if (i == 2) cLigne = TB_BRONZE;
            else             cLigne = TB_BLANC;

            /* Rang */
            char sRang[8];
            snprintf(sRang, sizeof(sRang), "%2d.", i + 1);
            tb_texte(r, font, sRang, cLigne, 72, y, 0);

            /* Nom */
            tb_texte(r, font, cl->entrees[i].nom, cLigne, 170, y, 0);

            /* Joueur (P1 / P2) */
            char sJoueur[8];
            snprintf(sJoueur, sizeof(sJoueur), "P%d",
                     cl->entrees[i].joueur);
            SDL_Color cJ = (cl->entrees[i].joueur == 1) ? TB_ROUGE : TB_BLEU;
            tb_texte(r, font, sJoueur, cJ, 580, y, 0);

            /* Score */
            char sScore[16];
            snprintf(sScore, sizeof(sScore), "%d",
                     cl->entrees[i].score);
            tb_texte(r, font, sScore, cLigne, 720, y, 0);
        }

        /* ── Pied de page ── */
        tb_hline(r, 60, WINDOW_WIDTH - 60,
                 WINDOW_HEIGHT - 55, TB_GRIS);
        tb_texte(r, font,
                 "Appuyez sur une touche pour continuer...",
                 TB_GRIS, 0, WINDOW_HEIGHT - 42, 1);

        SDL_RenderPresent(r);
        SDL_Delay(16);
    }
}

/* ════════════════════════════════════════════════════════════
   FONCTION PRINCIPALE : afficherSousMenuScores
   Appelez-la quand ctx->currentState == STATE_GAME_OVER
   ════════════════════════════════════════════════════════════ */

void afficherSousMenuScores(GameContext *ctx)
{
    /* ── Étape 0 : chargement du classement existant ── */
    ClassementTB cl;
    tb_charger(&cl);

    /* ── Étape 1 : saisie du nom de Player 1 ── */
    char nomP1[MAX_NOM_TB] = "";
    printf("[TACHE_BLANCHE] Score Player 1 : %d\n", ctx->player1.score);

    int valideP1 = tb_saisir_nom(ctx, nomP1);
    if (valideP1 && ctx->player1.score > 0) {
        int insere = tb_inserer(&cl, nomP1,
                                ctx->player1.score, 1);
        if (insere)
            printf("[TACHE_BLANCHE] Player 1 (%s) insere avec %d pts.\n",
                   nomP1, ctx->player1.score);
        else
            printf("[TACHE_BLANCHE] Score de Player 1 trop bas "
                   "pour le top %d.\n", MAX_SCORES_TB);
    }

    /* ── Étape 2 : saisie du nom de Player 2 ── */
    /*    (uniquement si Player 2 a participé)   */
    if (ctx->player2.score > 0) {
        char nomP2[MAX_NOM_TB] = "";
        printf("[TACHE_BLANCHE] Score Player 2 : %d\n",
               ctx->player2.score);

        int valideP2 = tb_saisir_nom(ctx, nomP2);
        if (valideP2) {
            int insere = tb_inserer(&cl, nomP2,
                                    ctx->player2.score, 2);
            if (insere)
                printf("[TACHE_BLANCHE] Player 2 (%s) insere "
                       "avec %d pts.\n",
                       nomP2, ctx->player2.score);
            else
                printf("[TACHE_BLANCHE] Score de Player 2 trop bas "
                       "pour le top %d.\n", MAX_SCORES_TB);
        }
    }

    /* ── Étape 3 : sauvegarde dans score.txt ── */
    tb_sauvegarder(&cl);

    /* ── Étape 4 : affichage du classement ── */
    tb_afficher_classement(ctx, &cl);
}


/* -------------------------------------------------------
   render_guide – tutorial pages
   ------------------------------------------------------- */
void render_guide(Game *g)
{
    SDL_SetRenderDrawColor(g->renderer, 15, 15, 25, 255);
    SDL_RenderClear(g->renderer);

    SDL_Color white  = {255, 255, 255, 255};
    SDL_Color yellow = {255, 215,   0, 255};
    SDL_Color cyan   = { 80, 220, 255, 255};
    SDL_Color grey   = {180, 180, 180, 255};

    /* Decorative frame */
    SDL_SetRenderDrawColor(g->renderer, 60, 60, 90, 255);
    SDL_Rect frame = {60, 40, WINDOW_W - 120, WINDOW_H - 80};
    SDL_RenderDrawRect(g->renderer, &frame);
    frame.x++; frame.y++; frame.w -= 2; frame.h -= 2;
    SDL_RenderDrawRect(g->renderer, &frame);

    if (g->state == STATE_GUIDE1) {
        draw_text(g, g->font_lg, "SHERLOCK HOLMES - CONTROLS", 160, 70, yellow);

        draw_text(g, g->font_lg, "MONO PLAYER MODE", 130, 140, cyan);
        draw_text(g, g->font_sm, "Arrow Keys  ->  Move your character", 150, 180, white);
        draw_text(g, g->font_sm, "Walk over a key to collect it automatically", 150, 210, white);
        draw_text(g, g->font_sm, "Walk onto a door holding the right key to unlock it", 150, 240, white);

        draw_text(g, g->font_lg, "MULTI PLAYER MODE  (press SHIFT any time)", 130, 300, cyan);
        draw_text(g, g->font_sm, "Player 1  ->  W A S D  keys", 150, 340, white);
        draw_text(g, g->font_sm, "Player 2  ->  Arrow keys", 150, 370, white);
        draw_text(g, g->font_sm, "Screen splits in two - each player sees their own view", 150, 400, white);

        draw_text(g, g->font_sm, "SHIFT  ->  Open mode selection menu at any time", 150, 460, grey);
        draw_text(g, g->font_sm, "ESC    ->  Quit the game", 150, 490, grey);

        draw_text(g, g->font_sm, "Press any key to continue  ->", 460, 650, yellow);
    } else {
        draw_text(g, g->font_lg, "SHERLOCK HOLMES - HOW TO PLAY", 160, 70, yellow);

        draw_text(g, g->font_lg, "OBJECTIVE", 130, 140, cyan);
        draw_text(g, g->font_sm, "Find hidden keys scattered across each level.", 150, 185, white);
        draw_text(g, g->font_sm, "Use them to unlock doors and progress to the next level.", 150, 215, white);

        draw_text(g, g->font_lg, "LEVEL 1  -  The House", 130, 280, cyan);
        draw_text(g, g->font_sm, "A heavy box is stacked on crates in the right dungeon area.", 150, 320, white);
        draw_text(g, g->font_sm, "Approach the box - it will fall and break, revealing KEY 1.", 150, 350, white);
        draw_text(g, g->font_sm, "KEY 1 unlocks the balcony door (top wall of the house).", 150, 380, white);
        draw_text(g, g->font_sm, "On the balcony, KEY 2 is resting on the ground.", 150, 410, white);
        draw_text(g, g->font_sm, "KEY 2 opens the exit door at the bottom - leads to Level 2!", 150, 440, white);

        draw_text(g, g->font_lg, "LEVEL 2  -  The Hotel (Upper Floors)", 130, 510, cyan);
        draw_text(g, g->font_sm, "Search the upper floors for the hidden key.", 150, 550, white);
        draw_text(g, g->font_sm, "Use it to solve the mystery and complete the case!", 150, 580, white);

        draw_text(g, g->font_sm, "Press any key to start  ->", 480, 650, yellow);
    }
}

/* -------------------------------------------------------
   render_mode_menu – mono / multi selection overlay
   ------------------------------------------------------- */
void render_mode_menu(Game *g)
{
    /* Dim the background */
    SDL_Color dim = {0, 0, 0, 170};
    SDL_Rect  full = {0, 0, WINDOW_W, WINDOW_H};
    filled_rect(g->renderer, full, dim);

    /* Menu box */
    SDL_Rect box = {480, 260, 440, 240};
    SDL_Color boxbg = {20, 20, 40, 240};
    filled_rect(g->renderer, box, boxbg);
    SDL_SetRenderDrawColor(g->renderer, 255, 215, 0, 255);
    SDL_RenderDrawRect(g->renderer, &box);

    SDL_Color yellow = {255, 215,  0, 255};
    SDL_Color white  = {255, 255, 255, 255};
    SDL_Color active = { 80, 220, 255, 255};

    draw_text(g, g->font_lg, "SELECT MODE", 555, 280, yellow);

    SDL_Color c1 = (g->mode == MODE_MONO) ? active : white;
    SDL_Color c2 = (g->mode == MODE_MULTI)? active : white;

    draw_text(g, g->font_lg, "[1]  Single Player  (Mono)", 500, 340, c1);
    draw_text(g, g->font_lg, "[2]  Two Players    (Split Screen)", 500, 390, c2);
    draw_text(g, g->font_sm, "ESC - cancel", 580, 460, white);
}

/* -------------------------------------------------------
   scrolling dans les quatre sens
   -------------------------------------------------------
   La caméra se déplace indépendamment du joueur.
   cam->x / cam->y = coin supérieur gauche de la vue
                     en coordonnées MAP (avant zoom).
   La portion visible est (vp_w/ZOOM) x (vp_h/ZOOM) pixels map.
   ------------------------------------------------------- */

void camera_init(Camera *cam, int vp_w, int vp_h)
{
    cam->x = 0;
    cam->y = 0;
    cam->w = vp_w;
    cam->h = vp_h;
}

/* scroll_camera_left – déplace la caméra vers la gauche */
void scroll_camera_left(Camera *cam)
{
    cam->x -= CAM_SCROLL_SPEED;
    if (cam->x < 0) cam->x = 0;
}

/* scroll_camera_right – déplace la caméra vers la droite */
void scroll_camera_right(Camera *cam)
{
    int view_w    = (int)(cam->w / ZOOM_FACTOR);
    int max_cam_x = MAP_W - view_w;
    cam->x += CAM_SCROLL_SPEED;
    if (cam->x > max_cam_x) cam->x = max_cam_x;
}

/* scroll_camera_up – déplace la caméra vers le haut */
void scroll_camera_up(Camera *cam)
{
    cam->y -= CAM_SCROLL_SPEED;
    if (cam->y < 0) cam->y = 0;
}

/* scroll_camera_down – déplace la caméra vers le bas */
void scroll_camera_down(Camera *cam)
{
    int view_h    = (int)(cam->h / ZOOM_FACTOR);
    int max_cam_y = MAP_H - view_h;
    cam->y += CAM_SCROLL_SPEED;
    if (cam->y > max_cam_y) cam->y = max_cam_y;
}

/* afficher_scrolling – appelle les 4 fonctions selon les touches pressées
   left/right/up/down : 1 si la touche est enfoncée, 0 sinon            */
void afficher_scrolling(Camera *cam, int left, int right, int up, int down)
{
    if (left)  scroll_camera_left(cam);
    if (right) scroll_camera_right(cam);
    if (up)    scroll_camera_up(cam);
    if (down)  scroll_camera_down(cam);
}

/* -------------------------------------------------------
   partage d'écran
   render_viewport – affiche la vue d'un joueur dans son viewport
   (mono: viewport plein écran | multi: demi-écran gauche/droit)
   ------------------------------------------------------- */
static void render_viewport(Game *g, SDL_Rect vp, Player *p,
                            int cam_scroll)
{
    SDL_RenderSetViewport(g->renderer, &vp);

    /* --- scrolling dans les quatre sens via Camera --- */
    Camera *cam = (p == &g->p1) ? &g->cam1 : &g->cam2;

    /* Re-initialise si le viewport change (mono ↔ split) */
    if (cam->w != vp.w || cam->h != vp.h)
        camera_init(cam, vp.w, vp.h);

    if (cam_scroll) {
        /* Lire les touches de défilement :
           Mono   → touches fléchées
           Multi  → P1: WASD   P2: touches fléchées                */
        const Uint8 *ks = SDL_GetKeyboardState(NULL);
        int left, right, up, down;

        if (p == &g->p2) {
            /* P2 scrolls with arrow keys in split mode */
            left  = ks[SDL_SCANCODE_LEFT];
            right = ks[SDL_SCANCODE_RIGHT];
            up    = ks[SDL_SCANCODE_UP];
            down  = ks[SDL_SCANCODE_DOWN];
        } else {
            /* P1 (and mono) scrolls with WASD */
            left  = ks[SDL_SCANCODE_A];
            right = ks[SDL_SCANCODE_D];
            up    = ks[SDL_SCANCODE_W];
            down  = ks[SDL_SCANCODE_S];
        }

        afficher_scrolling(cam, left, right, up, down);
    } else {
        cam->x = 0;
        cam->y = 0;
    }

    /* cam->x / cam->y sont en coordonnées MAP (avant zoom).
       src_rect indique quelle portion du background on affiche. */
    int src_w = (int)(vp.w / ZOOM_FACTOR);   /* portion visible en pixels map */
    int src_h = (int)(vp.h / ZOOM_FACTOR);

    /* Clamp camera to map bounds */
    if (cam->x + src_w > MAP_W) cam->x = MAP_W - src_w;
    if (cam->y + src_h > MAP_H) cam->y = MAP_H - src_h;
    if (cam->x < 0) cam->x = 0;
    if (cam->y < 0) cam->y = 0;

    /* --- initialiser et afficher background --- */
    SDL_Texture *tex_map = (g->level == LEVEL_1) ? g->tex_map1 : g->tex_map2;
    SDL_Rect src_rect = { cam->x, cam->y, src_w, src_h };
    SDL_Rect dst_rect = { 0, 0, vp.w, vp.h };
    SDL_RenderCopy(g->renderer, tex_map, &src_rect, &dst_rect);

/* Helper macro: convert a map-space rect to viewport-space screen rect */
#define MAP_TO_SCREEN(r) (SDL_Rect){ \
    (int)(((r).x - cam->x) * ZOOM_FACTOR), \
    (int)(((r).y - cam->y) * ZOOM_FACTOR), \
    (int)((r).w * ZOOM_FACTOR), \
    (int)((r).h * ZOOM_FACTOR) }

    /* --- Draw doors (plateformes fixes) --- */
    Door *dl     = (g->level == LEVEL_1) ? g->doors1    : g->doors2;
    int   dc     = (g->level == LEVEL_1) ? g->doors1_cnt: g->doors2_cnt;
    int  *door_open = (g->level == LEVEL_1) ? g->door1_open : g->door2_open;

    for (int i = 0; i < dc; i++) {
        SDL_Rect dr = MAP_TO_SCREEN(dl[i].rect);
        if (door_open[i]) {
            SDL_Color oc = {20, 180, 40, 100};
            filled_rect(g->renderer, dr, oc);
        } else {
            SDL_Color lc = {180, 60, 20, 210};
            filled_rect(g->renderer, dr, lc);
        }
    }

    /* --- Draw keys --- */
    Key *kl = (g->level == LEVEL_1) ? g->keys1 : g->keys2;
    int  kc = (g->level == LEVEL_1) ? g->keys1_cnt : g->keys2_cnt;

    for (int i = 0; i < kc; i++) {
        if (!kl[i].visible || kl[i].collected) continue;
        SDL_Rect kr = MAP_TO_SCREEN(kl[i].rect);
        SDL_Color kc_col = {255, 215, 0, 255};
        filled_rect(g->renderer, kr, kc_col);
        /* Key shine */
        SDL_Color shine = {255, 255, 180, 180};
        SDL_Rect shine_r = { kr.x + (int)(4 * ZOOM_FACTOR),
                             kr.y + (int)(3 * ZOOM_FACTOR),
                             (int)(8 * ZOOM_FACTOR),
                             (int)(6 * ZOOM_FACTOR) };
        filled_rect(g->renderer, shine_r, shine);
    }

    /* --- Draw falling box – plateforme destructible (level 1 only) --- */
    if (g->level == LEVEL_1) {
        FallingBox *fb = &g->fbox;
        SDL_Rect box_dst = MAP_TO_SCREEN(fb->rect);
        if (fb->state == BOX_BROKEN)
            SDL_RenderCopy(g->renderer, g->tex_broken_box, NULL, &box_dst);
        else
            SDL_RenderCopy(g->renderer, g->tex_box,        NULL, &box_dst);
    }

    /* --- Draw player --- */
    SDL_Rect pr = MAP_TO_SCREEN(p->rect);
    SDL_Color body = (p == &g->p1) ?
        (SDL_Color){30, 180, 255, 210} : (SDL_Color){255, 100, 40, 210};
    filled_rect(g->renderer, pr, body);
    /* Head highlight */
    SDL_Rect head = { pr.x + (int)(4 * ZOOM_FACTOR),
                      pr.y + (int)(2 * ZOOM_FACTOR),
                      (int)((p->rect.w - 8) * ZOOM_FACTOR),
                      (int)(9 * ZOOM_FACTOR) };
    SDL_Color hcol = {220, 220, 220, 180};
    filled_rect(g->renderer, head, hcol);

#undef MAP_TO_SCREEN

    SDL_RenderSetViewport(g->renderer, NULL);
}

/* -------------------------------------------------------
   render_hud – timer and key inventory (top-right)
   ------------------------------------------------------- */
static void render_hud(Game *g)
{
    char tbuf[TIMER_BUF];
    update_timer_str(g, tbuf);

    /* Timer box */
    SDL_Rect tbox = {WINDOW_W - 175, 8, 165, 38};
    SDL_Color tboxbg = {0, 0, 0, 160};
    filled_rect(g->renderer, tbox, tboxbg);
    SDL_SetRenderDrawColor(g->renderer, 255, 215, 0, 200);
    SDL_RenderDrawRect(g->renderer, &tbox);

    SDL_Color tcol = {255, 215, 0, 255};
    draw_text(g, g->font_sm, tbuf, WINDOW_W - 162, 15, tcol);

    /* Key indicator */
    Key *kl = (g->level == LEVEL_1) ? g->keys1 : g->keys2;
    int  kc = (g->level == LEVEL_1) ? g->keys1_cnt : g->keys2_cnt;

    int px = WINDOW_W - 175;
    int py = 54;
    SDL_Color white = {255, 255, 255, 200};
    draw_text(g, g->font_sm, "Keys:", px, py, white);

    for (int i = 0; i < kc; i++) {
        SDL_Rect kr = {px + 55 + i * 26, py + 2, 18, 18};
        SDL_Color kc_col = kl[i].collected ?
            (SDL_Color){255, 215, 0, 255} : (SDL_Color){60, 60, 60, 200};
        filled_rect(g->renderer, kr, kc_col);
        SDL_SetRenderDrawColor(g->renderer, 200, 180, 0, 255);
        SDL_RenderDrawRect(g->renderer, &kr);
    }

    /* Level indicator */
    SDL_Color lcol = {180, 180, 255, 230};
    char lbuf[32];
    snprintf(lbuf, sizeof(lbuf), "Level %d", g->level + 1);
    draw_text(g, g->font_sm, lbuf, px, py + 26, lcol);
}

/* -------------------------------------------------------
   partage d'écran – render_game
   Appelle render_viewport avec le mode (mono / multi).
   En mono  : un seul viewport plein écran, scrolling activé.
   En multi : deux demi-écrans côte à côte (partage d'écran),
              chaque joueur a sa propre caméra 4-directions.
   ------------------------------------------------------- */
void render_game(Game *g)
{
    SDL_SetRenderDrawColor(g->renderer, 8, 8, 12, 255);
    SDL_RenderClear(g->renderer);

    if (g->mode == MODE_MONO) {
        /* Mode mono : viewport plein écran, scrolling dans les 4 sens */
        SDL_Rect vp = {0, 0, WINDOW_W, WINDOW_H};
        render_viewport(g, vp, &g->p1, 1);   /* 1 = cam_scroll activé */
    } else {
        /* partage d'écran (split screen) : chaque joueur = demi-écran */
        int hw = WINDOW_W / 2;
        SDL_Rect vp1 = {0,  0, hw, WINDOW_H};
        SDL_Rect vp2 = {hw, 0, hw, WINDOW_H};
        render_viewport(g, vp1, &g->p1, 1);
        render_viewport(g, vp2, &g->p2, 1);

        /* Ligne de séparation centrale */
        SDL_RenderSetViewport(g->renderer, NULL);
        SDL_SetRenderDrawColor(g->renderer, 255, 215, 0, 255);
        SDL_RenderDrawLine(g->renderer, hw,     0, hw,     WINDOW_H);
        SDL_RenderDrawLine(g->renderer, hw - 1, 0, hw - 1, WINDOW_H);

        /* Labels joueurs */
        SDL_Color p1col = {30, 180, 255, 255};
        SDL_Color p2col = {255, 100, 40, 255};
        draw_text(g, g->font_sm, "P1", 6,      6, p1col);
        draw_text(g, g->font_sm, "P2", hw + 6, 6, p2col);
    }

    render_hud(g);
}
