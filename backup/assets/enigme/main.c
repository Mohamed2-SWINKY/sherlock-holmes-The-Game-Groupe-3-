#include "enigme.h"
#include <stdio.h>

int main()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    IMG_Init(IMG_INIT_PNG);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    SDL_Window *win = SDL_CreateWindow("ENIGME",
                                      SDL_WINDOWPOS_CENTERED,
                                      SDL_WINDOWPOS_CENTERED,
                                      900, 600, 0);

    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    Enigme e;
    initEnigme(&e, renderer);

    int running = 1;
    SDL_Event event;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = 0;

            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                running = 0;

            handleEnigmeEvents(&e, event);
        }

        SDL_RenderClear(renderer);
        renderEnigme(&e, renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    freeEnigme(&e);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
