#line 2 "ResetTest.ino"

#include <Arduino.h>
#include <AceRoutine.h>
#include <AUnitVerbose.h>
#include "ace_routine/testing/TestableCoroutine.h"

using namespace ace_routine;
using namespace ace_routine::testing;
using namespace aunit;

// ---------------------------------------------------------------------------
// Test Coroutine.reset() method using a Manual Coroutine.
// ---------------------------------------------------------------------------

class ResettingCoroutine: public TestableCoroutine {
  public:
    ResettingCoroutine() = default;

    int runCoroutine() override {
      // loop 5 times, then end the coroutine
      COROUTINE_BEGIN();
      for (; count < 5; count++) {
        COROUTINE_YIELD();
      }
      COROUTINE_END();
    }

    void restart() {
      count = 0;
      reset();
    }

  public:
    // Public for testing purposes.
    int count = 0;
};

ResettingCoroutine resettableCoroutine;

test(ResetTest, reset) {
  // Verify that the coroutine loops 5 times.
  resettableCoroutine.runCoroutine();
  assertEqual(0, resettableCoroutine.count);
  assertTrue(resettableCoroutine.isYielding());

  resettableCoroutine.runCoroutine();
  assertEqual(1, resettableCoroutine.count);
  assertTrue(resettableCoroutine.isYielding());

  resettableCoroutine.runCoroutine();
  assertEqual(2, resettableCoroutine.count);
  assertTrue(resettableCoroutine.isYielding());

  resettableCoroutine.runCoroutine();
  assertEqual(3, resettableCoroutine.count);
  assertTrue(resettableCoroutine.isYielding());

  resettableCoroutine.runCoroutine();
  assertEqual(4, resettableCoroutine.count);
  assertTrue(resettableCoroutine.isYielding());

  resettableCoroutine.runCoroutine();
  assertEqual(5, resettableCoroutine.count);
  assertTrue(resettableCoroutine.isDone());

  resettableCoroutine.runCoroutine();
  assertEqual(5, resettableCoroutine.count);
  assertTrue(resettableCoroutine.isDone());

  // Reset the coroutine using the custom restart() method, which delegates to
  // the Coroutine::reset() method. The coroutine goes into the Yielding state
  // so that it will run upon the next call to CoroutineScheduler::loop(). The
  // custom restart() method performs any additional reset of internal
  // variables, in this example, the 'count' variable.
  resettableCoroutine.restart();
  assertTrue(resettableCoroutine.isYielding());

  // Verify that it runs from the beginning of the loop again.
  resettableCoroutine.runCoroutine();
  assertEqual(0, resettableCoroutine.count);
  assertTrue(resettableCoroutine.isYielding());
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

