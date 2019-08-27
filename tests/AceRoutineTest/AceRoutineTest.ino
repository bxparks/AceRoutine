#line 2 "AceCoroutineTest.ino"

#include <AceRoutine.h>
#include <AUnitVerbose.h>
#include "ace_routine/testing/TestableCoroutine.h"

using namespace ace_routine;
using namespace ace_routine::testing;
using namespace aunit;

// ---------------------------------------------------------------------------

test(AceRoutineTest, FCString_compareTo) {
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

// ---------------------------------------------------------------------------

test(AceRoutineTest, statusStrings) {
  assertEqual(Coroutine::sStatusStrings[Coroutine::kStatusSuspended],
      "Suspended");
  assertEqual(Coroutine::sStatusStrings[Coroutine::kStatusYielding],
      "Yielding");
  assertEqual(Coroutine::sStatusStrings[Coroutine::kStatusDelaying],
      "Delaying");
  assertEqual(Coroutine::sStatusStrings[Coroutine::kStatusRunning],
      "Running");
  assertEqual(Coroutine::sStatusStrings[Coroutine::kStatusEnding],
      "Ending");
  assertEqual(Coroutine::sStatusStrings[Coroutine::kStatusTerminated],
      "Terminated");
}

// ---------------------------------------------------------------------------

Channel<int> channel;

test(AceRoutineTest, channelReadAndWrite) {
  int writeValue = 2;
  int readValue = 0;

  assertFalse(channel.read(readValue));
  assertFalse(channel.read(readValue)); // second call should also return false
  assertFalse(channel.write(writeValue));
  assertFalse(channel.write(writeValue)); // second call should return false

  assertTrue(channel.read(readValue));
  assertTrue(channel.write(writeValue)); // write() can proceed after read()

  assertEqual(readValue, writeValue);
}

test(AceRoutineTest, channelWriteMacro) {
  int writeValue = 2;
  int readValue = 0;

  assertFalse(channel.read(readValue));
  assertFalse(channel.read(readValue)); // second call should also return false

  // Test the methods used by COROUTINE_CHANNEL_WRITE()
  channel.setValue(writeValue);
  assertFalse(channel.write());
  assertFalse(channel.write()); // second call should return false

  assertTrue(channel.read(readValue));
  assertTrue(channel.write()); // write() can proceed after read()

  assertEqual(readValue, writeValue);
}

// ---------------------------------------------------------------------------

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

// Verify that multiple calls to Coroutine::runCoroutine() after it ends is ok.
test(AceRoutineTest, simpleCoroutine) {
  simpleCoroutine.coroutineMillis(0);
  assertTrue(simpleCoroutine.isSuspended());

  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isYielding());

  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isDelaying());

  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isDelaying());

  simpleCoroutine.coroutineMillis(1);
  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isYielding());

  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isYielding());

  simpleCoroutineFlag = true;
  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isEnding());

  simpleCoroutine.runCoroutine();
  assertTrue(simpleCoroutine.isEnding());
}

// 'c' is defined in another .cpp file
EXTERN_COROUTINE(TestableCoroutine, c);

// Define 'b' before a because 'a' uses 'b'
COROUTINE(TestableCoroutine, b) {
  COROUTINE_BEGIN();
  COROUTINE_YIELD();
  COROUTINE_DELAY(25);
  COROUTINE_AWAIT(c.isDone());
  COROUTINE_END();
}

// Define 'a' last. If there is a circular dependency between a and b, we can
// use a pointer (Coroutine *a and Coroutine* b) to break the circular
// dependency, just like any other normal objects.
COROUTINE(TestableCoroutine, a) {
  COROUTINE_LOOP() {
    COROUTINE_DELAY(25);
    COROUTINE_AWAIT(b.isDone());
  }
}

// An extra coroutine, initially suspended using extra.suspend().
COROUTINE(TestableCoroutine, extra) {
  COROUTINE_BEGIN();
  COROUTINE_END();
}

