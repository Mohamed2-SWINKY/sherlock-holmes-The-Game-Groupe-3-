#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    SDL_Window *win = SDL_CreateWindow("Quiz Pro",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600, 0);

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    // background
    SDL_Surface *bgSurf = IMG_Load("londonBackground.jpg");
    if (!bgSurf) {
        printf("Erreur image\n");
        return 1;
    }
    SDL_Texture *bg = SDL_CreateTextureFromSurface(ren, bgSurf);
    SDL_FreeSurface(bgSurf);

    // sound
    Mix_Chunk *click = Mix_LoadWAV("click.wav");

    // font
    TTF_Font *font = TTF_OpenFont("DejaVuSans.ttf", 24);
    if (!font) {
        printf("Erreur font\n");
        return 1;
    }

    Enigme t[MAX];
    int n;

    if (!charger(t, &n)) {
        printf("Erreur questions.txt\n");
        return 1;
    }

    srand(time(NULL));
    int current = generer(t, n);
    int score = 0;

    // TIMER
    Uint32 startTime = SDL_GetTicks();
    int maxTime = 10;

    // boutons
    SDL_Rect btnA = {50, 150, 300, 50};
    SDL_Rect btnB = {50, 220, 300, 50};
    SDL_Rect btnC = {50, 290, 300, 50};

    SDL_Event e;
    int running = 1;

    while (running) {

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = 0;

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                char choix = 0;
                SDL_Point p = {mouseX, mouseY};

                if (SDL_PointInRect(&p, &btnA)) choix = 'A';
                if (SDL_PointInRect(&p, &btnB)) choix = 'B';
                if (SDL_PointInRect(&p, &btnC)) choix = 'C';

                if (choix) {
                    Mix_PlayChannel(-1, click, 0);

                    if (choix == t[current].correcte)
                        score++;

                    current = generer(t, n);
                    startTime = SDL_GetTicks(); // reset timer
                }
            }
        }

        // TIMER CALCUL
        Uint32 now = SDL_GetTicks();
        int timeLeft = maxTime - (now - startTime) / 1000;

        if (timeLeft <= 0) {
            current = generer(t, n);
            startTime = SDL_GetTicks();
        }

        // affichage
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, bg, NULL, NULL);

        SDL_Color white = {255,255,255,255};

        SDL_Texture *q = renderText(t[current].question, font, white, ren);
        SDL_Texture *a = renderText(t[current].A, font, white, ren);
        SDL_Texture *b = renderText(t[current].B, font, white, ren);
        SDL_Texture *c = renderText(t[current].C, font, white, ren);

        char scoreTxt[50];
        sprintf(scoreTxt, "Score: %d", score);
        SDL_Texture *s = renderText(scoreTxt, font, white, ren);

        drawText(q, 50, 50, ren);

        SDL_Point p = {mouseX, mouseY};

        // boutons + hover
        SDL_SetRenderDrawColor(ren, 0, 0, 255, 255);
        if (SDL_PointInRect(&p, &btnA)) SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
        SDL_RenderFillRect(ren, &btnA);

        SDL_SetRenderDrawColor(ren, 0, 0, 255, 255);
        if (SDL_PointInRect(&p, &btnB)) SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
        SDL_RenderFillRect(ren, &btnB);

        SDL_SetRenderDrawColor(ren, 0, 0, 255, 255);
        if (SDL_PointInRect(&p, &btnC)) SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
        SDL_RenderFillRect(ren, &btnC);

        drawText(a, 60, 160, ren);
        drawText(b, 60, 230, ren);
        drawText(c, 60, 300, ren);
        drawText(s, 600, 20, ren);

        // TIMER BAR
        int barWidth = (timeLeft * 300) / maxTime;
        SDL_Rect timerBar = {50, 400, barWidth, 20};

        SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
        SDL_RenderFillRect(ren, &timerBar);

        SDL_RenderPresent(ren);

        SDL_DestroyTexture(q);
        SDL_DestroyTexture(a);
        SDL_DestroyTexture(b);
        SDL_DestroyTexture(c);
        SDL_DestroyTexture(s);
    }

    // cleanup
    Mix_FreeChunk(click);
    Mix_CloseAudio();

    SDL_DestroyTexture(bg);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}
