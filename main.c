/* main.c — Entry point: init SDL, open Arduino serial, run the bridge loop */

#include "arduino_controller.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* ── Link this bridge with your existing game ──────────────────────────────
   Replace the two declarations below with the actual headers from your game.
   The bridge calls game_init() once and game_loop() every frame.           */
extern int  game_init(void);   /* your game's setup function  */
extern void game_loop(void);   /* your game's per-frame logic */
extern void game_quit(void);   /* your game's cleanup         */

int main(int argc, char *argv[])
{
    /* Optional: override serial port via command-line argument */
    const char *port = (argc > 1) ? argv[1] : SERIAL_PORT;

    /* ── Init SDL (your game may already do this; remove if so) ── */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "[main] SDL_Init failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    /* ── Init your game ── */
    if (game_init() != 0) {
        fprintf(stderr, "[main] game_init failed\n");
        SDL_Quit();
        return EXIT_FAILURE;
    }

    /* ── Open Arduino serial port ── */
    int fd = controller_open(port, BAUD_RATE);
    if (fd < 0) {
        fprintf(stderr, "[main] Running without hardware controller (keyboard only)\n");
        /* Not fatal — the game still works with keyboard */
    }

    ControllerState ctrl = { .current = 0, .previous = 0 };
    bool running = true;

    /* ── Main loop ── */
    while (running) {

        /* 1. Read Arduino byte and inject SDL events */
        if (fd >= 0 && controller_read(fd, &ctrl))
            controller_inject_sdl_events(&ctrl);

        /* 2. Handle SDL quit event so the window can be closed normally */
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT)
                running = false;
        }

        /* 3. Run one frame of your game (it does its own SDL_PollEvent inside) */
        game_loop();
    }

    /* ── Cleanup ── */
    controller_close(fd);
    game_quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}
