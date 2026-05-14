/**
 * @file map.c
 */
 
#include "map.h"
#include "players.h"

int map_rects_overlap(SDL_Rect a, SDL_Rect b)
{
    return (a.x < b.x + b.w && a.x + a.w > b.x &&
            a.y < b.y + b.h && a.y + a.h > b.y);
}

void map_init(MapData *m, SDL_Renderer *renderer)
{
    // Updated paths to include the /map/ subdirectory
    m->door1_open[0]  = 0; // ensure initialized
    m->boxFallSound   = NULL;

    m->tex_map1       = IMG_LoadTexture(renderer, "assets/map/background/background1.png");
    m->tex_map2       = IMG_LoadTexture(renderer, "assets/map/background/background2.png");
    m->tex_box        = IMG_LoadTexture(renderer, "assets/map/box.png");
    m->tex_broken_box = IMG_LoadTexture(renderer, "assets/map/broken box.png");
    
    m->boxFallSound = Mix_LoadWAV("assets/sounds/box_fall.mp3");
    if (!m->boxFallSound) {
        printf("Erreur sound box_fall.mp3: %s\n", Mix_GetError());
    } else {
        printf("Box fall sound loaded successfully.\n");
    }

    m->level          = LEVEL_1;
    setup_level1(m);
}

void map_cleanup(MapData *m)
{
    if (m->tex_map1)       SDL_DestroyTexture(m->tex_map1);
    if (m->tex_map2)       SDL_DestroyTexture(m->tex_map2);
    if (m->tex_box)        SDL_DestroyTexture(m->tex_box);
    if (m->tex_broken_box) SDL_DestroyTexture(m->tex_broken_box);
    if (m->boxFallSound)   Mix_FreeChunk(m->boxFallSound);
}

#define ADD_OBS1(X,Y,W,H) m->obs1[m->obs1_cnt++] = (SDL_Rect){X,Y,W,H}
#define ADD_OBS2(X,Y,W,H) m->obs2[m->obs2_cnt++] = (SDL_Rect){X,Y,W,H}

