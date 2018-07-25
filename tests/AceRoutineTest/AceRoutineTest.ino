#line 2 "AceCoroutineTest.ino"

#include <AceRoutine.h>
#include <AUnitVerbose.h>
#include "ace_routine/testing/TestableCoroutine.h"

using namespace ace_routine;
using namespace ace_routine::testing;
using namespace aunit;

test(FCString_compareTo) {
  FCString n;
  FCString a("a");
  FCString b("b");
  FCString fa(F("a"));
  FCString fb(F("b"));

  assertEqual(n.compareTo(n), 0);
  assertLess(n.compareTo(a), 0);
  assertLess(n.compareTo(fa), 0);

  assertEqual(a.compareTo(fa), 0);
  assertEqual(fb.compareTo(b), 0);

  assertLess(a.compareTo(b), 0);
  assertLess(a.compareTo(fb), 0);
  assertMore(fb.compareTo(a), 0);
  assertMore(fb.compareTo(fa), 0);
}

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

// Verify that multiple calls to Coroutine::run() after it ends is ok.
test(simpleCoroutine) {
  simpleCoroutine.millis(0);
  assertEqual(Coroutine::kStatusSuspended, simpleCoroutine.getStatus());

  simpleCoroutine.run();
  assertEqual(Coroutine::kStatusYielding, simpleCoroutine.getStatus());

  simpleCoroutine.run();
  assertEqual(Coroutine::kStatusDelaying, simpleCoroutine.getStatus());

  simpleCoroutine.run();
  assertEqual(Coroutine::kStatusDelaying, simpleCoroutine.getStatus());

  simpleCoroutine.millis(1);
  simpleCoroutine.run();
  assertEqual(Coroutine::kStatusAwaiting, simpleCoroutine.getStatus());

  simpleCoroutine.run();
  assertEqual(Coroutine::kStatusAwaiting, simpleCoroutine.getStatus());

  simpleCoroutineFlag = true;
  simpleCoroutine.run();
  assertEqual(Coroutine::kStatusEnding, simpleCoroutine.getStatus());

  simpleCoroutine.run();
  assertEqual(Coroutine::kStatusEnding, simpleCoroutine.getStatus());
}

// c is defined in another .cpp file
EXTERN_COROUTINE(TestableCoroutine, c);

// Define b before a because 'a' uses 'b'
COROUTINE(TestableCoroutine, b) {
  COROUTINE_BEGIN();
  COROUTINE_YIELD();
  COROUTINE_DELAY(25);
  COROUTINE_AWAIT(c.isDone());
  COROUTINE_END();
}

// Define a last. If there is a circular dependency between a and b, we can use
// a pointer (Coroutine *a and Coroutine* b) to break the circular dependency,
// just like any other normal objects.
COROUTINE(TestableCoroutine, a) {
  COROUTINE_LOOP() {
    COROUTINE_DELAY(25);
    COROUTINE_YIELD();
    COROUTINE_AWAIT(b.isDone());
  }
}

// An extra coroutine, initially suspended using extra.suspend().
COROUTINE(TestableCoroutine, extra) {
  COROUTINE_BEGIN();
  COROUTINE_END();
}

