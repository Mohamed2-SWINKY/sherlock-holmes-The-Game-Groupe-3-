#include <Arduino.h>
#include "manette.h"

void setup() {
    Serial.begin(9600);
    manette_init();
}

void loop() {
    manette_update();
}
