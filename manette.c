#include <Arduino.h>
#include "manette.h"

// Définition des pins avec noms explicites
const int BTN_UP      = 10;
const int BTN_DOWN    = 12;
const int BTN_LEFT    = 13;
const int BTN_RIGHT   = 11;
const int BTN_ACTION1 = 9;
const int BTN_ACTION2 = 8;
const int BTN_ACTION3 = 7;
const int BTN_ACTION4 = 6;

// Tableau des pins et étiquettes
const int PINS[]     = {BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT,
                        BTN_ACTION1, BTN_ACTION2, BTN_ACTION3, BTN_ACTION4};
const char* LABELS[] = {"UP", "DOWN", "LEFT", "RIGHT",
                        "ACTION1", "ACTION2", "ACTION3", "ACTION4"};
const int NB_BUTTONS = 8;

// Délais de répétition (en millisecondes)
const unsigned long DELAY_INITIAL = 300; // délai avant la première répétition
const unsigned long DELAY_REPEAT  = 100; // délai entre chaque répétition

// État de chaque bouton
bool          lastState[8];
unsigned long pressTime[8];   // moment où le bouton a été enfoncé
bool          repeated[8];    // true = phase de répétition activée

void manette_init() {
    for (int i = 0; i < NB_BUTTONS; i++) {
        pinMode(PINS[i], INPUT_PULLUP);
        lastState[i] = HIGH;
        pressTime[i] = 0;
        repeated[i]  = false;
    }
}

void manette_update() {
    unsigned long now = millis();

    for (int i = 0; i < NB_BUTTONS; i++) {
        bool currentState = digitalRead(PINS[i]);

        if (currentState == LOW) {
            if (lastState[i] == HIGH) {
                // --- Première pression ---
                Serial.println(LABELS[i]);
                pressTime[i] = now;
                repeated[i]  = false;

            } else {
                // --- Bouton maintenu enfoncé ---
                unsigned long held = now - pressTime[i];

                if (!repeated[i] && held >= DELAY_INITIAL) {
                    // Début de la phase de répétition
                    Serial.println(LABELS[i]);
                    pressTime[i] = now; // réinitialise pour le rythme de répétition
                    repeated[i]  = true;

                } else if (repeated[i] && held >= DELAY_REPEAT) {
                    // Répétition continue
                    Serial.println(LABELS[i]);
                    pressTime[i] = now;
                }
            }
        } else {
            // Bouton relâché → réinitialisation
            repeated[i] = false;
        }

        lastState[i] = currentState;
    }
}
