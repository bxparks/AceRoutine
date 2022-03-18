/*
 * This sketch attempts to find how much overhead is introduced by the context
 * switching performed by the CoroutineScheduler among the various coroutines.
 *
 * For the Teensy ARM microcontrollers, the benchmark numbers seem highly
 * dependent on compiler optimizer settings.
 */

#include <Arduino.h>
#include <AceRoutine.h>
#include <AceCommon.h> // printUint32AsFloat3To()
using namespace ace_routine;
using ace_common::printUint32AsFloat3To;

//-----------------------------------------------------------------------------

// NUM_ITERATIONS must be in multiples of 1000, due to the algorithm used to
// convert to nanos below.
#if defined(EPOXY_DUINO)
	const uint32_t NUM_ITERATIONS = 300000;
#elif defined(ARDUINO_ARCH_AVR)
	const uint32_t NUM_ITERATIONS = 10000;
#elif defined(ESP8266)
	const uint32_t NUM_ITERATIONS = 10000;
#else
	const uint32_t NUM_ITERATIONS = 30000;
#endif

#if ! defined(SERIAL_PORT_MONITOR)
	#define SERIAL_PORT_MONITOR Serial
#endif

volatile uint32_t counter = 0;

COROUTINE(counterA) {
  COROUTINE_LOOP() {
    counter++;
    COROUTINE_YIELD();
  }
}

COROUTINE(counterB) {
  COROUTINE_LOOP() {
    counter++;
    COROUTINE_YIELD();
  }
}

void checkEqual(
    const __FlashStringHelper* msg, uint32_t expected, uint32_t observed) {
  if (expected != observed) {
    SERIAL_PORT_MONITOR.print(msg);
    SERIAL_PORT_MONITOR.print(F(": check failed: "));
    SERIAL_PORT_MONITOR.print(F("expected="));
    SERIAL_PORT_MONITOR.print(expected);
    SERIAL_PORT_MONITOR.print(F("; observed="));
    SERIAL_PORT_MONITOR.print(observed);
    SERIAL_PORT_MONITOR.println();
  }
}

//-----------------------------------------------------------------------------

uint32_t doEmptyLoop(uint32_t iterations) {
  yield();
  counter = 0;
  uint32_t start = millis();
  for (uint32_t i = 0; i < iterations; i++) {
    counter++;
  }
  uint32_t end = millis();
  yield();
  checkEqual(F("doEmptyLoop(): "), counter, iterations);
  return end - start;
}

uint32_t doDirectScheduling(uint32_t iterations) {
  yield();
  counter = 0;
  uint32_t start = millis();

  // Run for 1/2 as many iterations because each loop calls 2 coroutines.
  for (uint32_t i = 0; i < iterations / 2; i++) {
    counterA.runCoroutine();
    counterB.runCoroutine();
  }
  uint32_t end = millis();
  yield();
  checkEqual(F("doDirectScheduling(): "), counter, iterations);
  return end - start;
}

uint32_t doDirectSchedulingWithProfiler(uint32_t iterations) {
  yield();
  counter = 0;
  uint32_t start = millis();

  // Run for 1/2 as many iterations because each loop calls 2 coroutines.
  for (uint32_t i = 0; i < iterations / 2; i++) {
    counterA.runCoroutineWithProfiler();
    counterB.runCoroutineWithProfiler();
  }
  uint32_t end = millis();
  yield();
  checkEqual(F("doDirectSchedulingWithProfiler(): "), counter, iterations);
  return end - start;
}

uint32_t doCoroutineScheduling(uint32_t iterations) {
  yield();
  counter = 0;
  uint32_t start = millis();
  for (uint32_t i = 0; i < iterations; i++) {
    CoroutineScheduler::loop();
  }
  uint32_t end = millis();
  yield();
  checkEqual(F("doCoroutineScheduling()"), counter, iterations);
  return end - start;
}

uint32_t doCoroutineSchedulingWithProfiler(uint32_t iterations) {
  yield();
  counter = 0;
  uint32_t start = millis();
  for (uint32_t i = 0; i < iterations; i++) {
    CoroutineScheduler::loopWithProfiler();
  }
  uint32_t end = millis();
  yield();
  checkEqual(F("doCoroutineSchedulingWithProfiler()"), counter, iterations);
  return end - start;
}

//-----------------------------------------------------------------------------

// Print millis 'ms' as micros (to 3 decimal places) per iteration as a floating
// point number. The number of 'iterations' must be divisible by 1000.
void printStats(
    const __FlashStringHelper* name, uint32_t ms, uint32_t iterations) {
  uint32_t nanosPerIteration = ms * 1000 / (iterations / 1000);
  SERIAL_PORT_MONITOR.print(name);
  SERIAL_PORT_MONITOR.print(' ');
  printUint32AsFloat3To(SERIAL_PORT_MONITOR, nanosPerIteration);
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(iterations);
  SERIAL_PORT_MONITOR.println();
}

//-----------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro

  SERIAL_PORT_MONITOR.println(F("SIZEOF"));

  SERIAL_PORT_MONITOR.print(F("sizeof(Coroutine): "));
  SERIAL_PORT_MONITOR.println(sizeof(Coroutine));
  SERIAL_PORT_MONITOR.print(F("sizeof(CoroutineScheduler): "));
  SERIAL_PORT_MONITOR.println(sizeof(CoroutineScheduler));
  SERIAL_PORT_MONITOR.print(F("sizeof(Channel<int>): "));
  SERIAL_PORT_MONITOR.println(sizeof(Channel<int>));
  SERIAL_PORT_MONITOR.print(F("sizeof(LogBinProfiler): "));
  SERIAL_PORT_MONITOR.println(sizeof(LogBinProfiler));
  SERIAL_PORT_MONITOR.print(F("sizeof(LogBinTableRenderer): "));
  SERIAL_PORT_MONITOR.println(sizeof(LogBinTableRenderer));
  SERIAL_PORT_MONITOR.print(F("sizeof(LogBinJsonRenderer): "));
  SERIAL_PORT_MONITOR.println(sizeof(LogBinJsonRenderer));

  CoroutineScheduler::setup();
  //CoroutineScheduler::list(SERIAL_PORT_MONITOR);

  SERIAL_PORT_MONITOR.println(F("BENCHMARKS"));

  uint32_t emptyLoopMillis = doEmptyLoop(NUM_ITERATIONS);
  printStats(F("EmptyLoop"), emptyLoopMillis, NUM_ITERATIONS);

  uint32_t directMillis = doDirectScheduling(NUM_ITERATIONS);
  printStats(F("DirectScheduling"), directMillis, NUM_ITERATIONS);

  uint32_t directMillisWithProfiler =
      doDirectSchedulingWithProfiler(NUM_ITERATIONS);
  printStats(F("DirectSchedulingWithProfiler"),
      directMillisWithProfiler, NUM_ITERATIONS);

  uint32_t schedulerMillis = doCoroutineScheduling(NUM_ITERATIONS);
  printStats(F("CoroutineScheduling"), schedulerMillis, NUM_ITERATIONS);

  uint32_t schedulerMillisWithProfiler =
      doCoroutineSchedulingWithProfiler(NUM_ITERATIONS);
  printStats(F("CoroutineSchedulingWithProfiler"),
      schedulerMillisWithProfiler, NUM_ITERATIONS);

  SERIAL_PORT_MONITOR.println(F("END"));

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {
}
