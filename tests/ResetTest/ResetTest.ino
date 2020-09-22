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
      COROUTINE_LOOP() {
        Serial.print(count);
        Serial.print(' ');
        count++;
        COROUTINE_YIELD();
      }
    }

    int count = 0;
};

ResettingCoroutine resettingRoutine;

// ---------------------------------------------------------------------------

int iterCount = 0;

void setup() {
#if defined(ARDUINO)
  delay(1000); // some boards reboot twice
#endif

  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  resettingRoutine.setupCoroutine("resettingRoutine");
}

// This should cause the ResettingCoroutine to run twice.
// The output should be:
//
//    0 1 2 3 4 5 6 7 8 9
//    0 1 2 3 4 5 6 7 8 9
//    Done

void loop() {
  if (iterCount < 2) {
    resettingRoutine.runCoroutine();
    if (resettingRoutine.count >= 10) {
      Serial.println();
      resettingRoutine.count = 0;
      resettingRoutine.reset();

      iterCount++;
    }
  } else if (iterCount == 2) {
    Serial.println("Done");
    iterCount++;
  }
}