void setup_level1(MapData *m)
{
    m->obs1_cnt   = 0;
    m->doors1_cnt = 0;
    memset(m->door1_open, 0, sizeof(m->door1_open));

    /* ── OUTER MAP BOUNDARY ──────────────────────────────────────────── */
    ADD_OBS1(   0,   0,1400,   9);  /* border top    */
    ADD_OBS1(   0, 732,1400,   9);  /* border bottom */
    ADD_OBS1(   0,   0,   9, 742);  /* border left   */
    ADD_OBS1(1390,   0,   9, 742);  /* border right  */

    /* ── HOUSE PERIMETER ─────────────────────────────────────────────── */
    /* Top wall — gap at x 556-601 for balcony door */
    ADD_OBS1( 374, 213, 500,  70);  /* top left of gap  */
    /* Left wall — gap at y 364-439 for outdoor entry */
    ADD_OBS1( 350, 200,  30, 180);  /* left wall top    */
    ADD_OBS1( 350, 470,  30, 261);  /* left wall bottom */
    /* Right wall — solid */
    ADD_OBS1( 894, 213,  11, 300);
    ADD_OBS1( 380, 660,  560, 30);
    ADD_OBS1( 990, 660,  50, 30);
    ADD_OBS1( 380, 360,  90, 10);
    ADD_OBS1( 480, 430,  100, 10);
    ADD_OBS1( 640, 500,  30, 10);
    ADD_OBS1( 640, 620,  60, 30);
    ADD_OBS1( 381, 550,  30, 70);
    ADD_OBS1( 381, 590,  180, 30);
    ADD_OBS1( 1280, 390, 180, 110);
    ADD_OBS1( 1030, 390, 160, 110);
    ADD_OBS1( 1030, 200, 50, 200);
    ADD_OBS1( 1090, 350, 40, 10);
    ADD_OBS1( 1090, 300, 60, 10);
    ADD_OBS1( 1300, 280, 30, 10);
    ADD_OBS1( 1320, 540, 10, 100);
    ADD_OBS1( 1140, 520, 40, 10);
    ADD_OBS1( 1020, 620, 100, 30);
    ADD_OBS1( 1150, 660, 30, 30);
    ADD_OBS1( 670, 70, 10, 100);
    ADD_OBS1( 670, 70, 500, 10);
    ADD_OBS1( 1000, 70, 30, 100);
    ADD_OBS1( 880, 620, 30, 30);
    ADD_OBS1( 10, 500, 60, 30);
    ADD_OBS1( 210, 500, 140, 30);
    ADD_OBS1( 10, 200, 360, 30);

    /* ── DUNGEON OUTER WALLS ─────────────────────────────────────────── */
    ADD_OBS1(850, 213, 80,  30);  /* dungeon top    */
    ADD_OBS1(1000, 213, 400,  30);  /* dungeon top    */
    ADD_OBS1(1350, 213,  30, 487);  /* dungeon right  */
    ADD_OBS1(1080, 660,  70, 30);  /* left of gap  */
    ADD_OBS1(1204, 660, 211, 30);  /* right of gap */

    /* ── INDOOR VERTICAL DIVIDER (left / right room split) ──────────── */
    /* Gap at y 316-379 for passage */
    ADD_OBS1( 670, 213,  30, 300);  /* divider top    */

    /* ── INDOOR HORIZONTAL DIVIDER (right rooms upper/lower) ─────────── */
    /* Gap at x 771-845 for passage */
    ADD_OBS1( 700, 434, 180,  70);  /* horiz left  */

    /* Door 0: passage door — place in open floor area */
    m->doors1[0].rect     = (SDL_Rect){ 930, 210, 44, 18 };
    m->doors1[0].locked   = 1;
    m->doors1[0].key_id   = 0;
    m->doors1[0].to_level = -1;

    /* Door 1: exit to Level 2 — place in the dungeon gap you made */
    m->doors1[1].rect     = (SDL_Rect){ 950, 680, 54, 16 };
    m->doors1[1].locked   = 1;
    m->doors1[1].key_id   = 1;
    m->doors1[1].to_level = LEVEL_2;
    
    m->doors1_cnt = 2;
    


    /* ── KEYS ── */
    m->keys1_cnt = 2;

    m->keys1[0].rect      = (SDL_Rect){ 1200, 380, 24, 24 };
    m->keys1[0].id        = 0;
    m->keys1[0].visible   = 0;
    m->keys1[0].collected = 0;

    m->keys1[1].rect      = (SDL_Rect){672, 100, 24, 24 };
    m->keys1[1].id        = 1;
    m->keys1[1].visible   = 1;
    m->keys1[1].collected = 0;

    /* ── FALLING BOX ─────────────────────────────────────────────── */
    m->fbox.rect  = (SDL_Rect){ 1200, 230, 50, 50 };
    m->fbox.fy    = (float) 230;
    m->fbox.vel   = 0.0f;
    m->fbox.state = BOX_IDLE;
}

