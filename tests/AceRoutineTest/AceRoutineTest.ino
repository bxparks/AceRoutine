#line 2 "AceRoutineTest.ino"

#include <AceRoutine.h>
#include <AUnitVerbose.h>
#include "ace_routine/testing/TestableRoutine.h"

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

bool simpleRoutineFlag = false;

// A routine that yields then ends. Verify that multiple calls to
// Routine::run() after it ends is ok.
ROUTINE(TestableRoutine, simpleRoutine) {
  ROUTINE_BEGIN();
  ROUTINE_YIELD();
  ROUTINE_DELAY(1);
  ROUTINE_AWAIT(simpleRoutineFlag);
  ROUTINE_END();
}

test(simpleRoutine) {
  simpleRoutine.millis(0);
  assertEqual(Routine::kStatusSuspended, simpleRoutine.getStatus());

  simpleRoutine.run();
  assertEqual(Routine::kStatusYielding, simpleRoutine.getStatus());

  simpleRoutine.run();
  assertEqual(Routine::kStatusDelaying, simpleRoutine.getStatus());

  simpleRoutine.run();
  assertEqual(Routine::kStatusDelaying, simpleRoutine.getStatus());

  simpleRoutine.millis(1);
  simpleRoutine.run();
  assertEqual(Routine::kStatusAwaiting, simpleRoutine.getStatus());

  simpleRoutine.run();
  assertEqual(Routine::kStatusAwaiting, simpleRoutine.getStatus());

  simpleRoutineFlag = true;
  simpleRoutine.run();
  assertEqual(Routine::kStatusEnding, simpleRoutine.getStatus());

  simpleRoutine.run();
  assertEqual(Routine::kStatusEnding, simpleRoutine.getStatus());
}

// c is defined in another .cpp file
EXTERN_ROUTINE(TestableRoutine, c);

// b is defined before a because 'a' uses 'b'
ROUTINE(TestableRoutine, b) {
  ROUTINE_BEGIN();
  ROUTINE_YIELD();
  ROUTINE_DELAY(25);
  ROUTINE_AWAIT(c.isDone());
  ROUTINE_END();
}

ROUTINE(TestableRoutine, a) {
  ROUTINE_LOOP() {
    ROUTINE_DELAY(25);
    ROUTINE_YIELD();
    ROUTINE_AWAIT(b.isDone());
  }
}

// An extra routine, initially suspended using extra.suspend().
ROUTINE(TestableRoutine, extra) {
  ROUTINE_BEGIN();
  ROUTINE_END();
}

// Only 3 routines are initially active: a, b, c
test(scheduler) {
  // initially everything is enabled
  assertEqual(Routine::kStatusYielding, a.getStatus());
  assertEqual(Routine::kStatusYielding, b.getStatus());
  assertEqual(Routine::kStatusYielding, c.getStatus());

  // run a
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusDelaying, a.getStatus());
  assertEqual(Routine::kStatusYielding, b.getStatus());
  assertEqual(Routine::kStatusYielding, c.getStatus());

  // run b
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusDelaying, a.getStatus());
  assertEqual(Routine::kStatusYielding, b.getStatus());
  assertEqual(Routine::kStatusYielding, c.getStatus());

  // run c
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusDelaying, a.getStatus());
  assertEqual(Routine::kStatusYielding, b.getStatus());
  assertEqual(Routine::kStatusDelaying, c.getStatus());

  a.millis(10);
  b.millis(10);
  c.millis(10);

  // run a
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusDelaying, a.getStatus());
  assertEqual(Routine::kStatusYielding, b.getStatus());
  assertEqual(Routine::kStatusDelaying, c.getStatus());

  // run b
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusDelaying, a.getStatus());
  assertEqual(Routine::kStatusDelaying, b.getStatus());
  assertEqual(Routine::kStatusDelaying, c.getStatus());

  // run c
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusDelaying, a.getStatus());
  assertEqual(Routine::kStatusDelaying, b.getStatus());
  assertEqual(Routine::kStatusDelaying, c.getStatus());

  a.millis(36);
  b.millis(36);
  c.millis(36);

  // run a
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusYielding, a.getStatus());
  assertEqual(Routine::kStatusDelaying, b.getStatus());
  assertEqual(Routine::kStatusDelaying, c.getStatus());

  // run b
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusYielding, a.getStatus());
  assertEqual(Routine::kStatusAwaiting, b.getStatus());
  assertEqual(Routine::kStatusDelaying, c.getStatus());

  // run c
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusYielding, a.getStatus());
  assertEqual(Routine::kStatusAwaiting, b.getStatus());
  assertEqual(Routine::kStatusDelaying, c.getStatus());

  a.millis(101);
  b.millis(101);
  c.millis(101);

  // run a
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusAwaiting, a.getStatus());
  assertEqual(Routine::kStatusAwaiting, b.getStatus());
  assertEqual(Routine::kStatusDelaying, c.getStatus());

  // run b
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusAwaiting, a.getStatus());
  assertEqual(Routine::kStatusAwaiting, b.getStatus());
  assertEqual(Routine::kStatusDelaying, c.getStatus());

  // run c
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusAwaiting, a.getStatus());
  assertEqual(Routine::kStatusAwaiting, b.getStatus());
  assertEqual(Routine::kStatusEnding, c.getStatus());

  a.millis(102);
  b.millis(102);
  c.millis(102);

  // run a
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusAwaiting, a.getStatus());
  assertEqual(Routine::kStatusAwaiting, b.getStatus());
  assertEqual(Routine::kStatusEnding, c.getStatus());

  // run b
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusAwaiting, a.getStatus());
  assertEqual(Routine::kStatusEnding, b.getStatus());
  assertEqual(Routine::kStatusEnding, c.getStatus());

  // run c - removed from list
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusAwaiting, a.getStatus());
  assertEqual(Routine::kStatusEnding, b.getStatus());
  assertEqual(Routine::kStatusTerminated, c.getStatus());

  a.millis(103);
  b.millis(103);
  c.millis(103);

  // run a, waiting for b over, loops around to delay(25)
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusDelaying, a.getStatus());
  assertEqual(Routine::kStatusEnding, b.getStatus());

  // run b - removed from list
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusDelaying, a.getStatus());
  assertEqual(Routine::kStatusTerminated, b.getStatus());

  a.millis(104);
  b.millis(104);
  c.millis(104);

  // run a - continues to run the ROUTINE_LOOP()
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusDelaying, a.getStatus());

  a.millis(130);
  b.millis(130);
  c.millis(130);

  // run a - continues to run the ROUTINE_LOOP()
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusYielding, a.getStatus());

  a.millis(131);
  b.millis(131);
  c.millis(131);
  extra.millis(131);

  // resume 'extra'
  assertEqual(Routine::kStatusSuspended, extra.getStatus());
  extra.resume();

  // run 'extra'
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusEnding, extra.getStatus());
  assertEqual(Routine::kStatusYielding, a.getStatus());

  // run 'a'
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusEnding, extra.getStatus());
  assertEqual(Routine::kStatusDelaying, a.getStatus());

  a.millis(132);
  b.millis(132);
  c.millis(132);
  extra.millis(132);

  // run 'extra'
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusTerminated, extra.getStatus());
  assertEqual(Routine::kStatusDelaying, a.getStatus());

  // run 'a'
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusDelaying, a.getStatus());
}

void setup() {
  delay(1000); // some boards reboot twice
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  extra.suspend();
  simpleRoutine.suspend();
  RoutineScheduler::setup();
  RoutineScheduler::list(&Serial);
}

void loop() {
  TestRunner::run();
}
