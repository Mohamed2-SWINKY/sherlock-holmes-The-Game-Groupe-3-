#include "header.h"    // menu
#include "players.h"   // game

int main(void)
{
    /* ── Phase 1: Menu ── */
    MenuContext *menu = menu_init();
    menu_run(menu);
    
    menu_cleanup(menu);                    // destroys menu window/renderer

    /* ── Phase 2: Game ── */
    GameContext *game = game_init();

    game_run(game);
    game_cleanup(game);

    return 0;
}