void setup_level2(MapData *m)
{
    m->obs2_cnt   = 0;
    m->doors2_cnt = 0;
    m->level      = LEVEL_2;
    memset(m->door2_open, 0, sizeof(m->door2_open));

    ADD_OBS2(   0,   0,1400,   9);  /* border top    */
    ADD_OBS2(   0, 700,1400,   9);  /* border bottom */
    ADD_OBS2(   33,   0,   9, 742);  /* border left   */
    ADD_OBS2(1360,   0,   9, 742);  /* border right  */
    ADD_OBS2(43,   490,   330, 10);
    ADD_OBS2(43,   560,   100, 10);
    ADD_OBS2(43,   660,   100, 10);
    ADD_OBS2(272,   560,   100, 10);
    ADD_OBS2(360,   540,   10, 50);
    ADD_OBS2(373,   570,   280, 10);
    ADD_OBS2(543,   670,   300, 30);
    ADD_OBS2(845,   540,   600, 10);
    ADD_OBS2(730,   570,   120, 10);
    ADD_OBS2(650,   420,   10, 150);
    ADD_OBS2(730,   480,   10, 100);
    ADD_OBS2(730,   480,   110, 10);
    ADD_OBS2(840,   485,   600, 10);
    ADD_OBS2(735,   390,   600, 10);
    ADD_OBS2(730,   255,   10, 140);
    ADD_OBS2(650,   255,   10, 80);
    ADD_OBS2(400,   330,  260, 10);
    ADD_OBS2(45,   320,  350, 10);
    ADD_OBS2(45,   440,  350, 10);
    ADD_OBS2(405,   430,  60, 10);
    ADD_OBS2(450,   440,   10, 140);
    ADD_OBS2(540,   417,   10, 100);
    ADD_OBS2(550,   420,   100, 10);
    ADD_OBS2(740,   255,   90, 10);
    ADD_OBS2(830,   230,   130, 10);
    ADD_OBS2(820,   230,   10, 30);
    ADD_OBS2(950,   240,   10, 90);
    ADD_OBS2(960,   320,   400, 10);
    ADD_OBS2(960,   80,   400, 10);
    ADD_OBS2(830,   130,   130, 10);
    ADD_OBS2(960,   90,   10, 30);
    ADD_OBS2(560,   96,   260, 10);
    ADD_OBS2(400,   137,   160, 10);
    ADD_OBS2(400,   225,   160, 10);
    ADD_OBS2(550,   258,   100, 10);
    ADD_OBS2(40,   82,   340, 10);
    ADD_OBS2(40,   250,   340, 10);


 /* Door 0: passage door — place in open floor area */
    m->doors2[0].rect     = (SDL_Rect){ 370, 590, 10, 60 };
    m->doors2[0].locked   = 1;
    m->doors2[0].key_id   = 0;
    m->doors2[0].to_level = -1;
    
    m->doors2_cnt = 1;
    


    /* ── KEYS ── */
    m->keys2_cnt = 1;

    m->keys2[0].rect      = (SDL_Rect){ 400, 640, 24, 24 };
    m->keys2[0].id        = 0;
    m->keys2[0].visible   = 1;
    m->keys2[0].collected = 0;

}

void update_falling_box(MapData *m, int p1x, int p1y, int p2x, int p2y)
{
    FallingBox *fb = &m->fbox;
    if (fb->state == BOX_BROKEN) return;

    if (fb->state == BOX_IDLE) {
        int dx1 = p1x - fb->rect.x;
        int dy1 = p1y - fb->rect.y;
        int dx2 = p2x - fb->rect.x;
        int dy2 = p2y - fb->rect.y;
        float d1 = sqrtf((float)(dx1*dx1 + dy1*dy1));
        float d2 = sqrtf((float)(dx2*dx2 + dy2*dy2));
        if (d1 < 130 || d2 < 130)
            fb->state = BOX_FALLING;
        return;
    }

    fb->vel += 0.5f;
    if (fb->vel > 4.5f) fb->vel = 4.5f;
    fb->fy += fb->vel;
    fb->rect.y = (int)fb->fy;

    if (fb->rect.y >= 358) {
        fb->rect.y = 358;
        fb->fy     = 358.0f;
        fb->state  = BOX_BROKEN;
        if (m->boxFallSound) {
            Mix_PlayChannel(-1, m->boxFallSound, 0);
        }
        /* reveal the key hidden inside the box (key 0 in level 1) */
        m->keys1[0].visible = 1;
    }
}

const SDL_Color TB_OR     = { 255, 215,   0, 255 };
const SDL_Color TB_BLANC  = { 230, 230, 230, 255 };
const SDL_Color TB_GRIS   = { 140, 140, 140, 255 };
const SDL_Color TB_ROUGE  = { 210,  55,  55, 255 };
const SDL_Color TB_BLEU   = {  55, 120, 210, 255 };
const SDL_Color TB_VERT   = {  55, 190,  90, 255 };
const SDL_Color TB_ARGENT = { 192, 192, 192, 255 };
const SDL_Color TB_BRONZE = { 205, 127,  50, 255 };

