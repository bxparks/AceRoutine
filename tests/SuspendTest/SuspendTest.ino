#line 2 "SuspendTest.ino"

#include <AceRoutine.h>
#include <AUnitVerbose.h>
#include <AceCommon.h> // PrintStr

using ace_routine::CoroutineScheduler;
using aunit::TestRunner;
using ace_common::PrintStr;

// ---------------------------------------------------------------------------
// Test Coroutine.suspend() and resume() methods. For v1.1 and earlier, calling
// CoroutineScheduler::list() just after a suspend()/resume() pair would go
// into an infinite loop.
// ---------------------------------------------------------------------------

COROUTINE(routine1) {
  COROUTINE_BEGIN();

  COROUTINE_END();
}

COROUTINE(routine2) {
  COROUTINE_BEGIN();

  COROUTINE_END();
}

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
}

void loop() {
  TestRunner::run();
}
