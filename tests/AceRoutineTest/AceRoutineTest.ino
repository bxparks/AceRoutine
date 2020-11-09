#line 2 "AceRoutineTest.ino"

#include <AceRoutine.h>
#include <AUnitVerbose.h>
#include "ace_routine/testing/TestableCoroutine.h"

using namespace ace_routine;
using namespace ace_routine::testing;
using namespace aunit;

// ---------------------------------------------------------------------------

test(AceRoutineTest, statusStrings) {
  assertEqual(Coroutine::sStatusStrings[Coroutine::kStatusSuspended],
      "Suspended");
  assertEqual(Coroutine::sStatusStrings[Coroutine::kStatusYielding],
      "Yielding");
  assertEqual(Coroutine::sStatusStrings[Coroutine::kStatusDelaying],
      "Delaying");
  assertEqual(Coroutine::sStatusStrings[Coroutine::kStatusRunning],
      "Running");
  assertEqual(Coroutine::sStatusStrings[Coroutine::kStatusEnding],
      "Ending");
  assertEqual(Coroutine::sStatusStrings[Coroutine::kStatusTerminated],
      "Terminated");
}

// ---------------------------------------------------------------------------

// An external flag to await upon, for testing.
bool simpleCoroutineFlag = false;

// A coroutine that yields, delays, then ends.
COROUTINE(TestableCoroutine, simpleCoroutine) {
  COROUTINE_BEGIN();
  COROUTINE_YIELD();
  COROUTINE_DELAY(1);
  COROUTINE_AWAIT(simpleCoroutineFlag);
  COROUTINE_END();
}

// Verify that multiple calls to Coroutine::runCoroutine() after it ends is ok.
test(AceRoutineTest, simpleCoroutine) {
  simpleCoroutine.coroutineMillis(0);
  assertFalse(simpleCoroutine.isSuspended());

  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isYielding());

  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isDelaying());

  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isDelaying());

  simpleCoroutine.coroutineMillis(1);
  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isYielding());

  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isYielding());

  simpleCoroutineFlag = true;
  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isEnding());

  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isEnding());
}

// ---------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  delay(1000); // some boards reboot twice
#endif

  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro
}

void loop() {
  TestRunner::run();
}