void tb_fillRect(SDL_Renderer *r, SDL_Rect rect, SDL_Color c)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r, &rect);
}

void tb_drawRect(SDL_Renderer *r, SDL_Rect rect, SDL_Color c)
{
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderDrawRect(r, &rect);
}

void tb_hline(SDL_Renderer *r, int x1, int x2, int y, SDL_Color c)
{
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderDrawLine(r, x1, y, x2, y);
}

void tb_texte(SDL_Renderer *r, TTF_Font *font,
                     const char *txt, SDL_Color c,
                     int x, int y, int centrerH)
{
    if (!font || !txt || !txt[0]) return;
    SDL_Surface *s = TTF_RenderUTF8_Blended(font, txt, c);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    if (!t) { SDL_FreeSurface(s); return; }
    SDL_Rect dst;
    dst.w = s->w;
    dst.h = s->h;
    dst.x = centrerH ? (WINDOW_WIDTH - dst.w) / 2 : x;
    dst.y = y;
    SDL_RenderCopy(r, t, NULL, &dst);
    SDL_DestroyTexture(t);
    SDL_FreeSurface(s);
}

void tb_fond(SDL_Renderer *r)
{
    for (int y = 0; y < WINDOW_HEIGHT; y++) {
        Uint8 rv = (Uint8)(10 + 20 * y / WINDOW_HEIGHT);
        Uint8 gv = (Uint8)(10 + 10 * y / WINDOW_HEIGHT);
        Uint8 bv = (Uint8)(20 + 40 * y / WINDOW_HEIGHT);
        SDL_SetRenderDrawColor(r, rv, gv, bv, 255);
        SDL_RenderDrawLine(r, 0, y, WINDOW_WIDTH, y);
    }
}

void tb_charger(ClassementTB *cl)
{
    cl->nb = 0;
    FILE *f = fopen(SCORE_FILE, "r");
    if (!f) {
        f = fopen(SCORE_FILE, "w");
        if (f) fclose(f);
        return;
    }
    while (cl->nb < MAX_SCORES_TB) {
        EntreeTB *e = &cl->entrees[cl->nb];
        if (fscanf(f, "%49s %d %d", e->nom, &e->score, &e->joueur) != 3)
            break;
        cl->nb++;
    }
    fclose(f);
    tb_trier(cl);
}

void tb_sauvegarder(const ClassementTB *cl)
{
    FILE *f = fopen(SCORE_FILE, "w");
    if (!f) return;
    for (int i = 0; i < cl->nb; i++)
        fprintf(f, "%s %d %d\n",
                cl->entrees[i].nom,
                cl->entrees[i].score,
                cl->entrees[i].joueur);
    fclose(f);
}

void tb_trier(ClassementTB *cl)
{
    for (int i = 1; i < cl->nb; i++) {
        EntreeTB cle = cl->entrees[i];
        int j = i - 1;
        while (j >= 0 && cl->entrees[j].score < cle.score) {
            cl->entrees[j + 1] = cl->entrees[j];
            j--;
        }
        cl->entrees[j + 1] = cle;
    }
}

int tb_inserer(ClassementTB *cl, const char *nom, int score, int joueur)
{
    if (!nom || nom[0] == '\0') return 0;
    if (cl->nb >= MAX_SCORES_TB &&
        score <= cl->entrees[MAX_SCORES_TB - 1].score)
        return 0;
    int idx = (cl->nb < MAX_SCORES_TB) ? cl->nb : MAX_SCORES_TB - 1;
    strncpy(cl->entrees[idx].nom, nom, MAX_NOM_TB - 1);
    cl->entrees[idx].nom[MAX_NOM_TB - 1] = '\0';
    cl->entrees[idx].score  = score;
    cl->entrees[idx].joueur = joueur;
    if (cl->nb < MAX_SCORES_TB) cl->nb++;
    tb_trier(cl);
    return 1;
}

