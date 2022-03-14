/*
 * An example of using the Coroutine::reset() from one coroutine to control the
 * state of another coroutine. The SoundRoutine plays different sounds
 * depending on the 'currentSound' variable. The soundManager coroutine is able
 * to interrupt and reset the SoundRoutine to play a different sound.
 *
 * Adapted from the example posted by EkardNT@ on
 * https://github.com/bxparks/AceRoutine/issues/20.
 *
 * This example also demonstrates how coroutines can be defined in separate
 * *.cpp and *.h files. See https://github.com/bxparks/AceRoutine/issues/39.
 *
 * The output of this program should be the following:
 *
 * @verbatim
 * <silence>
 * Wait 5 seconds...
 * 
 * Request Beep and wait 5 seconds...
 * Coroutine soundRoutine; status: Yielding
 * Coroutine soundManager; status: Running
 * First BEEP
 * Second BEEP
 * 
 * Request Boop and wait 5 seconds...
 * Coroutine soundRoutine; status: Yielding
 * Coroutine soundManager; status: Running
 * First BOOP
 * Second BOOP
 * 
 * Request Silence and wait 5 seconds...
 * Coroutine soundRoutine; status: Yielding
 * Coroutine soundManager; status: Running
 * Wait 5 seconds...
 * <silence>
 * 
 * Request Beep and wait 5 seconds...
 * Coroutine soundRoutine; status: Yielding
 * Coroutine soundManager; status: Running
 * First BEEP
 * Second BEEP
 *
 * ...
 * @endverbatim
 */

#include <Arduino.h>
#include <AceRoutine.h>
#include "SoundRoutine.h"

using ace_routine::CoroutineScheduler;

SoundRoutine soundRoutine;
EXTERN_COROUTINE(soundManager);

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // needed for Leonardo/Micro

  // Set names using both c-string and f-string for testing purposes.
  soundRoutine.setCName("soundRoutine");
  soundManager.setFName(F("soundManager"));

  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
