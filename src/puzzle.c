#include "puzzle.h"

#define GRID 3

// Update dimensions for 1000x650 window
#undef BG_X
#undef BG_Y
#define BG_X 50
#define BG_Y 85

void puzzle_init_state(PuzzleState *ps, SDL_Renderer *rend) {
    ps->fate = rand() % 5;
    char fname[64];
    sprintf(fname, "assets/puzzle/puzzle%d.png", ps->fate + 1);
    ps->bg = IMG_LoadTexture(rend, fname);
    if (!ps->bg) printf("Erreur img %s : %s\n", fname, IMG_GetError());

    ps->snd = Mix_LoadWAV("assets/enigme/hover.wav"); // Reusing enigme sound for now
    ps->fnt = TTF_OpenFont("assets/fonts/pixelFont.ttf", 48);

    SDL_Color gold = {255, 215, 0, 255};
    SDL_Color red  = {210, 55, 55, 255};
    SDL_Surface *ws = TTF_RenderText_Solid(ps->fnt, "You have won !", gold);
    SDL_Surface *ls = TTF_RenderText_Solid(ps->fnt, "Time is out - You lost !", red);
    ps->wTex = SDL_CreateTextureFromSurface(rend, ws);
    ps->lTex = SDL_CreateTextureFromSurface(rend, ls);
    SDL_FreeSurface(ws);
    SDL_FreeSurface(ls);

    // Init logic
    int imgW, imgH;
    SDL_QueryTexture(ps->bg, NULL, NULL, &imgW, &imgH);

    int cw = BG_W / GRID,  ch = BG_H / GRID;
    int sw = imgW / GRID,  sh = imgH / GRID;

    int hRow = rand() % GRID, hCol = rand() % GRID;
    int wRow[2], wCol[2], k;
    for (k = 0; k < 2; k++) {
        do {
            wRow[k] = rand() % GRID;
            wCol[k] = rand() % GRID;
        } while ((wRow[k] == hRow && wCol[k] == hCol) ||
                 (k == 1 && wRow[k] == wRow[0] && wCol[k] == wCol[0]));
    }

    int correctSlot = rand() % 3;
    SDL_Rect srcPiece[3];
    srcPiece[correctSlot] = (SDL_Rect){ hCol*sw,    hRow*sh,    sw, sh };
    int w0 = (correctSlot == 0) ? 1 : 0;
    srcPiece[w0]          = (SDL_Rect){ wCol[0]*sw, wRow[0]*sh, sw, sh };
    int w1 = (correctSlot == 2) ? 1 : 2;
    srcPiece[w1]          = (SDL_Rect){ wCol[1]*sw, wRow[1]*sh, sw, sh };

    int slotX = BG_X + BG_W + 50; // More space for 1000 window
    ps->slots[0] = (SDL_Rect){ slotX, 60+40,              cw, ch };
    ps->slots[1] = (SDL_Rect){ slotX, 60+40 + ch + 30,   cw, ch };
    ps->slots[2] = (SDL_Rect){ slotX, 60+40 + 2*(ch+30), cw, ch };

    for (int i = 0; i < 3; i++) {
        ps->pc[i].src     = srcPiece[i];
        ps->pc[i].pos     = ps->slots[i];
        ps->pc[i].orig    = ps->slots[i];
        ps->pc[i].drag    = 0;
        ps->pc[i].placed  = 0;
        ps->pc[i].correct = (i == correctSlot);
    }

    ps->hScr   = (SDL_Rect){ BG_X + hCol*cw, BG_Y + hRow*ch, cw, ch };
    ps->target = ps->hScr;

    ps->startTime = SDL_GetTicks();
    ps->dIdx = -1;
    ps->ox = ps->oy = 0;
    ps->run = 1;
    ps->result = -1;
    ps->over = 0;
    ps->angle = 0.0;
    ps->scale = 1.0f;
}

