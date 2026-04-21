#include "game.h"

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    Game g = {0};
    if (!Game_Init(&g)) {
        fprintf(stderr, "Game_Init failed\n");
        return 1;
    }
    Game_Run(&g);
    Game_Shutdown(&g);
    return 0;
}