int tb_saisir_nom(GameContext *ctx, char *nomSortie, const char *promptTitle)
{
    char  buffer[MAX_NOM_TB] = "";
    int   longueur  = 0;
    int   valide    = 0;
    int   running   = 1;

    SDL_StartTextInput();

    Uint32 dernierCligno  = SDL_GetTicks();
    int    curseurVisible = 1;

    SDL_Renderer *r    = ctx->renderer;
    TTF_Font     *font = ctx->font;

    SDL_Event ev;

    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                running = 0; valide = 0;
            } else if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = 0; valide = 0;
                        break;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        if (longueur > 0) { running = 0; valide = 1; }
                        break;
                    case SDLK_BACKSPACE:
                        if (longueur > 0) buffer[--longueur] = '\0';
                        break;
                    default: break;
                }
            } else if (ev.type == SDL_TEXTINPUT) {
                int ajout = (int)strlen(ev.text.text);
                if (longueur + ajout < MAX_NOM_TB - 1) {
                    strncat(buffer, ev.text.text,
                            (size_t)(MAX_NOM_TB - longueur - 1));
                    longueur += ajout;
                }
            } else if (ev.type == SDL_MOUSEBUTTONDOWN &&
                       ev.button.button == SDL_BUTTON_LEFT) {
                SDL_Rect btnOk = { (WINDOW_WIDTH - 200) / 2, 380, 200, 50 };
                int mx = ev.button.x, my = ev.button.y;
                if (mx >= btnOk.x && mx <= btnOk.x + btnOk.w &&
                    my >= btnOk.y && my <= btnOk.y + btnOk.h &&
                    longueur > 0) {
                    running = 0; valide = 1;
                }
            }
        }

        if (SDL_GetTicks() - dernierCligno >= 500) {
            curseurVisible = !curseurVisible;
            dernierCligno  = SDL_GetTicks();
        }

        tb_fond(r);
        tb_texte(r, font, promptTitle, TB_OR, 0, 80, 1);
        tb_hline(r, 150, WINDOW_WIDTH - 150, 120, TB_OR);
        tb_texte(r, font, "Votre score sera sauvegarde dans " SCORE_FILE,
                 TB_GRIS, 0, 140, 1);

        SDL_Rect boite = { (WINDOW_WIDTH - 400) / 2, 220, 400, 55 };
        tb_fillRect(r, boite, (SDL_Color){30, 30, 60, 200});
        tb_drawRect(r, boite, TB_BLEU);

        char affiche[MAX_NOM_TB + 2];
        snprintf(affiche, sizeof(affiche), "%s%s",
                 buffer, curseurVisible ? "|" : " ");
        tb_texte(r, font, affiche, TB_BLEU, boite.x + 14, boite.y + 14, 0);

        tb_texte(r, font, "Entree : valider   |   Echap : passer",
                 TB_GRIS, 0, 310, 1);

        int mx2, my2;
        SDL_GetMouseState(&mx2, &my2);
        SDL_Rect btnOk = { (WINDOW_WIDTH - 200) / 2, 380, 200, 50 };
        int survol = (mx2 >= btnOk.x && mx2 <= btnOk.x + btnOk.w &&
                      my2 >= btnOk.y && my2 <= btnOk.y + btnOk.h);
        SDL_Color cBtn = survol ? TB_VERT : (SDL_Color){40, 150, 70, 255};
        tb_fillRect(r, btnOk, cBtn);
        tb_drawRect(r, btnOk, TB_BLANC);
        tb_texte(r, font, "VALIDER", TB_BLANC, btnOk.x + 60, btnOk.y + 13, 0);

        SDL_RenderPresent(r);
        SDL_Delay(16);
    }

    SDL_StopTextInput();

    if (valide) {
        strncpy(nomSortie, buffer, MAX_NOM_TB - 1);
        nomSortie[MAX_NOM_TB - 1] = '\0';
    }
    return valide;
}

