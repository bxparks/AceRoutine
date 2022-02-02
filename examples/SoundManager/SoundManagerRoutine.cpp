#include <Arduino.h>
#include <AceRoutine.h>
#include "SoundRoutine.h"

using ace_routine::CoroutineScheduler;

extern SoundRoutine soundRoutine;

// The soundManager controls what the soundRoutine will play.
COROUTINE(soundManager) {
  COROUTINE_LOOP() {
    Serial.println("Wait 5 seconds...");
    COROUTINE_DELAY(5000);

    Serial.println();
    Serial.println("Request Beep and wait 5 seconds...");
    CoroutineScheduler::list(Serial);
    soundRoutine.playSound(SOUND_BEEP);
    COROUTINE_DELAY(5000);

    Serial.println();
    Serial.println("Request Boop and wait 5 seconds...");
    CoroutineScheduler::list(Serial);
    soundRoutine.playSound(SOUND_BOOP);
    COROUTINE_DELAY(5000);

    Serial.println();
    Serial.println("Request Silence and wait 5 seconds...");
    CoroutineScheduler::list(Serial);
    soundRoutine.playSound(SOUND_NONE);
  }
}