// Only 3 coroutines are initially active: a, b, c
test(AceRoutineTest, scheduler) {
  // initially everything (except 'extra') is enabled
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isYielding());

  // run a
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isYielding());
  assertTrue(c.isYielding());

  // run b
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isYielding());
  assertTrue(c.isYielding());

  // run c
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());

  a.coroutineMillis(10);
  b.coroutineMillis(10);
  c.coroutineMillis(10);

  // run a
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());

  // run b
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isDelaying());
  assertTrue(c.isDelaying());

  // run c
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isDelaying());
  assertTrue(c.isDelaying());

  a.coroutineMillis(36);
  b.coroutineMillis(36);
  c.coroutineMillis(36);

  // run a
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isDelaying());
  assertTrue(c.isDelaying());

  // run b
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());

  // run c
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());

  a.coroutineMillis(101);
  b.coroutineMillis(101);
  c.coroutineMillis(101);

  // run a
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());

  // run b
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isDelaying());

  // run c
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isEnding());

  a.coroutineMillis(102);
  b.coroutineMillis(102);
  c.coroutineMillis(102);

  // run a
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isYielding());
  assertTrue(c.isEnding());

  // run b
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isEnding());
  assertTrue(c.isEnding());

  // run c - removed from list
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
  assertTrue(b.isEnding());
  assertTrue(c.isTerminated());

  a.coroutineMillis(103);
  b.coroutineMillis(103);
  c.coroutineMillis(103);

  // run a, waiting for b over, loops around to delay(25)
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isEnding());

  // run b - removed from list
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());
  assertTrue(b.isTerminated());

  a.coroutineMillis(104);
  b.coroutineMillis(104);
  c.coroutineMillis(104);

  // run a - hits COROUTINE_DELAY()
  CoroutineScheduler::loop();
  assertTrue(a.isDelaying());

  // step +26 millis
  a.coroutineMillis(130);
  b.coroutineMillis(130);
  c.coroutineMillis(130);

  // run a - hits COROUTINE_AWAIT() which yields immediately
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());

  // step 1 millis
  a.coroutineMillis(131);
  b.coroutineMillis(131);
  c.coroutineMillis(131);
  extra.coroutineMillis(131);

  // resume 'extra'
  assertTrue(extra.isSuspended());
  extra.resume();

  // run 'extra'
  CoroutineScheduler::loop();
  assertTrue(extra.isEnding());
  assertTrue(a.isYielding());

  // run 'a', hits COROUTINE_DELAY()
  CoroutineScheduler::loop();
  assertTrue(extra.isEnding());
  assertTrue(a.isDelaying());

  // step +28 millis
  a.coroutineMillis(159);
  b.coroutineMillis(159);
  c.coroutineMillis(159);
  extra.coroutineMillis(159);

  // run 'extra'
  CoroutineScheduler::loop();
  assertTrue(extra.isTerminated());
  assertTrue(a.isDelaying());

  // run 'a', hits COROUTINE_AWAIT()
  CoroutineScheduler::loop();
  assertTrue(a.isYielding());
}

// ---------------------------------------------------------------------------
test(AceRoutineTest, udiv1000Test) {
  assertEqual((unsigned long) 9, ace_routine::internal::udiv1000(10L*1000));
  assertEqual((unsigned long) 97, ace_routine::internal::udiv1000(100L*1000));
  assertEqual((unsigned long) 997,
      ace_routine::internal::udiv1000(1000L*1000));
  assertEqual((unsigned long) 9993,
      ace_routine::internal::udiv1000(10L*1000*1000));
  assertEqual((unsigned long) 99987,
      ace_routine::internal::udiv1000(100L*1000*1000));
  assertEqual((unsigned long) 999980,
      ace_routine::internal::udiv1000(1000L*1000*1000));
}

// ---------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  delay(1000); // some boards reboot twice
#endif

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
