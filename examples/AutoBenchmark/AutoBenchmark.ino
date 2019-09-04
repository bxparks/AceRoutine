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

#if defined(ESP32) && ! defined(SERIAL_PORT_MONITOR)
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

void printStats(float baseline, float aceCoroutine) {
  char buf[100];
  float diff = aceCoroutine - baseline;
  sprintf(buf, "      %2d.%02d |%2d.%02d |%2d.%02d |",
      (int)aceCoroutine, (int)(aceCoroutine*100)%100,
      (int)baseline, (int)(baseline*100)%100,
      (int)diff, (int)(diff*100)%100);
  SERIAL_PORT_MONITOR.println(buf);
}

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000);
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro

  SERIAL_PORT_MONITOR.print(F("sizeof(Coroutine): "));
  SERIAL_PORT_MONITOR.println(sizeof(Coroutine));
  SERIAL_PORT_MONITOR.print(F("sizeof(CoroutineScheduler): "));
  SERIAL_PORT_MONITOR.println(sizeof(CoroutineScheduler));
  SERIAL_PORT_MONITOR.print(F("sizeof(Channel<int>): "));
  SERIAL_PORT_MONITOR.println(sizeof(Channel<int>));

  CoroutineScheduler::setup();
  CoroutineScheduler::list(SERIAL_PORT_MONITOR);

  SERIAL_PORT_MONITOR.println(
      F("------------+------+------+"));
  SERIAL_PORT_MONITOR.println(
      F(" AceRoutine | base | diff |"));
  SERIAL_PORT_MONITOR.println(
      F("------------+------+------+"));

  doBaseline();
  float baseline = DURATION * 1000.0 / counter;

  doAceRoutine();
  float aceCoroutine = DURATION * 1000.0 / counter;

  printStats(baseline, aceCoroutine);
  SERIAL_PORT_MONITOR.println(
      F("------------+------+------+"));
}

void loop() {
}