// Only 3 coroutines are initially active: a, b, c
test(scheduler) {
  // initially everything is enabled
  assertEqual(Coroutine::kStatusYielding, a.getStatus());
  assertEqual(Coroutine::kStatusYielding, b.getStatus());
  assertEqual(Coroutine::kStatusYielding, c.getStatus());

  // run a
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusDelaying, a.getStatus());
  assertEqual(Coroutine::kStatusYielding, b.getStatus());
  assertEqual(Coroutine::kStatusYielding, c.getStatus());

  // run b
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusDelaying, a.getStatus());
  assertEqual(Coroutine::kStatusYielding, b.getStatus());
  assertEqual(Coroutine::kStatusYielding, c.getStatus());

  // run c
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusDelaying, a.getStatus());
  assertEqual(Coroutine::kStatusYielding, b.getStatus());
  assertEqual(Coroutine::kStatusDelaying, c.getStatus());

  a.millis(10);
  b.millis(10);
  c.millis(10);

  // run a
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusDelaying, a.getStatus());
  assertEqual(Coroutine::kStatusYielding, b.getStatus());
  assertEqual(Coroutine::kStatusDelaying, c.getStatus());

  // run b
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusDelaying, a.getStatus());
  assertEqual(Coroutine::kStatusDelaying, b.getStatus());
  assertEqual(Coroutine::kStatusDelaying, c.getStatus());

  // run c
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusDelaying, a.getStatus());
  assertEqual(Coroutine::kStatusDelaying, b.getStatus());
  assertEqual(Coroutine::kStatusDelaying, c.getStatus());

  a.millis(36);
  b.millis(36);
  c.millis(36);

  // run a
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusYielding, a.getStatus());
  assertEqual(Coroutine::kStatusDelaying, b.getStatus());
  assertEqual(Coroutine::kStatusDelaying, c.getStatus());

  // run b
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusYielding, a.getStatus());
  assertEqual(Coroutine::kStatusAwaiting, b.getStatus());
  assertEqual(Coroutine::kStatusDelaying, c.getStatus());

  // run c
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusYielding, a.getStatus());
  assertEqual(Coroutine::kStatusAwaiting, b.getStatus());
  assertEqual(Coroutine::kStatusDelaying, c.getStatus());

  a.millis(101);
  b.millis(101);
  c.millis(101);

  // run a
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusAwaiting, a.getStatus());
  assertEqual(Coroutine::kStatusAwaiting, b.getStatus());
  assertEqual(Coroutine::kStatusDelaying, c.getStatus());

  // run b
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusAwaiting, a.getStatus());
  assertEqual(Coroutine::kStatusAwaiting, b.getStatus());
  assertEqual(Coroutine::kStatusDelaying, c.getStatus());

  // run c
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusAwaiting, a.getStatus());
  assertEqual(Coroutine::kStatusAwaiting, b.getStatus());
  assertEqual(Coroutine::kStatusEnding, c.getStatus());

  a.millis(102);
  b.millis(102);
  c.millis(102);

  // run a
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusAwaiting, a.getStatus());
  assertEqual(Coroutine::kStatusAwaiting, b.getStatus());
  assertEqual(Coroutine::kStatusEnding, c.getStatus());

  // run b
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusAwaiting, a.getStatus());
  assertEqual(Coroutine::kStatusEnding, b.getStatus());
  assertEqual(Coroutine::kStatusEnding, c.getStatus());

  // run c - removed from list
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusAwaiting, a.getStatus());
  assertEqual(Coroutine::kStatusEnding, b.getStatus());
  assertEqual(Coroutine::kStatusTerminated, c.getStatus());

  a.millis(103);
  b.millis(103);
  c.millis(103);

  // run a, waiting for b over, loops around to delay(25)
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusDelaying, a.getStatus());
  assertEqual(Coroutine::kStatusEnding, b.getStatus());

  // run b - removed from list
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusDelaying, a.getStatus());
  assertEqual(Coroutine::kStatusTerminated, b.getStatus());

  a.millis(104);
  b.millis(104);
  c.millis(104);

  // run a - continues to run the COROUTINE_LOOP()
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusDelaying, a.getStatus());

  a.millis(130);
  b.millis(130);
  c.millis(130);

  // run a - continues to run the COROUTINE_LOOP()
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusYielding, a.getStatus());

  a.millis(131);
  b.millis(131);
  c.millis(131);
  extra.millis(131);

  // resume 'extra'
  assertEqual(Coroutine::kStatusSuspended, extra.getStatus());
  extra.resume();

  // run 'extra'
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusEnding, extra.getStatus());
  assertEqual(Coroutine::kStatusYielding, a.getStatus());

  // run 'a'
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusEnding, extra.getStatus());
  assertEqual(Coroutine::kStatusDelaying, a.getStatus());

  a.millis(132);
  b.millis(132);
  c.millis(132);
  extra.millis(132);

  // run 'extra'
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusTerminated, extra.getStatus());
  assertEqual(Coroutine::kStatusDelaying, a.getStatus());

  // run 'a'
  CoroutineScheduler::loop();
  assertEqual(Coroutine::kStatusDelaying, a.getStatus());
}

void setup() {
  delay(1000); // some boards reboot twice
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  extra.suspend();
  simpleCoroutine.suspend();
  CoroutineScheduler::setup();
  CoroutineScheduler::list(Serial);
}

void loop() {
  TestRunner::run();
}
