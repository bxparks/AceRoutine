/*
 * HelloCoroutine. Use 2 coroutines to print "Hello, World", the hard way.
 * A 3rd coroutine spins away on the side, blinking the LED.
 */

#include <Arduino.h>
#include <AceRoutine.h>
using namespace ace_routine;

#ifdef LED_BUILTIN
  const int LED = LED_BUILTIN;
#else
  // Some ESP32 boards do not define LED_BUILTIN. Sometimes they have more than
  // one built-in LEDs. Replace this with the pin number of your LED.
  const int LED = 5;
#endif

const int LED_ON = HIGH;
const int LED_OFF = LOW;

// Use asymmetric delays to demonstrate that COROUTINES eliminate the need to
// keep track of the blinking states explicitly.
const int LED_ON_DELAY = 100;
const int LED_OFF_DELAY = 500;

COROUTINE(blinkLed) {
  COROUTINE_LOOP() {
    digitalWrite(LED, LED_ON);
    COROUTINE_DELAY(LED_ON_DELAY);
    digitalWrite(LED, LED_OFF);
    COROUTINE_DELAY(LED_OFF_DELAY);
  }
}

COROUTINE(printHello) {
  COROUTINE_BEGIN();

  Serial.print(F("Hello, "));
  Serial.flush();
  COROUTINE_DELAY(2000);

  COROUTINE_END();
}

COROUTINE(printWorld) {
  COROUTINE_BEGIN();

  COROUTINE_AWAIT(printHello.isDone());
  Serial.println(F("World!"));

  COROUTINE_END();
}

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000);
#endif
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro
  pinMode(LED, OUTPUT);
}

// Manually execute the coroutines.
void loop() {
  blinkLed.runCoroutine();
  printHello.runCoroutine();
  printWorld.runCoroutine();
}
