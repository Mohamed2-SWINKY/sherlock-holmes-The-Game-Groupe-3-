#include "header.h"

int main() {
    srand(time(NULL));
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    SDL_Window   *win  = SDL_CreateWindow("Enigme 2",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            W, H, 0);
    SDL_Renderer *rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    int solved = 0;
    int fate = rand() % NB;
    enigme2(rend, fate, &solved);

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
