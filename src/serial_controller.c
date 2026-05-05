/**
 * @file serial_controller.c
 */

// Linux only (including Ubuntu VM on Windows)
#include "serial_controller.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

static int fd = -1;
static char buffer[64];
static int  buf_len = 0;

int controller_open(const char* port) {
    fd = open(port, O_RDONLY | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) return -1;

    struct termios tty;
    tcgetattr(fd, &tty);
    cfsetispeed(&tty, B9600);
    tty.c_cflag = CS8 | CREAD | CLOCAL;
    tty.c_iflag = IGNPAR;
    tty.c_oflag = 0;
    tty.c_lflag = 0;
    tcsetattr(fd, TCSANOW, &tty);

    return 0;
}

void controller_poll(ControllerState* state) {
    if (fd < 0) return;

    char c;
    while (read(fd, &c, 1) == 1) {
        if (c == '\n') {
            buffer[buf_len] = '\0';
            if (buf_len > 0 && buffer[buf_len-1] == '\r')
                buffer[--buf_len] = '\0';

            if      (strcmp(buffer, "UP")      == 0) state->up      = true;
            else if (strcmp(buffer, "DOWN")    == 0) state->down    = true;
            else if (strcmp(buffer, "LEFT")    == 0) state->left    = true;
            else if (strcmp(buffer, "RIGHT")   == 0) state->right   = true;
            else if (strcmp(buffer, "ACTION1") == 0) state->action1 = true;
            else if (strcmp(buffer, "ACTION2") == 0) state->action2 = true;
            else if (strcmp(buffer, "ACTION3") == 0) state->action3 = true;
            else if (strcmp(buffer, "ACTION4") == 0) state->action4 = true;

            buf_len = 0;
        } else {
            if (buf_len < 63) buffer[buf_len++] = c;
        }
    }
}

void controller_close() {
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}
