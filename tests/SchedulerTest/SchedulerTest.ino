#line 2 "SchedulerTest.ino"

#include <AceRoutine.h>
#include <AUnitVerbose.h>
#include "ace_routine/testing/TestableCoroutine.h"
#include <AceCommon.h> // PrintStr

using namespace aunit;
using namespace ace_routine;
using ace_routine::testing::TestableCoroutine;
using ace_common::PrintStr;

// ---------------------------------------------------------------------------

class CoroutineC : public TestableCoroutine {
  public:
    int runCoroutine() override {
      COROUTINE_BEGIN();
      COROUTINE_DELAY(100);
      COROUTINE_END();
    }
};

CoroutineC c;

class CoroutineB : public TestableCoroutine {
  public:
    int runCoroutine() override {
      COROUTINE_BEGIN();
      COROUTINE_YIELD();
      COROUTINE_DELAY(25);
      COROUTINE_AWAIT(c.isDone());
      COROUTINE_END();
    }
};

// Define 'b' before a because 'a' uses 'b'
CoroutineB b;

class CoroutineA : public TestableCoroutine {
  public:
    int runCoroutine() override {
      COROUTINE_LOOP() {
        COROUTINE_DELAY(25);
        COROUTINE_AWAIT(b.isDone());
      }
    }
};

// Define 'a' last. If there is a circular dependency between a and b, we can
// use a pointer (Coroutine *a and Coroutine* b) to break the circular
// dependency, just like any other normal objects.
CoroutineA a;

class CoroutineExtra : public TestableCoroutine {
  public:
    int runCoroutine() override {
      COROUTINE_BEGIN();
      COROUTINE_END();
    }
};

// An extra coroutine, initially suspended using extra.suspend().
CoroutineExtra extra;

// Only 3 coroutines are initially active: a, b, c
test(AceRoutineTest, scheduler) {
  // Validate the initial states of various coroutines.
  PrintStr<256> output;
  CoroutineScheduler::list(output);
  assertEqual(
    F(
      "Coroutine a; status: Yielding\r\n"
      "Coroutine b; status: Yielding\r\n"
      "Coroutine c; status: Yielding\r\n"
      "Coroutine extra; status: Suspended\r\n"
    ),
    output.getCstr()
  );

  // initially everything (except 'extra') is enabled
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isYielding());
  assertTrue(extra.isSuspended());

  // run a
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isYielding());
  assertTrue(c.isYielding());
  assertTrue(extra.isSuspended());

  // run b
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isYielding());
  assertTrue(c.isYielding());
  assertTrue(extra.isSuspended());

  // run c
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // skip extra
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  a.coroutineMillis(10);
  b.coroutineMillis(10);
  c.coroutineMillis(10);
  extra.coroutineMillis(10);

  // run a
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // run b
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isDelaying());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // run c
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isDelaying());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // skip extra
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isDelaying());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  a.coroutineMillis(36);
  b.coroutineMillis(36);
  c.coroutineMillis(36);
  extra.coroutineMillis(36);

  // run a
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isDelaying());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // run b, goes into Yielding
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // run c
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // run c
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  a.coroutineMillis(101);
  b.coroutineMillis(101);
  c.coroutineMillis(101);
  extra.coroutineMillis(101);

  // run a
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // run b
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // run c
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isEnding());
  assertTrue(extra.isSuspended());

  // run extra
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isEnding());
  assertTrue(extra.isSuspended());

  a.coroutineMillis(102);
  b.coroutineMillis(102);
  c.coroutineMillis(102);
  extra.coroutineMillis(102);

  // run a
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isEnding());
  assertTrue(extra.isSuspended());

  // run b - ending
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isEnding());
  assertTrue(c.isEnding());
  assertTrue(extra.isSuspended());

  // run c - terminated
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isEnding());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run extra - suspended
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isEnding());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  a.coroutineMillis(103);
  b.coroutineMillis(103);
  c.coroutineMillis(103);
  extra.coroutineMillis(103);

  // run a, waiting for b over
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isEnding());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run b - terminated
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run c - terminated
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run extra - suspended
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  a.coroutineMillis(104);
  b.coroutineMillis(104);
  c.coroutineMillis(104);
  extra.coroutineMillis(104);

  // run a - hits COROUTINE_DELAY(25)
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run b - terminated
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run c - terminated
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run extra - suspended
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // step +26 millis
  a.coroutineMillis(130);
  b.coroutineMillis(130);
  c.coroutineMillis(130);
  extra.coroutineMillis(130);

  // run a - hits COROUTINE_AWAIT() which yields immediately
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run b - terminated
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run c - terminated
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run extra - suspended
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // step 1 millis
  a.coroutineMillis(131);
  b.coroutineMillis(131);
  c.coroutineMillis(131);
  extra.coroutineMillis(131);

  // resume 'extra'
  assertTrue(extra.isSuspended());
  extra.resume();

  // run a, hits COROUTINE_DELAY()
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isYielding());

  // run b
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isYielding());

  // run c
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isYielding());

  // run extra
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isEnding());

  // step +28 millis
  a.coroutineMillis(159);
  b.coroutineMillis(159);
  c.coroutineMillis(159);
  extra.coroutineMillis(159);

  // run a, hits AWAIT()
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isEnding());

  // run b
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isEnding());

  // run c
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isEnding());

  // run extra
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isTerminated());
}

// ---------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  delay(1000); // some boards reboot twice
#endif

  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  a.setupCoroutineOrderedByName("a");
  b.setupCoroutineOrderedByName("b");
  c.setupCoroutineOrderedByName("c");
  extra.setupCoroutineOrderedByName("extra");

  // Start the 'extra' coroutine in suspended state. Starting v1.2, a
  // suspended coroutine will be retained in the linked list of coroutines.
  extra.suspend();

  CoroutineScheduler::setup();
}

void loop() {
  TestRunner::run();
}
