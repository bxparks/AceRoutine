/*
 * HelloScheduler. Same as HelloCoroutine, but using the CoroutineScheduler.
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

COROUTINE(blinkLed) {
  COROUTINE_LOOP() {
    digitalWrite(LED, LED_ON);
    COROUTINE_DELAY(100);
    digitalWrite(LED, LED_OFF);
    COROUTINE_DELAY(500);
  }
}

COROUTINE(printHelloWorld) {
  COROUTINE_LOOP() {
    Serial.print(F("Hello, "));
    Serial.flush();
    COROUTINE_DELAY(1000);
    Serial.println(F("World"));
    COROUTINE_DELAY(4000);
  }
}

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro
  pinMode(LED, OUTPUT);

  // Auto-register all coroutines into the scheduler.
  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
