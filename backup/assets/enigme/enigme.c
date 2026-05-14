#include "enigme.h"
#include <stdio.h>

// Helper: scale a rect outward from its center
static SDL_Rect scaleRect(SDL_Rect r, float scale)
{
    int nw = (int)(r.w * scale);
    int nh = (int)(r.h * scale);
    return (SDL_Rect){
        r.x - (nw - r.w) / 2,
        r.y - (nh - r.h) / 2,
        nw,
        nh
    };
}

// HOVER SOUND — dedicated channel 1, same as reference
static void play_hover_sound(Enigme *e)
{
    if (e->hoverSound)
    {
        Mix_HaltChannel(1);
        Mix_Volume(1, 100);
        Mix_PlayChannel(1, e->hoverSound, 0);
    }
}

// INITIALISATION
void initEnigme(Enigme *e, SDL_Renderer *r)
{
    e->bg = IMG_LoadTexture(r, "background4.png");
    if (!e->bg) fprintf(stderr, "Failed to load background4.png: %s\n", IMG_GetError());

    e->btnQuiz = IMG_LoadTexture(r, "quiz.png");
    if (!e->btnQuiz) fprintf(stderr, "Failed to load quiz.png: %s\n", IMG_GetError());

    e->btnPuzzle = IMG_LoadTexture(r, "puzzle.png");
    if (!e->btnPuzzle) fprintf(stderr, "Failed to load puzzle.png: %s\n", IMG_GetError());

    e->texA = IMG_LoadTexture(r, "A.png");
    if (!e->texA) fprintf(stderr, "Failed to load A.png: %s\n", IMG_GetError());

    e->texB = IMG_LoadTexture(r, "B.png");
    if (!e->texB) fprintf(stderr, "Failed to load B.png: %s\n", IMG_GetError());

    e->texC = IMG_LoadTexture(r, "C.png");
    if (!e->texC) fprintf(stderr, "Failed to load C.png: %s\n", IMG_GetError());

    e->texQuestion = IMG_LoadTexture(r, "question.png");
    if (!e->texQuestion) fprintf(stderr, "Failed to load question.png: %s\n", IMG_GetError());

    e->hoverSound = Mix_LoadWAV("hover.wav");
    if (!e->hoverSound) fprintf(stderr, "Failed to load hover.wav: %s\n", Mix_GetError());

    e->quizMusic = Mix_LoadMUS("quiz_music.mp3");
    if (!e->quizMusic) fprintf(stderr, "Failed to load quiz_music.mp3: %s\n", Mix_GetError());

    e->quizRect   = (SDL_Rect){200, 380, 190, 80};
    e->puzzleRect = (SDL_Rect){500, 380, 190, 80};
    e->A          = (SDL_Rect){290, 390, 90, 60};
    e->B          = (SDL_Rect){405, 390, 90, 60};
    e->C          = (SDL_Rect){520, 390, 90, 60};
    e->band       = (SDL_Rect){250, 150, 400, 320};

    e->showQuiz = 0;
    e->hoverQuiz = e->hoverPuzzle = e->hoverA = e->hoverB = e->hoverC = 0;
    e->lastHover = -1;
}

