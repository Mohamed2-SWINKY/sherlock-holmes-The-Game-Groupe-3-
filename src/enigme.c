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
    e->renderer = r;
    e->bg = IMG_LoadTexture(r, "assets/enigme/background4.png");
    if (!e->bg) fprintf(stderr, "Failed to load background4.png: %s\n", IMG_GetError());

    e->btnQuiz = IMG_LoadTexture(r, "assets/enigme/quiz.png");
    if (!e->btnQuiz) fprintf(stderr, "Failed to load quiz.png: %s\n", IMG_GetError());

    e->btnPuzzle = IMG_LoadTexture(r, "assets/enigme/puzzle.png");
    if (!e->btnPuzzle) fprintf(stderr, "Failed to load puzzle.png: %s\n", IMG_GetError());

    e->texA = IMG_LoadTexture(r, "assets/enigme/A.png");
    if (!e->texA) fprintf(stderr, "Failed to load A.png: %s\n", IMG_GetError());

    e->texB = IMG_LoadTexture(r, "assets/enigme/B.png");
    if (!e->texB) fprintf(stderr, "Failed to load B.png: %s\n", IMG_GetError());

    e->texC = IMG_LoadTexture(r, "assets/enigme/C.png");
    if (!e->texC) fprintf(stderr, "Failed to load C.png: %s\n", IMG_GetError());

    e->texQuestion = IMG_LoadTexture(r, "assets/enigme/question.png");
    if (!e->texQuestion) fprintf(stderr, "Failed to load question.png: %s\n", IMG_GetError());

    e->hoverSound = Mix_LoadWAV("assets/enigme/hover.wav");
    if (!e->hoverSound) fprintf(stderr, "Failed to load hover.wav: %s\n", Mix_GetError());

    e->quizMusic = Mix_LoadMUS("assets/enigme/quiz_music.mp3");
    if (!e->quizMusic) fprintf(stderr, "Failed to load quiz_music.mp3: %s\n", Mix_GetError());

    // Centering for 1000x650 window
    e->quizRect   = (SDL_Rect){260, 450, 190, 80};
    e->puzzleRect = (SDL_Rect){550, 450, 190, 80};
    
    e->band       = (SDL_Rect){300, 100, 400, 320};
    
    int startABC = (1000 - 320) / 2; // 340
    e->A          = (SDL_Rect){startABC, 450, 90, 60};
    e->B          = (SDL_Rect){startABC + 115, 450, 90, 60};
    e->C          = (SDL_Rect){startABC + 230, 450, 90, 60};

    e->showQuiz = 0;
    e->hoverQuiz = e->hoverPuzzle = e->hoverA = e->hoverB = e->hoverC = 0;
    e->lastHover = -1;
    e->over = 0;
    e->puzzleSelected = 0;

    e->font = TTF_OpenFont("assets/fonts/pixelFont.ttf", 24);
    e->fontSmall = TTF_OpenFont("assets/fonts/pixelFont.ttf", 18);
    e->numQuestions = 0;
    charger(e->questions, &e->numQuestions);
    e->currentQuestionIdx = -1;
    e->result = -1;
    e->texQText = e->texAText = e->texBText = e->texCText = NULL;
    e->quizTimeLimit = 10000; // 10 seconds
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
            lastHovered = -1;
            e->lastHover = -1;
            e->over = 1;
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
                // if (e->quizMusic) Mix_PlayMusic(e->quizMusic, -1);

                e->quizStartTime = SDL_GetTicks();

                // Load random question
                e->currentQuestionIdx = generer(e->questions, e->numQuestions);
                if (e->currentQuestionIdx != -1) {
                    SDL_Color white = {255, 255, 255, 255};
                    QuizQuestion q = e->questions[e->currentQuestionIdx];
                    e->texQText = renderText(q.question, e->fontSmall, white, e->renderer, 400); // Use fontSmall
                    e->texAText = renderText(q.A, e->font, white, e->renderer, 0);
                    e->texBText = renderText(q.B, e->font, white, e->renderer, 0);
                    e->texCText = renderText(q.C, e->font, white, e->renderer, 0);
                }
            }

            if (mx > e->puzzleRect.x && mx < e->puzzleRect.x + e->puzzleRect.w &&
                my > e->puzzleRect.y && my < e->puzzleRect.y + e->puzzleRect.h)
            {
                printf("Puzzle selected\n");
                e->puzzleSelected = 1;
                e->over = 1;
            }
        }
        else if (e->showQuiz)
        {
            char choice = ' ';
            if (mx > e->A.x && mx < e->A.x + e->A.w && my > e->A.y && my < e->A.y + e->A.h) {
                choice = 'A';
            }
            if (mx > e->B.x && mx < e->B.x + e->B.w && my > e->B.y && my < e->B.y + e->B.h) {
                choice = 'B';
            }
            if (mx > e->C.x && mx < e->C.x + e->C.w && my > e->C.y && my < e->C.y + e->C.h) {
                choice = 'C';
            }

            if (choice != ' ' && e->currentQuestionIdx != -1) {
                if (choice == e->questions[e->currentQuestionIdx].correcte) {
                    printf("Correct!\n");
                    e->result = 1;
                } else {
                    printf("Wrong!\n");
                    e->result = 0;
                }
                e->over = 1;
            }
            
            if (e->over) {
                e->showQuiz = 0;
            }
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
        Uint32 elapsed = SDL_GetTicks() - e->quizStartTime;
        if (elapsed > (Uint32)e->quizTimeLimit) {
            e->result = 0;
            e->over = 1;
            e->showQuiz = 0;
            return;
        }

        // Draw Timer Bar
        float pct = 1.0f - (float)elapsed / e->quizTimeLimit;
        SDL_Rect bar = { (1000 - 400) / 2, 435, (int)(400 * pct), 10 };
        SDL_SetRenderDrawColor(r, 255, 0, 0, 255);
        SDL_RenderFillRect(r, &bar);

        SDL_RenderCopy(r, e->texQuestion, NULL, &e->band);

        SDL_Rect destA = e->hoverA ? scaleRect(e->A, 1.15f) : e->A;
        SDL_Rect destB = e->hoverB ? scaleRect(e->B, 1.15f) : e->B;
        SDL_Rect destC = e->hoverC ? scaleRect(e->C, 1.15f) : e->C;

        SDL_RenderCopy(r, e->texA, NULL, &destA);
        SDL_RenderCopy(r, e->texB, NULL, &destB);
        SDL_RenderCopy(r, e->texC, NULL, &destC);

        // Draw Dynamic Text
        if (e->texQText) {
            SDL_Rect qr; SDL_QueryTexture(e->texQText, NULL, NULL, &qr.w, &qr.h);
            qr.x = e->band.x + (e->band.w - qr.w) / 2;
            qr.y = e->band.y + (e->band.h - qr.h) / 2;
            SDL_RenderCopy(r, e->texQText, NULL, &qr);
        }
        if (e->texAText) {
            SDL_Rect ar; SDL_QueryTexture(e->texAText, NULL, NULL, &ar.w, &ar.h);
            ar.x = e->A.x + (e->A.w - ar.w) / 2;
            ar.y = e->A.y + e->A.h + 10;
            SDL_RenderCopy(r, e->texAText, NULL, &ar);
        }
        if (e->texBText) {
            SDL_Rect br; SDL_QueryTexture(e->texBText, NULL, NULL, &br.w, &br.h);
            br.x = e->B.x + (e->B.w - br.w) / 2;
            br.y = e->B.y + e->B.h + 10;
            SDL_RenderCopy(r, e->texBText, NULL, &br);
        }
        if (e->texCText) {
            SDL_Rect cr; SDL_QueryTexture(e->texCText, NULL, NULL, &cr.w, &cr.h);
            cr.x = e->C.x + (e->C.w - cr.w) / 2;
            cr.y = e->C.y + e->C.h + 10;
            SDL_RenderCopy(r, e->texCText, NULL, &cr);
        }
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

    if (e->texQText) SDL_DestroyTexture(e->texQText);
    if (e->texAText) SDL_DestroyTexture(e->texAText);
    if (e->texBText) SDL_DestroyTexture(e->texBText);
    if (e->texCText) SDL_DestroyTexture(e->texCText);

    if (e->font) TTF_CloseFont(e->font);
    if (e->fontSmall) TTF_CloseFont(e->fontSmall);

    Mix_FreeChunk(e->hoverSound);
    Mix_FreeMusic(e->quizMusic);
}
