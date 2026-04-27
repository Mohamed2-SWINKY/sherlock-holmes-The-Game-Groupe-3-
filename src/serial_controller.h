#ifndef SERIAL_CONTROLLER_H
#define SERIAL_CONTROLLER_H

#include <stdbool.h>

typedef struct {
    bool up, down, left, right;
    bool action1, action2, action3, action4;
} ControllerState;

int  controller_open(const char* port);
void controller_poll(ControllerState* state);
void controller_close();

#endif
