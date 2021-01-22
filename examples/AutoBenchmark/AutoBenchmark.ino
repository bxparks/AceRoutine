/*
 * This sketch attempts to find how much overhead is introduced by the context
 * switching performed by the CoroutineScheduler among the various coroutines.
 *
 * For the Teensy ARM microcontrollers, the benchmark numbers seem highly
 * dependent on compiler optimizer settings.
 */

#include <Arduino.h>
#include <AceRoutine.h>
using namespace ace_routine;

#if defined(ESP8266)
	const unsigned long DURATION = 1000;
#else
	const unsigned long DURATION = 3000;
#endif

#if ! defined(SERIAL_PORT_MONITOR)
	#define SERIAL_PORT_MONITOR Serial
#endif

volatile unsigned long counter = 0;

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

void doBaseline() {
  counter = 0;
  unsigned long start = millis();
  yield();
  while (millis() - start < DURATION) {
    counter++;
  }
  yield();
}

void doAceRoutine() {
  counter = 0;
  unsigned long start = millis();
  yield();
  while (millis() - start < DURATION) {
    CoroutineScheduler::loop();
  }
  yield();
}

void printStats(float baselineMicros, float coroutineMicros) {
  float diff = coroutineMicros - baselineMicros;
  SERIAL_PORT_MONITOR.print(coroutineMicros);
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(baselineMicros);
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.println(diff);
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

  doBaseline();
  float baselineMicros = DURATION * 1000.0 / counter;
  doAceRoutine();
  float coroutineMicros = DURATION * 1000.0 / counter;
  printStats(baselineMicros, coroutineMicros);

  SERIAL_PORT_MONITOR.println(F("END"));

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {
}
