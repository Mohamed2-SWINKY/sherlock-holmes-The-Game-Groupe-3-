

/**
 * @file minimap.c
 */
 
#include "minimap.h"
#include <stdlib.h>

void draw_filled_circle(SDL_Renderer *re, int cx, int cy, int r, SDL_Color col) {
    SDL_SetRenderDrawColor(re, col.r, col.g, col.b, col.a);
    for (int dy = -r; dy <= r; dy++)
        for (int dx = -r; dx <= r; dx++)
            if (dx*dx + dy*dy <= r*r)
                SDL_RenderDrawPoint(re, cx + dx, cy + dy);
}

/* --- 4.1 : INITIALISATION ET AFFICHAGE --- */
void init_minimap(MiniMap *m, SDL_Renderer *re, int screenH) {
    m->img_map1  = IMG_LoadTexture(re, "assets/minimap/map_level1.png");
    m->img_map2  = IMG_LoadTexture(re, "assets/minimap/map_level2.png");
    m->img_point = IMG_LoadTexture(re, "point.png");
    m->img_point2 = IMG_LoadTexture(re, "point2.png");
    m->img_mask1 = SDL_LoadBMP("assets/minimap/mask_level1.bmp");
    m->img_mask2 = SDL_LoadBMP("assets/minimap/mask_level2.bmp");
    m->img_cadre = IMG_LoadTexture(re, "assets/minimap/cadre.png");

    m->num_level  = 1;
    m->pos_map = (SDL_Rect){10, screenH - 130, 170, 120};
    m->pos_point  = (SDL_Rect){0, 0, 6, 6};
    m->pos_point2 = (SDL_Rect){0, 0, 6, 6};
    m->joueurX    = 0;
    m->joueurY    = 0;
    m->joueur2X   = 0;
    m->joueur2Y   = 0;
    m->shakeTimer     = 0;
    m->shakeIntensity = 0;
    m->shakeOffsetX   = 0;
    m->shakeOffsetY   = 0;
}

// P2 minimap — shares textures/masks with m1, just different pos_map
void init_minimap2(MiniMap *m2, SDL_Renderer *re, int screenH, int screenW) {
    // reuse same textures — no need to reload
    m2->img_map1  = IMG_LoadTexture(re, "assets/minimap/map_level1.png");
    m2->img_map2  = IMG_LoadTexture(re, "assets/minimap/map_level2.png");
    m2->img_point = IMG_LoadTexture(re, "point.png");
    m2->img_point2 = IMG_LoadTexture(re, "point2.png");
    m2->img_mask1 = SDL_LoadBMP("assets/minimap/mask_level1.bmp");
    m2->img_mask2 = SDL_LoadBMP("assets/minimap/mask_level2.bmp");
    m2->img_cadre = IMG_LoadTexture(re, "assets/minimap/cadre.png");

    m2->num_level  = 1;
    m2->pos_map = (SDL_Rect){screenW - 180, screenH - 130, 170, 120};
    m2->pos_point  = (SDL_Rect){0, 0, 6, 6};
    m2->pos_point2 = (SDL_Rect){0, 0, 6, 6};
    m2->joueurX    = 0;
    m2->joueurY    = 0;
    m2->joueur2X   = 0;
    m2->joueur2Y   = 0;
    m2->shakeTimer     = 0;
    m2->shakeIntensity = 0;
    m2->shakeOffsetX   = 0;
    m2->shakeOffsetY   = 0;
}

void afficher_minimap(MiniMap m, SDL_Renderer *re) {
    // apply shake offset
    SDL_Rect mapRect   = m.pos_map;
    SDL_Rect pointRect = m.pos_point;
    SDL_Rect point2Rect = m.pos_point2;
    mapRect.x   += m.shakeOffsetX;
    mapRect.y   += m.shakeOffsetY;
    pointRect.x += m.shakeOffsetX;
    pointRect.y += m.shakeOffsetY;
    point2Rect.x += m.shakeOffsetX;
    point2Rect.y += m.shakeOffsetY;

    // map background
    if (m.num_level == 1)
        SDL_RenderCopy(re, m.img_map1, NULL, &mapRect);
    else
        SDL_RenderCopy(re, m.img_map2, NULL, &mapRect);

    // P1 — red dot
    draw_filled_circle(re,
        pointRect.x + 3, pointRect.y + 3,
        4, (SDL_Color){255, 0, 0, 255});

    // P2 — blue dot
    draw_filled_circle(re,
        point2Rect.x + 3, point2Rect.y + 3,
        4, (SDL_Color){0, 100, 255, 255});

    // cadre — slightly larger than the map so it frames around it
    SDL_Rect cadreRect = {
        mapRect.x - 6, mapRect.y - 6,
        mapRect.w + 12, mapRect.h + 12
    };
    SDL_RenderCopy(re, m.img_cadre, NULL, &cadreRect);
}

