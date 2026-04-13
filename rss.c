#include "header.h"

#define GRID 3

int score = 0;

int load_resources(SDL_Renderer *rend, int fate,
                   SDL_Texture **bg,
                   Mix_Chunk   **snd,
                   TTF_Font    **fnt,
                   SDL_Texture **wTex,
                   SDL_Texture **lTex)
{
    char fname[32];
    sprintf(fname, "puzzle%d.png", fate + 1);
    *bg = IMG_LoadTexture(rend, fname);
    if (!*bg) { printf("Erreur img %s : %s\n", fname, IMG_GetError()); return 0; }

    *snd = Mix_LoadWAV("effect.wav");
    *fnt = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 48);

    SDL_Color wh = {255, 255, 255, 255};
    SDL_Surface *ws = TTF_RenderText_Solid(*fnt, "You have won !", wh);
    SDL_Surface *ls = TTF_RenderText_Solid(*fnt, "Time is out - You lost !", wh);
    *wTex = SDL_CreateTextureFromSurface(rend, ws);
    *lTex = SDL_CreateTextureFromSurface(rend, ls);
    SDL_FreeSurface(ws);
    SDL_FreeSurface(ls);
    return 1;
}

void free_resources(SDL_Texture *bg,
                    SDL_Texture *wTex, SDL_Texture *lTex,
                    Mix_Chunk   *snd,  TTF_Font    *fnt)
{
    SDL_DestroyTexture(bg);
    SDL_DestroyTexture(wTex);
    SDL_DestroyTexture(lTex);
    if (snd) Mix_FreeChunk(snd);
    TTF_CloseFont(fnt);
}

void init_puzzle(SDL_Texture *bg,
                 Piece pc[3], SDL_Rect slots[3],
                 SDL_Rect *hScr, SDL_Rect *target)
{
    int imgW, imgH;
    SDL_QueryTexture(bg, NULL, NULL, &imgW, &imgH);

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

    int slotX = BG_X + BG_W + 20;
    slots[0] = (SDL_Rect){ slotX, 60,              cw, ch };
    slots[1] = (SDL_Rect){ slotX, 60 + ch + 30,   cw, ch };
    slots[2] = (SDL_Rect){ slotX, 60 + 2*(ch+30), cw, ch };

    int i;
    for (i = 0; i < 3; i++) {
        pc[i].src     = srcPiece[i];
        pc[i].pos     = slots[i];
        pc[i].orig    = slots[i];
        pc[i].drag    = 0;
        pc[i].placed  = 0;
        pc[i].correct = (i == correctSlot);
    }

    *hScr   = (SDL_Rect){ BG_X + hCol*cw, BG_Y + hRow*ch, cw, ch };
    *target = *hScr;
}

void on_quit(int *run, int *result)
{
    *result = 0;
    *run    = 0;
}

void on_mouse_down(SDL_MouseButtonEvent *ev,
                   Piece pc[3],
                   int *dIdx, int *ox, int *oy)
{
    int mx = ev->x, my = ev->y, i;
    for (i = 0; i < 3; i++) {
        if (!pc[i].placed &&
            mx >= pc[i].pos.x && mx < pc[i].pos.x + pc[i].pos.w &&
            my >= pc[i].pos.y && my < pc[i].pos.y + pc[i].pos.h) {
            *dIdx       = i;
            *ox         = mx - pc[i].pos.x;
            *oy         = my - pc[i].pos.y;
            pc[i].drag  = 1;
            break;
        }
    }
}

void on_mouse_motion(SDL_MouseMotionEvent *ev,
                     Piece pc[3], int dIdx, int ox, int oy)
{
    if (dIdx >= 0) {
        pc[dIdx].pos.x = ev->x - ox;
        pc[dIdx].pos.y = ev->y - oy;
    }
}

void on_mouse_up(SDL_MouseButtonEvent *ev,
                 Piece pc[3], SDL_Rect target,
                 int *dIdx,
                 int *run, int *result)
{
    (void)ev;
    if (*dIdx < 0) return;

    pc[*dIdx].drag = 0;
    int cx = pc[*dIdx].pos.x + pc[*dIdx].pos.w / 2;
    int cy = pc[*dIdx].pos.y + pc[*dIdx].pos.h / 2;
    int tx = target.x + target.w / 2;
    int ty = target.y + target.h / 2;
    int d  = (int)sqrt((double)((cx-tx)*(cx-tx) + (cy-ty)*(cy-ty)));

    if (d < SNAP) {
        if (pc[*dIdx].correct) {
            pc[*dIdx].pos    = target;
            pc[*dIdx].placed = 1;
            score++;
            *result = 1;
            *run    = 0;
        } else {
            pc[*dIdx].pos = pc[*dIdx].orig;
        }
    }
    *dIdx = -1;
}

