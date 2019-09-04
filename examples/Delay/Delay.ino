/*
 * Validate the actual delays of DELAY_SECONDS(), DELAY() and DELAY_MICROS() by
 * incrementing a counter for TEST_DURATION_MILLIS, then printing the actual
 * count versus the expected count.
 *
 * Results:
 *    * 16 MHz ATmega328P
 *      * countWithDelayMicros(): count=103349; expected=200000; elapsed=10000
 *      * countWithDelayMillis(): count=1993; expected=2000; elapsed=10004
 *      * countWithDelaySeconds(): count=11; expected=10; elapsed=10265
 *    * 48 MHz SAMD21
 *      * countWithDelayMillis(): count=2000; expected=2000; elapsed=10000
 *      * countWithDelayMicros(): count=161394; expected=200000; elapsed=10000
 *      * countWithDelaySeconds(): count=11; expected=10; elapsed=10253
 *    * ESP8266
 *      * countWithDelayMicros(): count=160897; expected=200000; elapsed=10000
 *      * countWithDelayMillis(): count=2000; expected=2000; elapsed=10001
 *      * countWithDelaySeconds(): count=10; expected=10; elapsed=10191
 *    * ESP32
 *      * countWithDelayMicros(): count=184164; expected=200000; elapsed=10000
 *      * countWithDelayMillis(): count=2000; expected=2000; elapsed=10000
 *      * countWithDelaySeconds(): count=11; expected=10; elapsed=10965
 *    * Teensy 3.2
 *      * countWithDelayMillis(): count=2000; expected=2000; elapsed=10000
 *      * countWithDelayMicros(): count=184460; expected=200000; elapsed=10000
 *      * countWithDelaySeconds(): count=11; expected=10; elapsed=10700
 */

#include <Arduino.h>
#include <AceRoutine.h>
using namespace ace_routine;

#if defined(ESP32) && ! defined(SERIAL_PORT_MONITOR)
  #define SERIAL_PORT_MONITOR Serial
#endif

static const unsigned long TEST_DURATION_MILLIS = 10000;
static const unsigned long DELAY_MICROS = 50; // 50 micros
static const unsigned long DELAY_MILLIS = 5; // 5 millis
static const unsigned long DELAY_SECONDS = 1; // 1 seconds

static unsigned long startMillis;

void printResults(unsigned long count, unsigned long expected,
    unsigned long elapsed) {
  SERIAL_PORT_MONITOR.print(F("count="));
  SERIAL_PORT_MONITOR.print(count);
  SERIAL_PORT_MONITOR.print(F("; expected="));
  SERIAL_PORT_MONITOR.print(expected);
  SERIAL_PORT_MONITOR.print(F("; elapsed="));
  SERIAL_PORT_MONITOR.println(elapsed);
}

COROUTINE(countWithDelayMicros) {
  static volatile unsigned long counter = 0;
  static unsigned long elapsed;
  COROUTINE_LOOP() {
    counter++;
    COROUTINE_DELAY_MICROS(DELAY_MICROS);
    elapsed = millis() - startMillis;
    if (elapsed >= TEST_DURATION_MILLIS) {
      SERIAL_PORT_MONITOR.print("countWithDelayMicros(): ");
      printResults(counter, TEST_DURATION_MILLIS * 1000 / DELAY_MICROS,
          elapsed);
      COROUTINE_END();
    }
  }
}

COROUTINE(countWithDelayMillis) {
  static volatile unsigned long counter = 0;
  static unsigned long elapsed;
  COROUTINE_LOOP() {
    counter++;
    COROUTINE_DELAY(DELAY_MILLIS);
    elapsed = millis() - startMillis;
    if (elapsed >= TEST_DURATION_MILLIS) {
      SERIAL_PORT_MONITOR.print("countWithDelayMillis(): ");
      printResults(counter, TEST_DURATION_MILLIS / DELAY_MILLIS,
          elapsed);
      COROUTINE_END();
    }
  }
}

COROUTINE(countWithDelaySeconds) {
  static volatile unsigned long counter = 0;
  static unsigned long elapsed;
  COROUTINE_LOOP() {
    counter++;
    COROUTINE_DELAY_SECONDS(DELAY_SECONDS);
    elapsed = millis() - startMillis;
    if (elapsed >= TEST_DURATION_MILLIS) {
      SERIAL_PORT_MONITOR.print("countWithDelaySeconds(): ");
      printResults(counter, TEST_DURATION_MILLIS / (DELAY_SECONDS * 1000),
          elapsed);
      COROUTINE_END();
    }
  }
}

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000);
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro

  startMillis = millis();
  SERIAL_PORT_MONITOR.println("Starting COROUTINE_DELAY*() benchmark...");
}

// Manually execute the coroutines.
void loop() {
  countWithDelayMicros.runCoroutine();
  countWithDelayMillis.runCoroutine();
  countWithDelaySeconds.runCoroutine();
}