void tb_afficher_classement(GameContext *ctx, const ClassementTB *cl)
{
    SDL_Renderer *r    = ctx->renderer;
    TTF_Font     *font = ctx->font;
    int      running = 1;
    SDL_Event ev;

    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT      ||
                ev.type == SDL_KEYDOWN   ||
                ev.type == SDL_MOUSEBUTTONDOWN)
                running = 0;
        }

        tb_fond(r);
        tb_texte(r, font, "MEILLEURS SCORES", TB_OR, 0, 28, 1);
        tb_hline(r, 80, WINDOW_WIDTH - 80, 68, TB_OR);

        tb_texte(r, font, "RANG",   TB_BLEU,  70,  80, 0);
        tb_texte(r, font, "NOM",    TB_BLEU, 170,  80, 0);
        tb_texte(r, font, "JOUEUR", TB_BLEU, 560,  80, 0);
        tb_texte(r, font, "SCORE",  TB_BLEU, 720,  80, 0);
        tb_hline(r, 60, WINDOW_WIDTH - 60, 106, TB_GRIS);

        if (cl->nb == 0)
            tb_texte(r, font, "Aucun score enregistre.", TB_GRIS, 0, 200, 1);

        for (int i = 0; i < cl->nb; i++) {
            int y = 115 + i * 44;

            if (i % 2 == 0) {
                SDL_Rect fond = { 55, y - 4, WINDOW_WIDTH - 110, 40 };
                tb_fillRect(r, fond, (SDL_Color){255, 255, 255, 12});
            }

            SDL_Color cLigne;
            if      (i == 0) cLigne = TB_OR;
            else if (i == 1) cLigne = TB_ARGENT;
            else if (i == 2) cLigne = TB_BRONZE;
            else             cLigne = TB_BLANC;

            char sRang[8];
            snprintf(sRang, sizeof(sRang), "%2d.", i + 1);
            tb_texte(r, font, sRang, cLigne, 72, y, 0);
            tb_texte(r, font, cl->entrees[i].nom, cLigne, 170, y, 0);

            char sJoueur[8];
            snprintf(sJoueur, sizeof(sJoueur), "P%d", cl->entrees[i].joueur);
            SDL_Color cJ = (cl->entrees[i].joueur == 1) ? TB_ROUGE : TB_BLEU;
            tb_texte(r, font, sJoueur, cJ, 580, y, 0);

            char sScore[16];
            snprintf(sScore, sizeof(sScore), "%d", cl->entrees[i].score);
            tb_texte(r, font, sScore, cLigne, 720, y, 0);
        }

        tb_hline(r, 60, WINDOW_WIDTH - 60, WINDOW_HEIGHT - 55, TB_GRIS);
        tb_texte(r, font, "Appuyez sur une touche pour continuer...",
                 TB_GRIS, 0, WINDOW_HEIGHT - 42, 1);

        SDL_RenderPresent(r);
        SDL_Delay(16);
    }
}

void afficherSousMenuScores(GameContext *ctx)
{
    ClassementTB cl;
    tb_charger(&cl);

    char nomP1[MAX_NOM_TB] = "";
    int valideP1 = tb_saisir_nom(ctx, nomP1, "PLAYER 1 : ENTREZ VOTRE NOM");
    if (valideP1)
        tb_inserer(&cl, nomP1, ctx->player1.score, 1);

    char nomP2[MAX_NOM_TB] = "";
    int valideP2 = tb_saisir_nom(ctx, nomP2, "PLAYER 2 : ENTREZ VOTRE NOM");
    if (valideP2)
        tb_inserer(&cl, nomP2, ctx->player2.score, 2);

    tb_sauvegarder(&cl);
    tb_afficher_classement(ctx, &cl);
}
