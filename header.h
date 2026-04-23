/* arduino_controller.h — Button definitions, serial protocol, and function declarations */

#ifndef ARDUINO_CONTROLLER_H
#define ARDUINO_CONTROLLER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

/* ── Serial port config ── */
#define SERIAL_PORT      "/dev/ttyUSB0"   /* change to /dev/ttyACM0 if needed */
#define BAUD_RATE        9600

/* ── Button bit-mask protocol (1 byte sent from Arduino each loop) ──
   Arduino sends a single byte where each bit = one button state.
   Bit 0 = LEFT  | Bit 1 = RIGHT | Bit 2 = UP    | Bit 3 = DOWN
   Bit 4 = SPRINT | Bit 5 = JUMP  | Bit 6 = PAUSE | Bit 7 = unused      */
#define BTN_LEFT         (1 << 0)
#define BTN_RIGHT        (1 << 1)
#define BTN_UP           (1 << 2)
#define BTN_DOWN         (1 << 3)
#define BTN_SPRINT       (1 << 4)
#define BTN_JUMP         (1 << 5)
#define BTN_PAUSE        (1 << 6)

/* ── SDL key mappings (what the game already listens to) ── */
#define KEY_MOVE_LEFT    SDLK_LEFT
#define KEY_MOVE_RIGHT   SDLK_RIGHT
#define KEY_MOVE_UP      SDLK_UP
#define KEY_MOVE_DOWN    SDLK_DOWN
#define KEY_SPRINT       SDLK_LSHIFT
#define KEY_JUMP         SDLK_SPACE
#define KEY_PAUSE        SDLK_ESCAPE

/* Quiz answer keys (UP=1, RIGHT=2, DOWN=3) */
#define KEY_QUIZ_OPT1    SDLK_UP
#define KEY_QUIZ_OPT2    SDLK_RIGHT
#define KEY_QUIZ_OPT3    SDLK_DOWN

/* ── Controller state ── */
typedef struct {
    uint8_t current;   /* bitmask this frame  */
    uint8_t previous;  /* bitmask last frame  */
} ControllerState;

/* ── Public API ── */
int  controller_open(const char *port, int baud);   /* open serial, return fd or -1 */
void controller_close(int fd);
bool controller_read(int fd, ControllerState *state); /* read 1 byte, update state   */
void controller_inject_sdl_events(const ControllerState *state); /* push SDL events   */

/* Helpers */
bool button_pressed(const ControllerState *s, uint8_t btn);  /* newly pressed this frame */
bool button_held   (const ControllerState *s, uint8_t btn);  /* held down                */
bool button_released(const ControllerState *s, uint8_t btn); /* just released            */

#endif /* ARDUINO_CONTROLLER_H */
