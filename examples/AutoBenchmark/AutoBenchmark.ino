/*
 * This sketch attempts to find how much overhead is introduced by the context
 * switching performed by the CoroutineScheduler among the various coroutines.
 *
 * For the Teensy ARM microcontrollers, the benchmark numbers seem highly
 * dependent on compiler optimizer settings.
 */

#include <Arduino.h>
#include <AceRoutine.h>
#include <AceCommon.h> // printPad3To()
using namespace ace_routine;
using ace_common::printPad3To;

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

volatile uint16_t counter = 0;

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

uint16_t doBaseline() {
  yield();
  uint16_t start = millis();
  for (uint32_t i = 0; i < NUM_ITERATIONS; i++) {
    counter++;
  }
  uint16_t end = millis();
  yield();
  return end - start;
}

uint16_t doAceRoutine() {
  yield();
  uint16_t start = millis();
  for (uint32_t i = 0; i < NUM_ITERATIONS; i++) {
    CoroutineScheduler::loop();
  }
  uint16_t end = millis();
  yield();
  return end - start;
}

void printNanosAsMicros(Print& printer, uint16_t nanos) {
  uint16_t wholeMicros = nanos / 1000;
  uint16_t fracMicros = nanos - wholeMicros * 1000;
  printer.print(wholeMicros);
  printer.print('.');
  printPad3To(printer, fracMicros, '0');
}

void printStats(uint16_t baselineMillis, uint16_t coroutineMillis) {
  uint16_t baseNanosPerIteration =
      (uint32_t) baselineMillis * 1000 * 1000 / NUM_ITERATIONS;
  uint16_t coroutineNanosPerIteration =
      (uint32_t) coroutineMillis * 1000 * 1000 / NUM_ITERATIONS;
  uint16_t diffNanos = coroutineNanosPerIteration - baseNanosPerIteration;
  printNanosAsMicros(SERIAL_PORT_MONITOR, coroutineNanosPerIteration);
  SERIAL_PORT_MONITOR.print(' ');
  printNanosAsMicros(SERIAL_PORT_MONITOR, baseNanosPerIteration);
  SERIAL_PORT_MONITOR.print(' ');
  printNanosAsMicros(SERIAL_PORT_MONITOR, diffNanos);
  SERIAL_PORT_MONITOR.println();
}

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

  CoroutineScheduler::setup();
  //CoroutineScheduler::list(SERIAL_PORT_MONITOR);

  SERIAL_PORT_MONITOR.println(F("BENCHMARKS"));

  uint16_t baselineMillis = doBaseline();
  uint16_t coroutineMillis = doAceRoutine();
  printStats(baselineMillis, coroutineMillis);

  SERIAL_PORT_MONITOR.println(F("END"));

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {
}
