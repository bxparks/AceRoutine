/*
 * This sketch demonstrates how 4 coroutines can be used to blink and count at
 * the same time. It blinks quickly when counting, and slowly when finished
 * counting.
 */

#include <Arduino.h>
#include <AceRoutine.h>
using namespace ace_routine;

#ifdef LED_BUILTIN
  const int LED = LED_BUILTIN;
#else
  // Some ESP32 boards do not define LED_BUILTIN. Sometimes they have more than
  // 1. Replace this with the proper pin number.
  const int LED = 5;
#endif

const int LED_ON = HIGH;
const int LED_OFF = LOW;

// Initially a fast blink rate
int ledDelayMillis = 100;

// Blink the LED at a certain frequency controlled by ledDelayMillis.
COROUTINE(blinkLed) {
  COROUTINE_LOOP() {
    digitalWrite(LED, LED_ON);
    COROUTINE_DELAY(ledDelayMillis);
    digitalWrite(LED, LED_OFF);
    COROUTINE_DELAY(ledDelayMillis);
  }
}

// Count from 0 to 20 with 500 ms delay between iteration
COROUTINE(countTo10) {
  COROUTINE_BEGIN();

  Serial.println(F("countTo10: Hello!"));
  COROUTINE_YIELD();

  static int i = 0;
  for (i = 0; i < 10; i++) {
    Serial.print("countTo10: ");
    Serial.println(i);
    COROUTINE_DELAY(1000);
  }

  COROUTINE_END();
}

// Count from 0 to 10 with 1000 ms delay between iteration
COROUTINE(countTo20) {
  COROUTINE_BEGIN();

  Serial.println(F("countTo20: Hello!"));
  COROUTINE_YIELD();

  static int i = 0;
  for (i = 0; i < 20; i++) {
    Serial.print(F("countTo20: "));
    Serial.println(i);
    COROUTINE_DELAY(500);
  }

  COROUTINE_END();
}

// Wait for countTo10 and countTo20 to finish, then slow down the LED blink
// rate.
COROUTINE(slowDownBlinking) {
  COROUTINE_BEGIN();

  Serial.println(F("slowDownBlinking: Hello!"));
  Serial.println(F("slowDownBlinking: Waiting for countTo10 and countTo20..."));
  COROUTINE_AWAIT(countTo10.isDone());
  COROUTINE_AWAIT(countTo20.isDone());
  Serial.println(F("slowDownBlinking: Slowing LED blink rate."));
  ledDelayMillis = 600;

  COROUTINE_END();
}

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro
  pinMode(LED, OUTPUT);

  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
