#include "map.h"

int map_rects_overlap(SDL_Rect a, SDL_Rect b)
{
    return (a.x < b.x + b.w && a.x + a.w > b.x &&
            a.y < b.y + b.h && a.y + a.h > b.y);
}

void map_init(MapData *m, SDL_Renderer *renderer)
{
    // Updated paths to include the /map/ subdirectory
    m->door1_open[0]  = 0; // ensure initialized
    m->boxFallSound   = NULL;

    m->tex_map1       = IMG_LoadTexture(renderer, "assets/map/background/background1.png");
    m->tex_map2       = IMG_LoadTexture(renderer, "assets/map/background/background2.png");
    m->tex_box        = IMG_LoadTexture(renderer, "assets/map/box.png");
    m->tex_broken_box = IMG_LoadTexture(renderer, "assets/map/broken box.png");
    
    m->boxFallSound = Mix_LoadWAV("assets/sounds/box_fall.mp3");
    if (!m->boxFallSound) {
        printf("Erreur sound box_fall.mp3: %s\n", Mix_GetError());
    } else {
        printf("Box fall sound loaded successfully.\n");
    }

    m->level          = LEVEL_1;
    setup_level1(m);
}

void map_cleanup(MapData *m)
{
    if (m->tex_map1)       SDL_DestroyTexture(m->tex_map1);
    if (m->tex_map2)       SDL_DestroyTexture(m->tex_map2);
    if (m->tex_box)        SDL_DestroyTexture(m->tex_box);
    if (m->tex_broken_box) SDL_DestroyTexture(m->tex_broken_box);
    if (m->boxFallSound)   Mix_FreeChunk(m->boxFallSound);
}

#define ADD_OBS1(X,Y,W,H) m->obs1[m->obs1_cnt++] = (SDL_Rect){X,Y,W,H}
#define ADD_OBS2(X,Y,W,H) m->obs2[m->obs2_cnt++] = (SDL_Rect){X,Y,W,H}

void setup_level1(MapData *m)
{
    m->obs1_cnt   = 0;
    m->doors1_cnt = 0;
    memset(m->door1_open, 0, sizeof(m->door1_open));

    /* ── OUTER MAP BOUNDARY ──────────────────────────────────────────── */
    ADD_OBS1(   0,   0,1400,   9);  /* border top    */
    ADD_OBS1(   0, 732,1400,   9);  /* border bottom */
    ADD_OBS1(   0,   0,   9, 742);  /* border left   */
    ADD_OBS1(1390,   0,   9, 742);  /* border right  */

    /* ── HOUSE PERIMETER ─────────────────────────────────────────────── */
    /* Top wall — gap at x 556-601 for balcony door */
    ADD_OBS1( 374, 213, 500,  70);  /* top left of gap  */
    /* Left wall — gap at y 364-439 for outdoor entry */
    ADD_OBS1( 350, 200,  30, 180);  /* left wall top    */
    ADD_OBS1( 350, 470,  30, 261);  /* left wall bottom */
    /* Right wall — solid */
    ADD_OBS1( 894, 213,  11, 300);
    ADD_OBS1( 380, 660,  560, 30);
    ADD_OBS1( 990, 660,  50, 30);
    ADD_OBS1( 380, 360,  90, 10);
    ADD_OBS1( 480, 430,  100, 10);
    ADD_OBS1( 640, 500,  30, 10);
    ADD_OBS1( 640, 620,  60, 30);
    ADD_OBS1( 381, 550,  30, 70);
    ADD_OBS1( 381, 590,  180, 30);
    ADD_OBS1( 1280, 390, 180, 110);
    ADD_OBS1( 1030, 390, 160, 110);
    ADD_OBS1( 1030, 200, 50, 200);
    ADD_OBS1( 1090, 350, 40, 10);
    ADD_OBS1( 1090, 300, 60, 10);
    ADD_OBS1( 1300, 280, 30, 10);
    ADD_OBS1( 1320, 540, 10, 100);
    ADD_OBS1( 1140, 520, 40, 10);
    ADD_OBS1( 1020, 620, 100, 30);
    ADD_OBS1( 1150, 660, 30, 30);
    ADD_OBS1( 670, 70, 10, 100);
    ADD_OBS1( 670, 70, 500, 10);
    ADD_OBS1( 1000, 70, 30, 100);
    ADD_OBS1( 880, 620, 30, 30);
    ADD_OBS1( 10, 500, 60, 30);
    ADD_OBS1( 210, 500, 140, 30);
    ADD_OBS1( 10, 200, 360, 30);

    /* ── DUNGEON OUTER WALLS ─────────────────────────────────────────── */
    ADD_OBS1(850, 213, 80,  30);  /* dungeon top    */
    ADD_OBS1(1000, 213, 400,  30);  /* dungeon top    */
    ADD_OBS1(1350, 213,  30, 487);  /* dungeon right  */
    ADD_OBS1(1080, 660,  70, 30);  /* left of gap  */
    ADD_OBS1(1204, 660, 211, 30);  /* right of gap */

    /* ── INDOOR VERTICAL DIVIDER (left / right room split) ──────────── */
    /* Gap at y 316-379 for passage */
    ADD_OBS1( 670, 213,  30, 300);  /* divider top    */

    /* ── INDOOR HORIZONTAL DIVIDER (right rooms upper/lower) ─────────── */
    /* Gap at x 771-845 for passage */
    ADD_OBS1( 700, 434, 180,  70);  /* horiz left  */

    /* Door 0: passage door — place in open floor area */
    m->doors1[0].rect     = (SDL_Rect){ 930, 210, 44, 18 };
    m->doors1[0].locked   = 1;
    m->doors1[0].key_id   = 0;
    m->doors1[0].to_level = -1;

    /* Door 1: exit to Level 2 — place in the dungeon gap you made */
    m->doors1[1].rect     = (SDL_Rect){ 950, 680, 54, 16 };
    m->doors1[1].locked   = 1;
    m->doors1[1].key_id   = 1;
    m->doors1[1].to_level = LEVEL_2;
    
    m->doors1_cnt = 2;
    


    /* ── KEYS ── */
    m->keys1_cnt = 2;

    m->keys1[0].rect      = (SDL_Rect){ 1200, 380, 24, 24 };
    m->keys1[0].id        = 0;
    m->keys1[0].visible   = 0;
    m->keys1[0].collected = 0;

    m->keys1[1].rect      = (SDL_Rect){672, 100, 24, 24 };
    m->keys1[1].id        = 1;
    m->keys1[1].visible   = 1;
    m->keys1[1].collected = 0;

    /* ── FALLING BOX ─────────────────────────────────────────────── */
    m->fbox.rect  = (SDL_Rect){ 1200, 230, 50, 50 };
    m->fbox.fy    = (float) 230;
    m->fbox.vel   = 0.0f;
    m->fbox.state = BOX_IDLE;
}

