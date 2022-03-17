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
 * ...
 * @endverbatim
 */

#include <Arduino.h>
#include <AceCommon.h>
#include <AceRoutine.h>
#include "SoundRoutine.h"

using ace_routine::CoroutineScheduler;

EXTERN_COROUTINE(soundManager);
SoundRoutine soundRoutine;

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // needed for Leonardo/Micro

  // Coroutine name can be ether C-string or F-string.
  soundRoutine.setName("soundRoutine"); 
  soundManager.setName(F("soundManager"));

  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