void puzzle_handle_event(PuzzleState *ps, SDL_Event *ev) {
    if (!ps->run) return;

    if (ev->type == SDL_MOUSEBUTTONDOWN) {
        int mx = ev->button.x, my = ev->button.y;
        for (int i = 0; i < 3; i++) {
            if (!ps->pc[i].placed &&
                mx >= ps->pc[i].pos.x && mx < ps->pc[i].pos.x + ps->pc[i].pos.w &&
                my >= ps->pc[i].pos.y && my < ps->pc[i].pos.y + ps->pc[i].pos.h) {
                ps->dIdx = i;
                ps->ox = mx - ps->pc[i].pos.x;
                ps->oy = my - ps->pc[i].pos.y;
                ps->pc[i].drag = 1;
                break;
            }
        }
    } else if (ev->type == SDL_MOUSEMOTION) {
        if (ps->dIdx >= 0) {
            ps->pc[ps->dIdx].pos.x = ev->motion.x - ps->ox;
            ps->pc[ps->dIdx].pos.y = ev->motion.y - ps->oy;
        }
    } else if (ev->type == SDL_MOUSEBUTTONUP) {
        if (ps->dIdx < 0) return;
        ps->pc[ps->dIdx].drag = 0;
        int cx = ps->pc[ps->dIdx].pos.x + ps->pc[ps->dIdx].pos.w / 2;
        int cy = ps->pc[ps->dIdx].pos.y + ps->pc[ps->dIdx].pos.h / 2;
        int tx = ps->target.x + ps->target.w / 2;
        int ty = ps->target.y + ps->target.h / 2;
        int d = (int)sqrt((double)((cx-tx)*(cx-tx) + (cy-ty)*(cy-ty)));

        if (d < SNAP) {
            if (ps->pc[ps->dIdx].correct) {
                ps->pc[ps->dIdx].pos = ps->target;
                ps->pc[ps->dIdx].placed = 1;
                ps->result = 1;
                ps->run = 0;
            } else {
                ps->pc[ps->dIdx].pos = ps->pc[ps->dIdx].orig;
            }
        }
        ps->dIdx = -1;
    }
}

void puzzle_update(PuzzleState *ps) {
    if (!ps->run) {
        ps->angle += 5.0;
        if (ps->angle >= 360.0) ps->angle -= 360.0;

        // Pulsing scale effect
        static float t = 0.0f;
        t += 0.1f;
        ps->scale = 1.0f + 0.2f * sinf(t);

        static Uint32 endT = 0;
        if (endT == 0) endT = SDL_GetTicks();
        if (SDL_GetTicks() - endT > 3000) { // Increased delay to see animation
            ps->over = 1;
            endT = 0;
            t = 0.0f;
        }
        return;
    }

    if (SDL_GetTicks() - ps->startTime >= TMAX) {
        ps->result = 0;
        ps->run = 0;
    }
}

void puzzle_render(PuzzleState *ps, SDL_Renderer *rend) {
    int cw = BG_W / GRID, ch = BG_H / GRID;
    
    SDL_SetRenderDrawColor(rend, 30, 30, 30, 255);
    SDL_RenderClear(rend);

    SDL_Rect bgDest = {BG_X, BG_Y, BG_W, BG_H};
    SDL_RenderCopy(rend, ps->bg, NULL, &bgDest);

    SDL_SetRenderDrawColor(rend, 40, 40, 40, 255);
    SDL_RenderFillRect(rend, &ps->hScr);

    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    for (int r = 0; r <= GRID; r++)
        SDL_RenderDrawLine(rend, BG_X, BG_Y+r*ch, BG_X+BG_W, BG_Y+r*ch);
    for (int c = 0; c <= GRID; c++)
        SDL_RenderDrawLine(rend, BG_X+c*cw, BG_Y, BG_X+c*cw, BG_Y+BG_H);

    for (int i = 0; i < 3; i++) {
        SDL_RenderCopy(rend, ps->bg, &ps->pc[i].src, &ps->pc[i].pos);
        SDL_SetRenderDrawColor(rend, 200, 180, 40, 255);
        SDL_RenderDrawRect(rend, &ps->pc[i].pos);
    }

    // Timer bar
    int bw = (int)((TMAX - (int)(SDL_GetTicks() - ps->startTime)) * 1000 / TMAX);
    if (bw < 0) bw = 0;
    SDL_SetRenderDrawColor(rend, 210, 55, 55, 255);
    SDL_Rect bar = {0, 0, bw, 25};
    SDL_RenderFillRect(rend, &bar);

    if (!ps->run) {
        SDL_Texture *resTex = ps->result == 1 ? ps->wTex : ps->lTex;
        int tw, th;
        SDL_QueryTexture(resTex, NULL, NULL, &tw, &th);
        
        int dw = (int)(tw * ps->scale);
        int dh = (int)(th * ps->scale);
        SDL_Rect dst = { 500 - dw/2, 325 - dh/2, dw, dh };
        
        SDL_RenderCopyEx(rend, resTex, NULL, &dst, ps->angle, NULL, SDL_FLIP_NONE);
    }
}

void puzzle_free_state(PuzzleState *ps) {
    if (ps->bg) SDL_DestroyTexture(ps->bg);
    if (ps->wTex) SDL_DestroyTexture(ps->wTex);
    if (ps->lTex) SDL_DestroyTexture(ps->lTex);
    if (ps->snd) Mix_FreeChunk(ps->snd);
    if (ps->fnt) TTF_CloseFont(ps->fnt);
}

// Stub enigme2 to avoid compile errors if called elsewhere
int enigme2(SDL_Renderer *rend, int fate, int *solved) {
    (void)rend; (void)fate; (void)solved;
    return 0;
}
