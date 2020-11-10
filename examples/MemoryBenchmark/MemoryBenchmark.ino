/*
 * A program which attempts to calculate how much flash and static memory are
 * used by the AceRoutine library. Set the FEATURE macro to various integer to
 * compile different parts of the AceButton library.
 */

#include <AceRoutine.h>
using namespace ace_routine;

// Set this to [0..n] to extract the flash and static memory usage.
// 0 - baseline
// 1 - One Coroutine
// 2 - Two Coroutines
// 3 - CoroutineScheduler, One Coroutine
// 4 - CoroutineScheduler, Two Coroutines
#define FEATURE 0

// A volatile integer to prevent the compiler from optimizing away the entire
// program.
volatile int disableCompilerOptimization = 0;

#if FEATURE == 1

  COROUTINE(a) {
    COROUTINE_LOOP() {
      disableCompilerOptimization = 1;
      COROUTINE_DELAY(10);
    }
  }

#elif FEATURE == 2

  COROUTINE(a) {
    COROUTINE_LOOP() {
      disableCompilerOptimization = 1;
      COROUTINE_DELAY(10);
    }
  }

  COROUTINE(b) {
    COROUTINE_LOOP() {
      disableCompilerOptimization = 1;
      COROUTINE_DELAY(10);
    }
  }

#elif FEATURE == 3

  class MyCoroutine : public Coroutine {
    public:
      int runCoroutine() override {
        COROUTINE_BEGIN();
        disableCompilerOptimization = 1;
        COROUTINE_END();
      }
  };

  MyCoroutine a;

#elif FEATURE == 4

  class MyCoroutine : public Coroutine {
    public:
      int runCoroutine() override {
        COROUTINE_LOOP() {
          disableCompilerOptimization = 1;
          COROUTINE_DELAY(10);
        }
      }
  };

  MyCoroutine a;
  MyCoroutine b;

#endif

void setup() {
  delay(1000);

#if FEATURE == 3
  a.setupCoroutine(F("a"));
#elif FEATURE == 4
  a.setupCoroutine(F("a"));
  b.setupCoroutine(F("b"));
#endif
}

void loop() {
#if FEATURE == 0
  disableCompilerOptimization = 1;
#elif FEATURE == 1
  a.runCoroutine();
#elif FEATURE == 2
  a.runCoroutine();
  b.runCoroutine();
#elif FEATURE == 3
  CoroutineScheduler::loop();
#elif FEATURE == 4
  CoroutineScheduler::loop();
#endif
}
