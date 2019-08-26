/*
 * Validate the actual delays of DELAY_SECONDS(), DELAY() and DELAY_MICROS() by
 * incrementing a counter for TEST_DURATION_MILLIS, then printing the actual
 * count versus expected count.
 */

#include <Arduino.h>
#include <AceRoutine.h>
using namespace ace_routine;

static const unsigned long TEST_DURATION_MILLIS = 10000;
static const unsigned long DELAY_MICROS = 50; // 50 micros
static const unsigned long DELAY_MILLIS = 5; // 5 millis
static const unsigned long DELAY_SECONDS = 1; // 1 seconds

static unsigned long startMillis;

void printResults(unsigned long count, unsigned long expected,
    unsigned long elapsed) {
  Serial.print(F("count="));
  Serial.print(count);
  Serial.print(F("; expected="));
  Serial.print(expected);
  Serial.print(F("; elapsed="));
  Serial.println(elapsed);
}

COROUTINE(countWithDelayMicros) {
  static volatile unsigned long counter = 0;
  COROUTINE_LOOP() {
    counter++;
    COROUTINE_DELAY_MICROS(DELAY_MICROS);
    unsigned long elapsed = millis() - startMillis;
    if (elapsed >= TEST_DURATION_MILLIS) {
      Serial.print("countWithDelayMicros(): ");
      printResults(counter, TEST_DURATION_MILLIS * 1000 / DELAY_MICROS,
          elapsed);
      COROUTINE_END();
    }
  }
}

COROUTINE(countWithDelayMillis) {
  static volatile unsigned long counter = 0;
  COROUTINE_LOOP() {
    counter++;
    COROUTINE_DELAY(DELAY_MILLIS);
    unsigned long elapsed = millis() - startMillis;
    if (elapsed >= TEST_DURATION_MILLIS) {
      Serial.print("countWithDelayMillis(): ");
      printResults(counter, TEST_DURATION_MILLIS / DELAY_MILLIS,
          elapsed);
      COROUTINE_END();
    }
  }
}

COROUTINE(countWithDelaySeconds) {
  static volatile unsigned long counter = 0;
  static uint16_t loopCounter = 0;
  COROUTINE_LOOP() {
    counter++;
    COROUTINE_DELAY_SECONDS(loopCounter, DELAY_SECONDS);
    unsigned long elapsed = millis() - startMillis;
    if (elapsed >= TEST_DURATION_MILLIS) {
      Serial.print("countWithDelaySeconds(): ");
      printResults(counter, TEST_DURATION_MILLIS / (DELAY_SECONDS * 1000),
          elapsed);
      COROUTINE_END();
    }
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  startMillis = millis();
}

// Manually execute the coroutines.
void loop() {
  countWithDelayMicros.runCoroutine();
  countWithDelayMillis.runCoroutine();
  countWithDelaySeconds.runCoroutine();
}
