#line 2 "AceRoutineTest.ino"

#include <AceRoutine.h>
#include <AUnitVerbose.h>
#include "ace_routine/testing/TestableCoroutine.h"
#include "ace_routine/testing/TestableClockInterface.h"

using namespace ace_routine;
using namespace ace_routine::testing;
using namespace aunit;

// ---------------------------------------------------------------------------

test(AceRoutineTest, statusStrings) {
  assertEqual(sStatusStrings[Coroutine::kStatusSuspended], "Suspended");
  assertEqual(sStatusStrings[Coroutine::kStatusYielding], "Yielding");
  assertEqual(sStatusStrings[Coroutine::kStatusDelaying], "Delaying");
  assertEqual(sStatusStrings[Coroutine::kStatusRunning], "Running");
  assertEqual(sStatusStrings[Coroutine::kStatusEnding], "Ending");
  assertEqual(sStatusStrings[Coroutine::kStatusTerminated], "Terminated");
}

// ---------------------------------------------------------------------------

// An external flag to await upon, for testing.
bool simpleCoroutineFlag = false;

// A coroutine that yields, delays for a millisecond, waits for flag, then ends.
COROUTINE(TestableCoroutine, simpleCoroutine) {
  COROUTINE_BEGIN();
  COROUTINE_YIELD();
  COROUTINE_DELAY(1);
  COROUTINE_AWAIT(simpleCoroutineFlag);
  COROUTINE_END();
}

// Verify a simple coroutine that uses COROUTINE_DELAY() in milliseconds.
test(AceRoutineTest, simpleCoroutine) {
  simpleCoroutineFlag = false;

  TestableClockInterface::setMillis(0);
  assertFalse(simpleCoroutine.isSuspended());

  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isYielding());

  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isDelaying());

  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isDelaying());

  TestableClockInterface::setMillis(1);
  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isYielding());

  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isYielding());

  simpleCoroutineFlag = true;
  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isEnding());

  // Verify that multiple calls to Coroutine::runCoroutine() after it ends is
  // ok.
  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isEnding());
}

// ---------------------------------------------------------------------------

bool microCoroutineFlag = false;

// A coroutine that yields, delays for a microsecond, waits for flag, then ends.
COROUTINE(TestableCoroutine, microCoroutine) {
  COROUTINE_BEGIN();
  COROUTINE_YIELD();
  COROUTINE_DELAY_MICROS(1);
  COROUTINE_AWAIT(microCoroutineFlag);
  COROUTINE_END();
}

// Verify COROUTINE_DELAY_MICROS().
test(AceRoutineTest, microCoroutine) {
  microCoroutineFlag = false;

  TestableClockInterface::setMicros(0);
  assertFalse(microCoroutine.isSuspended());

  microCoroutine.runCoroutine();
  assertTrue(microCoroutine.isYielding());

  microCoroutine.runCoroutine();
  assertTrue(microCoroutine.isDelaying());

  microCoroutine.runCoroutine();
  assertTrue(microCoroutine.isDelaying());

  TestableClockInterface::setMicros(1);
  microCoroutine.runCoroutine();
  assertTrue(microCoroutine.isYielding());

  microCoroutine.runCoroutine();
  assertTrue(microCoroutine.isYielding());

  microCoroutineFlag = true;
  microCoroutine.runCoroutine();
  assertTrue(microCoroutine.isEnding());

  microCoroutine.runCoroutine();
  assertTrue(microCoroutine.isEnding());
}

// ---------------------------------------------------------------------------

bool macroCoroutineFlag = false;

// A coroutine that yields, delays for a second, waits for flag, then ends.
COROUTINE(TestableCoroutine, macroCoroutine) {
  COROUTINE_BEGIN();
  COROUTINE_YIELD();
  COROUTINE_DELAY_SECONDS(1);
  COROUTINE_AWAIT(macroCoroutineFlag);
  COROUTINE_END();
}

// Verify COROUTINE_DELAY_SECONDS().
test(AceRoutineTest, macroCoroutine) {
  macroCoroutineFlag = false;

  TestableClockInterface::setMicros(0);
  assertFalse(macroCoroutine.isSuspended());

  macroCoroutine.runCoroutine();
  assertTrue(macroCoroutine.isYielding());

  macroCoroutine.runCoroutine();
  assertTrue(macroCoroutine.isDelaying());

  macroCoroutine.runCoroutine();
  assertTrue(macroCoroutine.isDelaying());

  TestableClockInterface::setSeconds(1);
  macroCoroutine.runCoroutine();
  assertTrue(macroCoroutine.isYielding());

  macroCoroutine.runCoroutine();
  assertTrue(macroCoroutine.isYielding());

  macroCoroutineFlag = true;
  macroCoroutine.runCoroutine();
  assertTrue(macroCoroutine.isEnding());

  macroCoroutine.runCoroutine();
  assertTrue(macroCoroutine.isEnding());
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
