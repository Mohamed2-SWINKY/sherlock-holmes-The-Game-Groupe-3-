#include "minimap.h"

/* --- 4.1 : INITIALISATION ET AFFICHAGE --- */
void init_minimap(MiniMap *m, SDL_Renderer *re) {
    m->img_map1 = IMG_LoadTexture(re, "map_level1.png");
    m->img_map2 = IMG_LoadTexture(re, "map_level2.png");
    m->img_point = IMG_LoadTexture(re, "point.png");

    m->img_mask1 = SDL_LoadBMP("mask_level1.bmp");
    m->img_mask2 = SDL_LoadBMP("mask_level2.bmp");

    m->num_level = 1;
    m->pos_map = (SDL_Rect){10, 10, 200, 150};
    m->pos_point = (SDL_Rect){0, 0, 30, 30};
    m->joueurX = 0;
    m->joueurY = 0;
}

void afficher_minimap(MiniMap m, SDL_Renderer *re) {
    if (m.num_level == 1)
        SDL_RenderCopy(re, m.img_map1, NULL, &m.pos_map);
    else
        SDL_RenderCopy(re, m.img_map2, NULL, &m.pos_map);

    SDL_RenderCopy(re, m.img_point, NULL, &m.pos_point);
}

/* --- 4.2 : Mise à jour --- */
void MAJ_minimap(MiniMap *m, int x, int y, int ratio) {
    m->joueurX = x;
    m->joueurY = y;
    m->pos_point.x = m->pos_map.x + (x / ratio);
    m->pos_point.y = m->pos_map.y + (y / ratio);
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
int collision_BB(SDL_Rect joueur, SDL_Rect plateforme) {
    return SDL_HasIntersection(&joueur, &plateforme);
}

/* --- 4.5 : Animation --- */
void animer_minimap(MiniMap *m, int frame) {
    if (frame % 2 == 0)
        SDL_SetTextureColorMod(m->img_point, 255, 0, 0);
    else
        SDL_SetTextureColorMod(m->img_point, 255, 255, 255);
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
}

