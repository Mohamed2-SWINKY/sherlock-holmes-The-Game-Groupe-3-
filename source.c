/* arduino_controller.c — Serial I/O and SDL2 event injection */

#include "arduino_controller.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

/* ── Map a baud-rate integer to a termios constant ── */
static speed_t baud_to_speed(int baud)
{
    switch (baud) {
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
        default:     return B9600;
    }
}

/* Open the serial port and configure it for raw 8N1 comms */
int controller_open(const char *port, int baud)
{
    int fd = open(port, O_RDONLY | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        fprintf(stderr, "[controller] Cannot open %s: %s\n", port, strerror(errno));
        return -1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "[controller] tcgetattr failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    speed_t spd = baud_to_speed(baud);
    cfsetispeed(&tty, spd);
    cfsetospeed(&tty, spd);

    /* 8N1, no flow control, raw mode */
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag &= ~(PARENB | CSTOPB | CRTSCTS);
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_lflag  = 0;   /* raw */
    tty.c_iflag  = 0;
    tty.c_oflag  = 0;
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 0;

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &tty);

    printf("[controller] Opened %s at %d baud\n", port, baud);
    return fd;
}

void controller_close(int fd)
{
    if (fd >= 0) close(fd);
}

/* Read one byte from Arduino; returns true if a byte was available */
bool controller_read(int fd, ControllerState *state)
{
    uint8_t byte;
    ssize_t n = read(fd, &byte, 1);
    if (n != 1) return false;

    state->previous = state->current;
    state->current  = byte;
    return true;
}

/* ── Inject a synthetic SDL key event ── */
static void push_key(SDL_Keycode key, bool pressed)
{
    SDL_Event ev;
    memset(&ev, 0, sizeof ev);
    ev.type           = pressed ? SDL_KEYDOWN : SDL_KEYUP;
    ev.key.state      = pressed ? SDL_PRESSED : SDL_RELEASED;
    ev.key.keysym.sym = key;
    SDL_PushEvent(&ev);
}

/* Convert button bitmask changes into SDL key-down / key-up events.
   Called once per game frame after controller_read(). */
void controller_inject_sdl_events(const ControllerState *state)
{
    /* Table: { button bit, SDL key } */
    static const struct { uint8_t btn; SDL_Keycode key; } map[] = {
        { BTN_LEFT,   KEY_MOVE_LEFT  },
        { BTN_RIGHT,  KEY_MOVE_RIGHT },
        { BTN_UP,     KEY_MOVE_UP    },
        { BTN_DOWN,   KEY_MOVE_DOWN  },
        { BTN_SPRINT, KEY_SPRINT     },
        { BTN_JUMP,   KEY_JUMP       },
        { BTN_PAUSE,  KEY_PAUSE      },
    };

    for (int i = 0; i < (int)(sizeof map / sizeof map[0]); i++) {
        bool was = (state->previous & map[i].btn) != 0;
        bool now = (state->current  & map[i].btn) != 0;

        if (!was && now)  push_key(map[i].key, true);   /* press   */
        if (was  && !now) push_key(map[i].key, false);  /* release */
    }
}

/* ── Button state helpers ── */
bool button_pressed(const ControllerState *s, uint8_t btn)
{
    return !(s->previous & btn) && (s->current & btn);
}

bool button_held(const ControllerState *s, uint8_t btn)
{
    return (s->current & btn) != 0;
}

bool button_released(const ControllerState *s, uint8_t btn)
{
    return (s->previous & btn) && !(s->current & btn);
}
