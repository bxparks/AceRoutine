#line 2 "SuspendTest.ino"

#include <AceRoutine.h>
#include <AUnitVerbose.h>
#include <AceCommon.h> // PrintStr

using ace_routine::Coroutine;
using ace_routine::CoroutineScheduler;
using aunit::TestRunner;
using ace_common::PrintStr;

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

TestRoutine routine1;
TestRoutine routine2;

test(suspendAndResume) {
  PrintStr<256> output;

  CoroutineScheduler::list(output);
  assertEqual(
    F("Coroutine routine1; status: Yielding\r\n"
      "Coroutine routine2; status: Yielding\r\n"),
    output.getCstr()
  );

  output.flush();
  routine2.suspend();
  CoroutineScheduler::list(output);
  assertEqual(
    F("Coroutine routine1; status: Yielding\r\n"
      "Coroutine routine2; status: Suspended\r\n"),
    output.getCstr()
  );

  output.flush();
  routine2.resume();
  CoroutineScheduler::list(output);
  assertEqual(
    F("Coroutine routine1; status: Yielding\r\n"
      "Coroutine routine2; status: Yielding\r\n"),
    output.getCstr()
  );

}

// ---------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  delay(1000); // some boards reboot twice
#endif

  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  // Use setupCoroutineOrderedByName() to get a deterministic ordering of
  // coroutines, so that the output of CoroutineScheduler::list() is
  // predictable.
  routine1.setupCoroutineOrderedByName("routine1");
  routine2.setupCoroutineOrderedByName("routine2");
}

void loop() {
  TestRunner::run();
}
