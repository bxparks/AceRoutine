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
 * name          <8us <16us <32us <64us<128us<256us<512us  <1ms  <2ms    >>
 * soundManager  1440     0     1     0     0     0     0     0     1     0
 * 0x55C5AB3CE2  1438     1     2     0     0     0     0     0     0     1
 * soundRoutine  1440     0     0     0     2     0     0     0     0     0
 * {
 * "soundManager":[1440,0,1,0,0,0,0,0,1,0],
 * "0x55C5AB3CE240":[1438,1,2,0,0,0,0,0,0,1],
 * "soundRoutine":[1440,0,0,0,2,0,0,0,0,0]
 * }
 * 
 * Request Boop and wait 5 seconds...
 * Coroutine soundRoutine; status: Yielding
 * Coroutine soundManager; status: Running
 * First BOOP
 * Second BOOP
 * name          <8us <16us <32us <64us<128us<256us<512us  <1ms  <2ms    >>
 * soundManager  1440     0     1     0     0     0     0     0     1     0
 * 0x55C5AB3CE2  1438     1     2     0     0     0     0     0     0     1
 * soundRoutine  1440     0     0     0     2     0     0     0     0     0
 * {
 * "soundManager":[1440,0,1,0,0,0,0,0,1,0],
 * "0x55C5AB3CE240":[1438,1,2,0,0,0,0,0,0,1],
 * "soundRoutine":[1440,0,0,0,2,0,0,0,0,0]
 * }
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

using ace_routine::Coroutine;;
using ace_routine::CoroutineScheduler;
using ace_routine::LogBinProfiler;
using ace_routine::LogBinTableRenderer;
using ace_routine::LogBinJsonRenderer;

SoundRoutine soundRoutine;
EXTERN_COROUTINE(soundManager);

// Every 5 seconds, print out the elapsed time frequency distribution from the
// LogBinProfiler.
COROUTINE(printProfiler) {
  COROUTINE_LOOP() {
    LogBinTableRenderer tableRenderer(getRoot());
    tableRenderer.printTo(Serial, 2, 12, false /*clear*/);

    LogBinJsonRenderer jsonRenderer(getRoot());
    jsonRenderer.printTo(Serial, 2, 12);

    COROUTINE_DELAY_SECONDS(5);
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // needed for Leonardo/Micro

  // Set names using both c-string and f-string for testing purposes.
  soundRoutine.setCName("soundRoutine");
  soundManager.setFName(F("soundManager"));

  // Don't set the name of 'printProfiler' to verify that the name of the
  // coroutine becomes the pointer address in hexadecimal.
  // printProfiler.setFName(F("printProfiler"));

  // Attach profilers to all coroutines.
  LogBinProfiler::createProfilers(Coroutine::getRoot());

  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
