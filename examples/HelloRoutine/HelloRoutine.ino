/*
 * HelloRoutine. Use 2 Routines to print "Hello, World", the hard way.
 * A 3rd Routine spins away on the side, blinking the LED.
 */

#include <AceRoutine.h>
using namespace ace_routine;

const int LED = LED_BUILTIN;
const int LED_ON = HIGH;
const int LED_OFF = LOW;

const int LED_DELAY = 200;

ROUTINE(blinkLed) {
  ROUTINE_LOOP() {
    digitalWrite(LED, LED_ON);
    ROUTINE_DELAY(LED_DELAY);
    digitalWrite(LED, LED_OFF);
    ROUTINE_DELAY(LED_DELAY);
  }
}

ROUTINE(printHello) {
  ROUTINE_BEGIN();

  Serial.print(F("Hello, "));
  ROUTINE_DELAY(1000);

  ROUTINE_END();
}

ROUTINE(printWorld) {
  ROUTINE_BEGIN();

  ROUTINE_AWAIT(printHello.isTerminated());
  Serial.println(F("World!"));

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