void setup_level2(MapData *m)
{
    m->obs2_cnt   = 0;
    m->doors2_cnt = 0;
    m->level      = LEVEL_2;
    memset(m->door2_open, 0, sizeof(m->door2_open));

    ADD_OBS2(   0,   0,1400,   9);  /* border top    */
    ADD_OBS2(   0, 700,1400,   9);  /* border bottom */
    ADD_OBS2(   33,   0,   9, 742);  /* border left   */
    ADD_OBS2(1360,   0,   9, 742);  /* border right  */
    ADD_OBS2(43,   490,   330, 10);
    ADD_OBS2(43,   560,   100, 10);
    ADD_OBS2(43,   660,   100, 10);
    ADD_OBS2(272,   560,   100, 10);
    ADD_OBS2(360,   540,   10, 50);
    ADD_OBS2(373,   570,   280, 10);
    ADD_OBS2(543,   670,   300, 30);
    ADD_OBS2(845,   540,   600, 10);
    ADD_OBS2(730,   570,   120, 10);
    ADD_OBS2(650,   420,   10, 150);
    ADD_OBS2(730,   480,   10, 100);
    ADD_OBS2(730,   480,   110, 10);
    ADD_OBS2(840,   485,   600, 10);
    ADD_OBS2(735,   390,   600, 10);
    ADD_OBS2(730,   255,   10, 140);
    ADD_OBS2(650,   255,   10, 80);
    ADD_OBS2(400,   330,  260, 10);
    ADD_OBS2(45,   320,  350, 10);
    ADD_OBS2(45,   440,  350, 10);
    ADD_OBS2(405,   430,  60, 10);
    ADD_OBS2(450,   440,   10, 140);
    ADD_OBS2(540,   417,   10, 100);
    ADD_OBS2(550,   420,   100, 10);
    ADD_OBS2(740,   255,   90, 10);
    ADD_OBS2(830,   230,   130, 10);
    ADD_OBS2(820,   230,   10, 30);
    ADD_OBS2(950,   240,   10, 90);
    ADD_OBS2(960,   320,   400, 10);
    ADD_OBS2(960,   80,   400, 10);
    ADD_OBS2(830,   130,   130, 10);
    ADD_OBS2(960,   90,   10, 30);
    ADD_OBS2(560,   96,   260, 10);
    ADD_OBS2(400,   137,   160, 10);
    ADD_OBS2(400,   225,   160, 10);
    ADD_OBS2(550,   258,   100, 10);
    ADD_OBS2(40,   82,   340, 10);
    ADD_OBS2(40,   250,   340, 10);


 /* Door 0: passage door — place in open floor area */
    m->doors2[0].rect     = (SDL_Rect){ 370, 590, 10, 60 };
    m->doors2[0].locked   = 1;
    m->doors2[0].key_id   = 0;
    m->doors2[0].to_level = -1;
    
    m->doors2_cnt = 1;
    


    /* ── KEYS ── */
    m->keys2_cnt = 1;

    m->keys2[0].rect      = (SDL_Rect){ 400, 640, 24, 24 };
    m->keys2[0].id        = 0;
    m->keys2[0].visible   = 1;
    m->keys2[0].collected = 0;

}

void update_falling_box(MapData *m, int p1x, int p1y, int p2x, int p2y)
{
    FallingBox *fb = &m->fbox;
    if (fb->state == BOX_BROKEN) return;

    if (fb->state == BOX_IDLE) {
        int dx1 = p1x - fb->rect.x;
        int dy1 = p1y - fb->rect.y;
        int dx2 = p2x - fb->rect.x;
        int dy2 = p2y - fb->rect.y;
        float d1 = sqrtf((float)(dx1*dx1 + dy1*dy1));
        float d2 = sqrtf((float)(dx2*dx2 + dy2*dy2));
        if (d1 < 130 || d2 < 130)
            fb->state = BOX_FALLING;
        return;
    }

    fb->vel += 0.5f;
    if (fb->vel > 4.5f) fb->vel = 4.5f;
    fb->fy += fb->vel;
    fb->rect.y = (int)fb->fy;

    if (fb->rect.y >= 358) {
        fb->rect.y = 358;
        fb->fy     = 358.0f;
        fb->state  = BOX_BROKEN;
        if (m->boxFallSound) {
            Mix_PlayChannel(-1, m->boxFallSound, 0);
        }
        /* reveal the key hidden inside the box (key 0 in level 1) */
        m->keys1[0].visible = 1;
    }
}
