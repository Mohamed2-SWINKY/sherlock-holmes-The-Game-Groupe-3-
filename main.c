#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "minimap.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;

    SDL_Window *window = SDL_CreateWindow("Mini Map Level 1 & 2", 100, 100, 800, 600, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    MiniMap m;
    int score = 0, frame = 0;
    init_minimap(&m, renderer);

    SDL_Rect rectJoueur = {100, 100, 50, 50};
    SDL_Rect plateformeMobile = {400, 300, 100, 20};

    int running = 1;
    SDL_Event event;

    while (running) {
        frame++;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_RIGHT: m.joueurX += 10; break;
                    case SDLK_LEFT:  m.joueurX -= 10; break;
                    case SDLK_UP:    m.joueurY -= 10; break;
                    case SDLK_DOWN:  m.joueurY += 10; break;
                    case SDLK_n:     m.num_level = (m.num_level == 1) ? 2 : 1; break;
                    case SDLK_s:     sauvegarder_jeu(m, score, "savegame.bin"); break;
                    case SDLK_l:     charger_jeu(&m, &score, "savegame.bin"); break;
                }
            }
        }

        MAJ_minimap(&m, m.joueurX, m.joueurY, 10);

        if (collision_BB(rectJoueur, plateformeMobile) || collision_PP(&m, m.joueurX, m.joueurY)) {
            animer_minimap(&m, frame);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        afficher_minimap(m, renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    liberer_minimap(&m);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