void afficher_minimap2(MiniMap m, SDL_Renderer *re) {
    // identical render logic, different pos_map so points land correctly
    afficher_minimap(m, re);
}

/* --- 4.2 : Mise à jour --- */
/* MAP_W=1400 MAP_H=742, minimap tile=170x120 — compute per-axis ratios */
#define MINIMAP_WORLD_W 1400
#define MINIMAP_WORLD_H  742

void MAJ_minimap(MiniMap *m, int x1, int y1, int x2, int y2) {
    m->joueurX  = x1;
    m->joueurY  = y1;
    m->joueur2X = x2;
    m->joueur2Y = y2;

    float ratioX = (float)MINIMAP_WORLD_W / (float)m->pos_map.w;
    float ratioY = (float)MINIMAP_WORLD_H / (float)m->pos_map.h;

    m->pos_point.x  = m->pos_map.x + (int)(x1 / ratioX);
    m->pos_point.y  = m->pos_map.y + (int)(y1 / ratioY);

    m->pos_point2.x = m->pos_map.x + (int)(x2 / ratioX);
    m->pos_point2.y = m->pos_map.y + (int)(y2 / ratioY);
}

/* --- 4.3 : Collision Perfect Pixel --- */
int collision_PP(MiniMap *m, int x, int y) {
    SDL_Surface *mask = (m->num_level == 1) ? m->img_mask1 : m->img_mask2;
    if (!mask) return 0;

    Uint32 *pixels = (Uint32 *)mask->pixels;
    Uint32 color = pixels[y * mask->w + x];

    Uint8 r, g, b;
    SDL_GetRGB(color, mask->format, &r, &g, &b);

    return (r == 0 && g == 0 && b == 0) ? 1 : 0;
}

/* --- 4.4 : Collision BB --- */
int collision_BB(SDL_Rect a, SDL_Rect b) {
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}

/* --- 4.5 : Animation --- */
void animer_minimap(MiniMap *m, int frame) {
    if (frame % 2 == 0)
        SDL_SetTextureColorMod(m->img_point, 255, 0, 0);
    else
        SDL_SetTextureColorMod(m->img_point, 255, 255, 255);
}

/* --- Shake --- */
void minimap_trigger_shake(MiniMap *m) {
    if (m->shakeTimer <= 0) {
        m->shakeTimer     = 15;
        m->shakeIntensity = 4;
    }
}

void minimap_update_shake(MiniMap *m) {
    if (m->shakeTimer > 0) {
        int intensity = m->shakeIntensity * m->shakeTimer / 15;
        if (intensity < 1) intensity = 1;
        m->shakeOffsetX = (rand() % (2 * intensity + 1)) - intensity;
        m->shakeOffsetY = (rand() % (2 * intensity + 1)) - intensity;
        m->shakeTimer--;
    } else {
        m->shakeOffsetX = 0;
        m->shakeOffsetY = 0;
    }
}

/* --- Sauvegarde / Chargement --- */
void sauvegarder_jeu(MiniMap m, int score, char *nomF) {
    FILE *f = fopen(nomF, "wb");
    if (f) {
        fwrite(&m.joueurX, sizeof(int), 1, f);
        fwrite(&m.joueurY, sizeof(int), 1, f);
        fwrite(&m.num_level, sizeof(int), 1, f);
        fwrite(&score, sizeof(int), 1, f);
        fclose(f);
    }
}

void charger_jeu(MiniMap *m, int *score, char *nomF) {
    FILE *f = fopen(nomF, "rb");
    if (f) {
        fread(&m->joueurX, sizeof(int), 1, f);
        fread(&m->joueurY, sizeof(int), 1, f);
        fread(&m->num_level, sizeof(int), 1, f);
        fread(score, sizeof(int), 1, f);
        fclose(f);
    }
}

/* --- Nettoyage --- */
void liberer_minimap(MiniMap *m) {
    SDL_DestroyTexture(m->img_map1);
    SDL_DestroyTexture(m->img_map2);
    SDL_DestroyTexture(m->img_point);
    SDL_FreeSurface(m->img_mask1);
    SDL_FreeSurface(m->img_mask2);
    SDL_DestroyTexture(m->img_cadre);
}
