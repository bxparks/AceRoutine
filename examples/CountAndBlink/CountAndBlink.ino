/*
 * This sketch demonstrates how 4 routines can be used to blink and count at the
 * same time. It blinks quickly when counting, and slowly when finished
 * counting.
 */

#include <AceRoutine.h>
using namespace ace_routine;

const int LED = LED_BUILTIN;
const int LED_ON = HIGH;
const int LED_OFF = LOW;

// Initially a fast blink rate
int ledDelayMillis = 100;

// Blink the LED at a certain frequency controlled by ledDelayMillis.
ROUTINE(blinkLed) {
  ROUTINE_LOOP() {
    digitalWrite(LED, LED_ON);
    ROUTINE_DELAY(ledDelayMillis);
    digitalWrite(LED, LED_OFF);
    ROUTINE_DELAY(ledDelayMillis);
  }
}

// Count from 0 to 20 with 500 ms delay between iteration
ROUTINE(countTo10) {
  ROUTINE_BEGIN();

  Serial.println(F("countTo10: Hello!"));
  ROUTINE_YIELD();

  static int i = 0;
  for (i = 0; i < 10; i++) {
    Serial.print("countTo10: ");
    Serial.println(i);
    ROUTINE_DELAY(1000);
  }

  ROUTINE_END();
}

// Count from 0 to 10 with 1000 ms delay between iteration
ROUTINE(countTo20) {
  ROUTINE_BEGIN();

  Serial.println(F("countTo20: Hello!"));
  ROUTINE_YIELD();

  static int i = 0;
  for (i = 0; i < 20; i++) {
    Serial.print(F("countTo20: "));
    Serial.println(i);
    ROUTINE_DELAY(500);
  }

  ROUTINE_END();
}

// Wait for countTo10 and countTo20 to finish, then slow down the LED blink
// rate.
ROUTINE(slowDownBlinking) {
  ROUTINE_BEGIN();

  Serial.println(F("slowDownBlinking: Hello!"));
  Serial.println(F("slowDownBlinking: Waiting for countTo10 and countTo20..."));
  ROUTINE_AWAIT(countTo10.isDone());
  ROUTINE_AWAIT(countTo20.isDone());
  Serial.println(F("slowDownBlinking: Slowing LED blink rate."));
  ledDelayMillis = 600;

  ROUTINE_END();
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  RoutineScheduler::setup();
}

void loop() {
  RoutineScheduler::loop();
}
