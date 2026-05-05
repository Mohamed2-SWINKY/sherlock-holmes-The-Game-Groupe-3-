/**
 * @file main.c
 */
#include "players.h"

int main(void)
{
    GameContext *ctx = game_init();
    game_run(ctx);
    game_cleanup(ctx);
    return 0;
}
