#include <Arduino.h>
#include "SoundRoutine.h"

void SoundRoutine::playSound(int sound) {
  currentSound = sound;
  reset();
}

int SoundRoutine::runCoroutine() {
  COROUTINE_LOOP() {
    switch (currentSound) {
      case SOUND_NONE:
        Serial.println("<silence>");
        break;

      case SOUND_BEEP:
        // Calls to tone() go here, interspersed with some COROUTINE_DELAYs.
        Serial.println("First BEEP");
        COROUTINE_DELAY(500);
        Serial.println("Second BEEP");
        break;

      case SOUND_BOOP:
        // Calls to tone() go here, interspersed with some COROUTINE_DELAYs.
        Serial.println("First BOOP");
        COROUTINE_DELAY(500);
        Serial.println("Second BOOP");
        break;

      default:
        Serial.println("Unknown sound!");
    }

    currentSound = SOUND_NONE;
    COROUTINE_AWAIT(currentSound != SOUND_NONE);
  }
}