void render_frame(SDL_Renderer *rend, SDL_Texture *bg,
                  Piece pc[3], SDL_Rect hScr, Uint32 elapsed)
{
    int cw = BG_W / GRID, ch = BG_H / GRID;
    int r, c, i;

    SDL_SetRenderDrawColor(rend, 30, 30, 30, 255);
    SDL_RenderClear(rend);

    SDL_Rect bgDest = {BG_X, BG_Y, BG_W, BG_H};
    SDL_RenderCopy(rend, bg, NULL, &bgDest);

    SDL_SetRenderDrawColor(rend, 40, 40, 40, 255);
    SDL_RenderFillRect(rend, &hScr);

    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    for (r = 0; r <= GRID; r++)
        SDL_RenderDrawLine(rend, BG_X, BG_Y+r*ch, BG_X+BG_W, BG_Y+r*ch);
    for (c = 0; c <= GRID; c++)
        SDL_RenderDrawLine(rend, BG_X+c*cw, BG_Y, BG_X+c*cw, BG_Y+BG_H);

    for (i = 0; i < 3; i++) {
        SDL_RenderCopy(rend, bg, &pc[i].src, &pc[i].pos);
        SDL_SetRenderDrawColor(rend, 200, 180, 40, 255);
        SDL_RenderDrawRect(rend, &pc[i].pos);
    }

    int bw = (int)((TMAX - (int)elapsed) * W / TMAX);
    if (bw < 0) bw = 0;
    SDL_SetRenderDrawColor(rend, 210, 55, 55, 255);
    SDL_Rect bar = {0, 0, bw, 25};
    SDL_RenderFillRect(rend, &bar);

    SDL_RenderPresent(rend);
}

void rotozoom(SDL_Renderer *rend, SDL_Texture *tex, int win)
{
    Uint32 t0 = SDL_GetTicks();
    double angle = 0;
    SDL_Event e;
    while (SDL_GetTicks() - t0 < 3000) {
        SDL_PollEvent(&e);
        angle += 4;
        float s = 1.0f + 0.28f * (float)sin((SDL_GetTicks() - t0) * 0.006);
        SDL_Rect dst = {
            W/2 - (int)(250*s), H/2 - (int)(55*s),
            (int)(500*s),       (int)(110*s)
        };
        SDL_SetRenderDrawColor(rend, win ? 20:150, win ? 150:20, 20, 255);
        SDL_RenderClear(rend);
        SDL_RenderCopyEx(rend, tex, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
        SDL_RenderPresent(rend);
        SDL_Delay(16);
    }
}

int enigme2(SDL_Renderer *rend, int fate, int *solved)
{
    SDL_Texture *bg, *wTex, *lTex;
    Mix_Chunk   *snd;
    TTF_Font    *fnt;

    if (!load_resources(rend, fate, &bg, &snd, &fnt, &wTex, &lTex))
        return 0;

    Piece    pc[3];
    SDL_Rect slots[3], hScr, target;
    init_puzzle(bg, pc, slots, &hScr, &target);

    SDL_Event ev;
    Uint32 t0     = SDL_GetTicks();
    int    run    = 1;
    int    dIdx   = -1, ox = 0, oy = 0;
    int    result = -1;

    while (run) {
        if (SDL_GetTicks() - t0 >= TMAX) { result = 0; break; }

        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_QUIT:
                    on_quit(&run, &result);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    on_mouse_down(&ev.button, pc, &dIdx, &ox, &oy);
                    break;
                case SDL_MOUSEMOTION:
                    on_mouse_motion(&ev.motion, pc, dIdx, ox, oy);
                    break;
                case SDL_MOUSEBUTTONUP:
                    on_mouse_up(&ev.button, pc, target, &dIdx, &run, &result);
                    break;
            }
        }

        render_frame(rend, bg, pc, hScr, SDL_GetTicks() - t0);
        SDL_Delay(16);
    }

    if (snd) { Mix_PlayChannel(-1, snd, 0); SDL_Delay(400); }
    rotozoom(rend, result == 1 ? wTex : lTex, result == 1);

    *solved = (result == 1);
    free_resources(bg, wTex, lTex, snd, fnt);
    return *solved;
}
