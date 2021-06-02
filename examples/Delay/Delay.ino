/*
 * Validate the actual delays of DELAY() incrementing a counter for
 * TEST_DURATION_MILLIS, then printing the actual count versus the expected
 * count.
 *
 * Results:
 *    * 16 MHz ATmega328P
 *      * countWithDelayMillis(): count=1993; expected=2000; elapsed=10004
 *    * 48 MHz SAMD21
 *      * countWithDelayMillis(): count=2000; expected=2000; elapsed=10000
 *    * ESP8266
 *      * countWithDelayMillis(): count=2000; expected=2000; elapsed=10001
 *    * ESP32
 *      * countWithDelayMillis(): count=2000; expected=2000; elapsed=10000
 *    * Teensy 3.2
 *      * countWithDelayMillis(): count=2000; expected=2000; elapsed=10000
 */

#include <Arduino.h>
#include <AceRoutine.h>
using namespace ace_routine;

#if defined(ESP32) && ! defined(SERIAL_PORT_MONITOR)
  #define SERIAL_PORT_MONITOR Serial
#endif

static const unsigned long TEST_DURATION_MILLIS = 10000;
static const unsigned long DELAY_MILLIS = 5; // 5 millis

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

COROUTINE(countWithDelayMillis) {
  static volatile unsigned long counter = 0;
  static unsigned long elapsed;
  COROUTINE_LOOP() {
    counter++;
    COROUTINE_DELAY(DELAY_MILLIS);
    elapsed = millis() - startMillis;
    if (elapsed >= TEST_DURATION_MILLIS) {
      SERIAL_PORT_MONITOR.print("countWithDelayMillis(): ");
      printResults(counter, TEST_DURATION_MILLIS / DELAY_MILLIS, elapsed);
      COROUTINE_END();
    }
  }
}

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro

  startMillis = millis();
  SERIAL_PORT_MONITOR.println("Starting COROUTINE_DELAY*() benchmark...");
}

// Manually execute the coroutines.
void loop() {
  countWithDelayMillis.runCoroutine();
}
