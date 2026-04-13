#ifndef MINIMAP_H
#define MINIMAP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

typedef struct {
    SDL_Texture *img_map1;
    SDL_Texture *img_map2;
    SDL_Texture *img_point;
    SDL_Surface *img_mask1;   // Mask pour Level 1
    SDL_Surface *img_mask2;   // Mask pour Level 2
    SDL_Rect pos_map;
    SDL_Rect pos_point;
    int joueurX, joueurY;
    int num_level;
} MiniMap;

// --- Initialisation et affichage ---
void init_minimap(MiniMap *m, SDL_Renderer *re);
void afficher_minimap(MiniMap m, SDL_Renderer *re);

// --- Mise à jour ---
void MAJ_minimap(MiniMap *m, int x, int y, int ratio);

// --- Collisions ---
int collision_PP(MiniMap *m, int x, int y); // Perfect Pixel
int collision_BB(SDL_Rect joueur, SDL_Rect plateforme);

// --- Animation ---
void animer_minimap(MiniMap *m, int frame);

// --- Sauvegarde / Chargement ---
void sauvegarder_jeu(MiniMap m, int score, char *nomF);
void charger_jeu(MiniMap *m, int *score, char *nomF);

// --- Nettoyage ---
void liberer_minimap(MiniMap *m);

#endif