// EVENTS
void handleEnigmeEvents(Enigme *e, SDL_Event event)
{
    int mx, my;
    static int lastHovered = -1;

    if (event.type == SDL_MOUSEMOTION)
    {
        mx = event.motion.x;
        my = event.motion.y;

        e->hoverQuiz = e->hoverPuzzle = e->hoverA = e->hoverB = e->hoverC = 0;
        int currentHover = -1;

        if (!e->showQuiz)
        {
            if (mx > e->quizRect.x && mx < e->quizRect.x + e->quizRect.w &&
                my > e->quizRect.y && my < e->quizRect.y + e->quizRect.h)
            { e->hoverQuiz = 1; currentHover = 1; }

            if (mx > e->puzzleRect.x && mx < e->puzzleRect.x + e->puzzleRect.w &&
                my > e->puzzleRect.y && my < e->puzzleRect.y + e->puzzleRect.h)
            { e->hoverPuzzle = 1; currentHover = 2; }
        }

        if (e->showQuiz)
        {
            if (mx > e->A.x && mx < e->A.x + e->A.w && my > e->A.y && my < e->A.y + e->A.h)
            { e->hoverA = 1; currentHover = 3; }

            if (mx > e->B.x && mx < e->B.x + e->B.w && my > e->B.y && my < e->B.y + e->B.h)
            { e->hoverB = 1; currentHover = 4; }

            if (mx > e->C.x && mx < e->C.x + e->C.w && my > e->C.y && my < e->C.y + e->C.h)
            { e->hoverC = 1; currentHover = 5; }
        }

        if (currentHover != lastHovered && currentHover != -1)
            play_hover_sound(e);

        lastHovered = currentHover;
        e->lastHover = currentHover;
    }

    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
    {
        if (e->showQuiz)
        {
            e->showQuiz = 0;
            Mix_HaltMusic();
            lastHovered = -1;
            e->lastHover = -1;
        }
    }

    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        mx = event.button.x;
        my = event.button.y;

        if (!e->showQuiz)
        {
            if (mx > e->quizRect.x && mx < e->quizRect.x + e->quizRect.w &&
                my > e->quizRect.y && my < e->quizRect.y + e->quizRect.h)
            {
                e->showQuiz = 1;
                lastHovered = -1;
                e->lastHover = -1;
                if (e->quizMusic) Mix_PlayMusic(e->quizMusic, -1);
            }

            if (mx > e->puzzleRect.x && mx < e->puzzleRect.x + e->puzzleRect.w &&
                my > e->puzzleRect.y && my < e->puzzleRect.y + e->puzzleRect.h)
            {
                printf("Puzzle selected\n");
            }
        }

        if (e->showQuiz)
        {
            if (mx > e->A.x && mx < e->A.x + e->A.w && my > e->A.y && my < e->A.y + e->A.h)
                printf("Answer A selected\n");

            if (mx > e->B.x && mx < e->B.x + e->B.w && my > e->B.y && my < e->B.y + e->B.h)
                printf("Answer B selected\n");

            if (mx > e->C.x && mx < e->C.x + e->C.w && my > e->C.y && my < e->C.y + e->C.h)
                printf("Answer C selected\n");
        }
    }
}

// RENDER
void renderEnigme(Enigme *e, SDL_Renderer *r)
{
    SDL_RenderCopy(r, e->bg, NULL, NULL);

    if (!e->showQuiz)
    {
        SDL_Rect destQuiz   = e->hoverQuiz   ? scaleRect(e->quizRect,   1.15f) : e->quizRect;
        SDL_Rect destPuzzle = e->hoverPuzzle ? scaleRect(e->puzzleRect, 1.15f) : e->puzzleRect;

        SDL_RenderCopy(r, e->btnQuiz,   NULL, &destQuiz);
        SDL_RenderCopy(r, e->btnPuzzle, NULL, &destPuzzle);
    }

    if (e->showQuiz)
    {
        SDL_RenderCopy(r, e->texQuestion, NULL, &e->band);

        SDL_Rect destA = e->hoverA ? scaleRect(e->A, 1.15f) : e->A;
        SDL_Rect destB = e->hoverB ? scaleRect(e->B, 1.15f) : e->B;
        SDL_Rect destC = e->hoverC ? scaleRect(e->C, 1.15f) : e->C;

        SDL_RenderCopy(r, e->texA, NULL, &destA);
        SDL_RenderCopy(r, e->texB, NULL, &destB);
        SDL_RenderCopy(r, e->texC, NULL, &destC);
    }
}

// FREE
void freeEnigme(Enigme *e)
{
    SDL_DestroyTexture(e->bg);
    SDL_DestroyTexture(e->btnQuiz);
    SDL_DestroyTexture(e->btnPuzzle);
    SDL_DestroyTexture(e->quizTitle);
    SDL_DestroyTexture(e->texA);
    SDL_DestroyTexture(e->texB);
    SDL_DestroyTexture(e->texC);
    SDL_DestroyTexture(e->texQuestion);

    Mix_FreeChunk(e->hoverSound);
    Mix_FreeMusic(e->quizMusic);
}
