/*
 * Same as HelloCoroutine, but using the CoroutineScheduler.
 */

#include <AceRoutine.h>
using namespace ace_routine;

const int LED = LED_BUILTIN;
const int LED_ON = HIGH;
const int LED_OFF = LOW;

const int LED_DELAY = 200;

COROUTINE(blinkLed) {
  COROUTINE_LOOP() {
    digitalWrite(LED, LED_ON);
    COROUTINE_DELAY(LED_DELAY);
    digitalWrite(LED, LED_OFF);
    COROUTINE_DELAY(LED_DELAY);
  }
}

COROUTINE(printHello) {
  COROUTINE_BEGIN();

  Serial.print(F("Hello, "));
  COROUTINE_DELAY(1000);

  COROUTINE_END();
}

COROUTINE(printWorld) {
  COROUTINE_BEGIN();

  COROUTINE_AWAIT(printHello.isDone());
  Serial.println(F("World!"));

  COROUTINE_END();
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  // Auto-register all coroutines into the scheduler.
  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
