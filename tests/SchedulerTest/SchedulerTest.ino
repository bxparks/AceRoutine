#line 2 "SchedulerTest.ino"

#include <AceRoutine.h>
#include <AUnitVerbose.h>
#include <AceCommon.h> // PrintStr
#include "ace_routine/testing/TestableCoroutine.h"
#include "ace_routine/testing/TestableCoroutineScheduler.h"
#include "ace_routine/testing/TestableClockInterface.h"

using namespace aunit;
using namespace ace_routine;
using ace_routine::testing::TestableClockInterface;
using ace_routine::testing::TestableCoroutine;
using ace_routine::testing::TestableCoroutineScheduler;
using ace_common::PrintStr;

// ---------------------------------------------------------------------------

// Create the coroutines in the reverse order to the order desired, because each
// coroutine is inserted at the head of the singly-linked list.

class CoroutineExtra : public TestableCoroutine {
  public:
    int runCoroutine() override {
      COROUTINE_BEGIN();
      COROUTINE_END();
    }
};

// An extra coroutine, initially suspended using extra.suspend().
CoroutineExtra extra;

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

// Only 3 coroutines are initially active: a, b, c
test(AceRoutineTest, scheduler) {
  // Validate the initial states of various coroutines.
  PrintStr<200> output;
  TestableCoroutineScheduler::list(output);

  PrintStr<200> expected;
  printfTo(expected, "Coroutine %ld; status: Yielding\r\n", (uintptr_t) &a);
  printfTo(expected, "Coroutine %ld; status: Yielding\r\n", (uintptr_t) &b);
  printfTo(expected, "Coroutine %ld; status: Yielding\r\n", (uintptr_t) &c);
  printfTo(expected, "Coroutine %ld; status: Suspended\r\n",
      (uintptr_t) &extra);
  assertEqual(expected.cstr(), output.cstr());

  // initially everything (except 'extra') is enabled
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isYielding());
  assertTrue(extra.isSuspended());

  // run a
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isYielding());
  assertTrue(c.isYielding());
  assertTrue(extra.isSuspended());

  // run b
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isYielding());
  assertTrue(c.isYielding());
  assertTrue(extra.isSuspended());

  // run c
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // skip extra
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  TestableClockInterface::setMillis(10);

  // run a
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // run b
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isDelaying());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // run c
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isDelaying());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // skip extra
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isDelaying());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  TestableClockInterface::setMillis(36);

  // run a
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isDelaying());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // run b, goes into Yielding
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // run c
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // run c
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  TestableClockInterface::setMillis(101);

  // run a
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // run b
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());
  assertTrue(extra.isSuspended());

  // run c
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isEnding());
  assertTrue(extra.isSuspended());

  // run extra
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isEnding());
  assertTrue(extra.isSuspended());

  TestableClockInterface::setMillis(102);

  // run a
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isEnding());
  assertTrue(extra.isSuspended());

  // run b - ending
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isEnding());
  assertTrue(c.isEnding());
  assertTrue(extra.isSuspended());

  // run c - terminated
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isEnding());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run extra - suspended
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isEnding());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  TestableClockInterface::setMillis(103);

  // run a, waiting for b over
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isEnding());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run b - terminated
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run c - terminated
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run extra - suspended
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  TestableClockInterface::setMillis(104);

  // run a - hits COROUTINE_DELAY(25)
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run b - terminated
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run c - terminated
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run extra - suspended
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // step +26 millis
  TestableClockInterface::setMillis(130);

  // run a - hits COROUTINE_AWAIT() which yields immediately
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run b - terminated
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run c - terminated
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // run extra - suspended
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isSuspended());

  // step 1 millis
  TestableClockInterface::setMillis(131);

  // resume 'extra'
  assertTrue(extra.isSuspended());
  extra.resume();

  // run a, hits COROUTINE_DELAY()
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isYielding());

  // run b
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isYielding());

  // run c
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isYielding());

  // run extra
  TestableCoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isEnding());

  // step +28 millis
  TestableClockInterface::setMillis(159);

  // run a, hits AWAIT()
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isEnding());

  // run b
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isEnding());

  // run c
  TestableCoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isTerminated());
  assertTrue(c.isTerminated());
  assertTrue(extra.isEnding());

  // run extra
  TestableCoroutineScheduler::loop();
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

  // Start the 'extra' coroutine in suspended state. Starting v1.2, a
  // suspended coroutine will be retained in the linked list of coroutines.
  extra.suspend();

  TestableCoroutineScheduler::setup();
}

void loop() {
  TestRunner::run();
}
