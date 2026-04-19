#ifndef MINIMAP_H
#define MINIMAP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

typedef struct {
    SDL_Texture *img_map1;
    SDL_Texture *img_map2;
    SDL_Texture *img_point;
    SDL_Texture *img_point2;
    SDL_Surface *img_mask1;   // Mask pour Level 1
    SDL_Surface *img_mask2;   // Mask pour Level 2
    SDL_Texture *img_cadre;
    SDL_Rect pos_map;
    SDL_Rect pos_point;    // P1's point
    SDL_Rect pos_point2;   // P2's pointSDL_Rect pos_point;
    int joueurX, joueurY;
    int joueur2X, joueur2Y;
    int num_level;
    int shakeTimer;
    int shakeIntensity;
    int shakeOffsetX;
    int shakeOffsetY;
} MiniMap;

// --- Initialisation et affichage ---
void init_minimap(MiniMap *m, SDL_Renderer *re, int screenH);
void init_minimap2(MiniMap *m2, SDL_Renderer *re, int screenH, int screenW);
void afficher_minimap(MiniMap m, SDL_Renderer *re);
void afficher_minimap2(MiniMap m, SDL_Renderer *re);

// --- Mise à jour ---
// ratioX = MAP_W / minimap_w, ratioY = MAP_H / minimap_h
void MAJ_minimap(MiniMap *m, int x1, int y1, int x2, int y2);

// --- Collisions ---
int collision_PP(MiniMap *m, int x, int y); // Perfect Pixel
int collision_BB(SDL_Rect joueur, SDL_Rect plateforme);

// --- Animation ---
void animer_minimap(MiniMap *m, int frame);

// --- Shake ---
void minimap_trigger_shake(MiniMap *m);
void minimap_update_shake(MiniMap *m);

// --- Sauvegarde / Chargement ---
void sauvegarder_jeu(MiniMap m, int score, char *nomF);
void charger_jeu(MiniMap *m, int *score, char *nomF);

// --- Nettoyage ---
void liberer_minimap(MiniMap *m);

#endif
