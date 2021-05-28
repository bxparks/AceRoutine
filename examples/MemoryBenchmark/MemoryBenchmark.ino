/*
 * A program which attempts to calculate how much flash and static memory are
 * used by the AceRoutine library. Set the FEATURE macro to various integer to
 * compile different parts of the AceButton library.
 */

#include <Arduino.h>

// DO NOT MODIFY THIS LINE. This will be overwritten by collect.sh on each
// iteration, incrementing from 0 to N. The Arduino IDE will compile the
// program, then the script will extract the flash and static memory usage
// numbers printed out by the Arduino compiler. The numbers will be printed on
// the STDOUT, which then can be saved to a file specific for a particular
// hardware platform, e.g. "nano.txt" or "esp8266.txt".
#define FEATURE 0

// List of features to gather memory statistics.
#define FEATURE_BASELINE 0
#define FEATURE_MANUAL_DELAY_LOOP 1
#define FEATURE_ONE_COROUTINE 2
#define FEATURE_ONE_COROUTINE_DELAY_SECONDS 3
#define FEATURE_TWO_COROUTINES 4
#define FEATURE_TWO_COROUTINES_DELAY_SECONDS 5
#define FEATURE_SCHEDULER_ONE_COROUTINE 6
#define FEATURE_SCHEDULER_TWO_COROUTINES 7

#if FEATURE != FEATURE_BASELINE
  #include <AceRoutine.h>
  using namespace ace_routine;
#endif

// A volatile integer to prevent the compiler from optimizing away the entire
// program.
volatile int disableCompilerOptimization = 0;

#if FEATURE == FEATURE_MANUAL_DELAY_LOOP

  // Hand-rolled alternative to using a COROUTINE() that executes every 10
  // milliseconds.
  void manualDelayLoop() {
    static uint16_t prevMillis;

    uint16_t nowMillis = millis();
    if ((uint16_t) nowMillis - prevMillis >= 10) {
      prevMillis = nowMillis;

      disableCompilerOptimization = 1;
    }
  }

#elif FEATURE == FEATURE_ONE_COROUTINE

  COROUTINE(a) {
    COROUTINE_LOOP() {
      disableCompilerOptimization = 1;
      COROUTINE_DELAY(10);
    }
  }

#elif FEATURE == FEATURE_ONE_COROUTINE_DELAY_SECONDS

  COROUTINE(a) {
    COROUTINE_LOOP() {
      disableCompilerOptimization = 1;
      COROUTINE_DELAY_SECONDS(10);
    }
  }

#elif FEATURE == FEATURE_TWO_COROUTINES

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

#elif FEATURE == FEATURE_TWO_COROUTINES_DELAY_SECONDS

  COROUTINE(a) {
    COROUTINE_LOOP() {
      disableCompilerOptimization = 1;
      COROUTINE_DELAY_SECONDS(10);
    }
  }

  COROUTINE(b) {
    COROUTINE_LOOP() {
      disableCompilerOptimization = 1;
      COROUTINE_DELAY_SECONDS(10);
    }
  }

#elif FEATURE == FEATURE_SCHEDULER_ONE_COROUTINE

  class MyCoroutine : public Coroutine {
    public:
      int runCoroutine() override {
        COROUTINE_BEGIN();
        disableCompilerOptimization = 1;
        COROUTINE_END();
      }
  };

  MyCoroutine a;

#elif FEATURE == FEATURE_SCHEDULER_TWO_COROUTINES

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

#if FEATURE == FEATURE_SCHEDULER_ONE_COROUTINE
  a.setupCoroutine(F("a"));
#elif FEATURE == FEATURE_SCHEDULER_TWO_COROUTINES
  a.setupCoroutine(F("a"));
  b.setupCoroutine(F("b"));
#endif
}

void loop() {
#if FEATURE == FEATURE_BASELINE
  disableCompilerOptimization = 1;
#elif FEATURE == FEATURE_MANUAL_DELAY_LOOP
  manualDelayLoop();
#elif FEATURE == FEATURE_ONE_COROUTINE
  a.runCoroutine();
#elif FEATURE == FEATURE_ONE_COROUTINE_DELAY_SECONDS
  a.runCoroutine();
#elif FEATURE == FEATURE_TWO_COROUTINES
  a.runCoroutine();
  b.runCoroutine();
#elif FEATURE == FEATURE_TWO_COROUTINES_DELAY_SECONDS
  a.runCoroutine();
  b.runCoroutine();
#elif FEATURE == FEATURE_SCHEDULER_ONE_COROUTINE
  CoroutineScheduler::loop();
#elif FEATURE == FEATURE_SCHEDULER_TWO_COROUTINES
  CoroutineScheduler::loop();
#endif
}
