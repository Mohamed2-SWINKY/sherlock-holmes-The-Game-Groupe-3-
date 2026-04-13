
#include "header.h"

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    Game g;
    if (!game_init(&g)) {
        fprintf(stderr, "game_init failed – exiting.\n");
        return 1;
    }

    int running = 1;
    SDL_Event e;

    while (running) {
        /* --- Event handling --- */
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
                break;
            }
            if (e.type == SDL_KEYDOWN &&
                e.key.keysym.sym == SDLK_ESCAPE) {
                if (g.state == STATE_PLAYING)
                    running = 0;
            }

            switch (g.state) {
                case STATE_GUIDE1:
                case STATE_GUIDE2:
                    input_guide(&g, &e);
                    break;
                case STATE_MODE_MENU:
                    input_mode_menu(&g, &e);
                    break;
                case STATE_PLAYING:
                    input_playing(&g, &e);
                    break;
            }
        }

        /* --- Per-frame update (only while playing) --- */
        if (g.state == STATE_PLAYING) {
            const Uint8 *ks = SDL_GetKeyboardState(NULL);

            if (g.mode == MODE_MONO) {
                /* Single player – arrow keys */
                update_player(&g, &g.p1,
                    ks[SDL_SCANCODE_UP],
                    ks[SDL_SCANCODE_DOWN],
                    ks[SDL_SCANCODE_LEFT],
                    ks[SDL_SCANCODE_RIGHT]);
            } else {
                /* Multi – P1: WASD  |  P2: arrows */
                update_player(&g, &g.p1,
                    ks[SDL_SCANCODE_W],
                    ks[SDL_SCANCODE_S],
                    ks[SDL_SCANCODE_A],
                    ks[SDL_SCANCODE_D]);

                update_player(&g, &g.p2,
                    ks[SDL_SCANCODE_UP],
                    ks[SDL_SCANCODE_DOWN],
                    ks[SDL_SCANCODE_LEFT],
                    ks[SDL_SCANCODE_RIGHT]);
            }

            /* Update falling box (level 1 only) */
            if (g.level == LEVEL_1)
                update_falling_box(&g);
        }

        /* --- Render (single SDL_RenderPresent per frame) --- */
        switch (g.state) {
            case STATE_GUIDE1:
            case STATE_GUIDE2:
                render_guide(&g);
                break;
            case STATE_MODE_MENU:
                render_game(&g);          /* game behind menu  */
                render_mode_menu(&g);     /* overlay on top    */
                break;
            case STATE_PLAYING:
                render_game(&g);
                break;
        }
        SDL_RenderPresent(g.renderer);

        SDL_Delay(16); /* ~60 fps */
    }

    game_cleanup(&g);
    return 0;
}
