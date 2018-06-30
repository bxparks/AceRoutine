/*
 * HelloCoroutine. Use 2 coroutines to print "Hello, World", the hard way.
 * A 3rd coroutine spins away on the side, blinking the LED.
 */

#include <AceRoutine.h>
using namespace ace_routine;

#ifdef LED_BUILTIN
  const int LED = LED_BUILTIN;
#else
  // Some ESP32 boards do not LED_BUILTIN, sometimes they have more than 1.
  const int LED = 5;
#endif

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
}

// Manually execute the coroutines.
void loop() {
  blinkLed.run();
  printHello.run();
  printWorld.run();
}
