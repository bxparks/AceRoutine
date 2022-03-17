#line 2 "SuspendTest.ino"

#include <AceRoutine.h>
#include <AUnitVerbose.h>
#include <AceCommon.h> // PrintStr

using ace_routine::Coroutine;
using ace_routine::CoroutineScheduler;
using aunit::TestRunner;
using ace_common::PrintStr;
using ace_common::printfTo;

// ---------------------------------------------------------------------------
// Test Coroutine.suspend() and resume() methods. Before v1.2, calling
// CoroutineScheduler::list() just after a suspend()/resume() pair would cause
// an infinite loop.
// ---------------------------------------------------------------------------

class TestRoutine : public Coroutine {
  public:
    int runCoroutine() override {
      COROUTINE_BEGIN();
      COROUTINE_END();
    }
};

// Must be defined in reverse order to the order expected below.
TestRoutine routine2;
TestRoutine routine1;

test(suspendAndResume) {
  PrintStr<100> output;
  CoroutineScheduler::list(output);

  PrintStr<100> expected;
  printfTo(expected, "Coroutine 0x%lX; status: Yielding\r\n",
      (unsigned long) &routine1);
  printfTo(expected, "Coroutine 0x%lX; status: Yielding\r\n",
      (unsigned long) &routine2);
  assertEqual(expected.cstr(), output.cstr());

  output.flush();
  expected.flush();
  routine2.suspend();
  CoroutineScheduler::list(output);
  printfTo(expected, "Coroutine 0x%lX; status: Yielding\r\n",
      (unsigned long) &routine1);
  printfTo(expected, "Coroutine 0x%lX; status: Suspended\r\n",
      (unsigned long) &routine2);
  assertEqual(expected.cstr(), output.cstr());

  output.flush();
  expected.flush();
  routine2.resume();
  CoroutineScheduler::list(output);
  printfTo(expected, "Coroutine 0x%lX; status: Yielding\r\n",
      (unsigned long) &routine1);
  printfTo(expected, "Coroutine 0x%lX; status: Yielding\r\n",
      (unsigned long) &routine2);
  assertEqual(expected.cstr(), output.cstr());
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
