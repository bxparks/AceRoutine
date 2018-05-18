#line 2 "AceRoutineTest.ino"

#include <AceRoutine.h>
#include <AUnitVerbose.h>
#include "ace_routine/testing/TestableRoutine.h"
#include "ace_routine/testing/FakePrinter.h"

using namespace ace_routine;
using namespace ace_routine::testing;
using namespace aunit;

// Import an internal function in Routine.cpp or RoutineScheduler.cpp.
namespace ace_routine {
namespace internal {
extern int compareName(const char* n, const char* m);
}
}

using namespace ace_routine::internal;

// Initially suspended.
ROUTINE(TestableRoutine, named) {
  ROUTINE_BEGIN();
  ROUTINE_END();
}

// c is defined in another .cpp file
EXTERN_ROUTINE(TestableRoutine, c);

// b is defined before a because 'a' uses 'b'
ROUTINE(TestableRoutine, b) {
  ROUTINE_BEGIN();
  ROUTINE_YIELD();
  ROUTINE_DELAY(25);
  ROUTINE_AWAIT(c.isTerminated());
  ROUTINE_END();
}

ROUTINE(TestableRoutine, a) {
  ROUTINE_LOOP() {
    ROUTINE_DELAY(25);
    ROUTINE_YIELD();
    ROUTINE_AWAIT(b.isTerminated());
  }
}

// Only 3 routines are initially active: a, b, c
test(testRoutineMacros) {
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
  assertEqual(Routine::kStatusYielding, b.getStatus());
  assertEqual(Routine::kStatusDelaying, c.getStatus());

  // run c
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusYielding, a.getStatus());
  assertEqual(Routine::kStatusYielding, b.getStatus());
  assertEqual(Routine::kStatusDelaying, c.getStatus());

  a.millis(101);
  b.millis(101);
  c.millis(101);

  // run a
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusYielding, a.getStatus());
  assertEqual(Routine::kStatusYielding, b.getStatus());
  assertEqual(Routine::kStatusDelaying, c.getStatus());

  // run b
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusYielding, a.getStatus());
  assertEqual(Routine::kStatusYielding, b.getStatus());
  assertEqual(Routine::kStatusDelaying, c.getStatus());

  // run c
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusYielding, a.getStatus());
  assertEqual(Routine::kStatusYielding, b.getStatus());
  assertEqual(Routine::kStatusEnding, c.getStatus());

  a.millis(102);
  b.millis(102);
  c.millis(102);

  // run a
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusYielding, a.getStatus());
  assertEqual(Routine::kStatusYielding, b.getStatus());
  assertEqual(Routine::kStatusEnding, c.getStatus());

  // run b
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusYielding, a.getStatus());
  assertEqual(Routine::kStatusYielding, b.getStatus());
  assertEqual(Routine::kStatusEnding, c.getStatus());

  // run c - removed from list
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusYielding, a.getStatus());
  assertEqual(Routine::kStatusYielding, b.getStatus());
  assertEqual(Routine::kStatusTerminated, c.getStatus());

  a.millis(103);
  b.millis(103);
  c.millis(103);

  // run a
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusYielding, a.getStatus());
  assertEqual(Routine::kStatusYielding, b.getStatus());

  // run b
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusYielding, a.getStatus());
  assertEqual(Routine::kStatusEnding, b.getStatus());

  a.millis(104);
  b.millis(104);
  c.millis(104);

  // run a
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusYielding, a.getStatus());
  assertEqual(Routine::kStatusEnding, b.getStatus());

  // run b - removed from list
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusYielding, a.getStatus());
  assertEqual(Routine::kStatusTerminated, b.getStatus());

  a.millis(105);
  b.millis(105);
  c.millis(105);

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
  named.millis(131);

  // resume 'named'
  assertEqual(Routine::kStatusSuspended, named.getStatus());
  named.resume();

  // run 'named'
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusEnding, named.getStatus());
  assertEqual(Routine::kStatusYielding, a.getStatus());

  // run 'a'
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusEnding, named.getStatus());
  assertEqual(Routine::kStatusDelaying, a.getStatus());

  a.millis(132);
  b.millis(132);
  c.millis(132);
  named.millis(132);

  // run 'named'
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusTerminated, named.getStatus());
  assertEqual(Routine::kStatusDelaying, a.getStatus());

  // run 'a'
  RoutineScheduler::loop();
  assertEqual(Routine::kStatusDelaying, a.getStatus());
}

test(compareName) {
  assertEqual(0, compareName(
      (const char*) nullptr, (const char*) nullptr));
  assertEqual(-1, compareName(nullptr, "a"));
  assertEqual(1, compareName("a", nullptr));
  assertEqual(-1, compareName("a", "b"));
}

// TODO: fill this out
test(printName) {
  const FCString& name = named.getName();
}

void setup() {
  delay(1000); // some boards reboot twice
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  named.suspend();
  RoutineScheduler::setup();
}

void loop() {
  TestRunner::run();
}
