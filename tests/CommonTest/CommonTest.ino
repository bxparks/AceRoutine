#line 2 "CommonTests.ino"

#include <AceRoutine.h>
#include <AUnitVerbose.h>
#include "ace_routine/testing/TestableRoutine.h"
#include "ace_routine/testing/FakePrinter.h"

// Import an internal function in Routine.cpp or RoutineScheduler.cpp.
namespace ace_routine {
namespace internal {
extern int compareName(const char* n, const char* m);
}
}

using namespace ace_routine;
using namespace ace_routine::testing;
using namespace ace_routine::internal;
using namespace aunit;

// These ROUTINE()s need to be a separate .ino file from AceRoutineTest because
// they change the ordering of the RoutineScheduler.loop() processing, which
// changes the assertions. If/when we add the ability to suspend() and resume()
// Routines, we might be able to move these back into AceRoutineTest, and
// suspend them before they are run by the scheduler.

ROUTINE(unamed) {
  ROUTINE_BEGIN();
  ROUTINE_END();
}

ROUTINE_NAMED(named) {
  ROUTINE_BEGIN();
  ROUTINE_END();
}

test(compareName) {
  assertEqual(0, compareName(
      (const char*) nullptr, (const char*) nullptr));
  assertEqual(-1, compareName(nullptr, "a"));
  assertEqual(1, compareName("a", nullptr));
  assertEqual(-1, compareName("a", "b"));
}

test(printName) {
  FakePrinter fakePrinter;

  assertEqual("named", named.getName());

  // Verify that the name of the routine with null getName() is printed as a
  // hexadecimal number of the 'this' pointer.
  assertEqual((const char*) nullptr, unamed.getName());
  unamed.printName(&fakePrinter);
  int compare = strncmp("0x", (const char*) fakePrinter.getBuffer(), 2);
  assertEqual(0, compare);
}

void setup() {
  delay(1000); // some boards reboot twice
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  RoutineScheduler::setup();
  RoutineScheduler::list(&Serial);
}

void loop() {
  TestRunner::run();
}
