/*
 * This sketch uses 3 coroutines:
 *    * the 'blinkSlow' coroutine to blink the LED slowly.
 *    * the 'blinkFast' coroutine blinks the LED quickly.
 *    * The 'button' coroutine scans the D2 pin for a button press. Each time
 *    the button is pressed, the blink mode cycles from "both", "fast" only, to
 *    "slow" only, then "both" again.
 *
 * Communication between the coroutines happens through the 'blinkState' global
 * variable. See BlinkCustomCoroutine to see how this communication can happen
 * through methods on custom Coroutine classes.
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

// Replace with the pin of your button.
const int BUTTON_PIN = 2;

// If the LED of your board is wired in reverse, flip these values.
const int LED_ON = HIGH;
const int LED_OFF = LOW;

const int BLINK_STATE_SLOW = 0;
const int BLINK_STATE_FAST = 1;
const int BLINK_STATE_BOTH = 2;
int blinkState = BLINK_STATE_SLOW;

COROUTINE(blinkSlow) {
  COROUTINE_LOOP() {
    switch (blinkState) {
      case BLINK_STATE_BOTH:
      case BLINK_STATE_SLOW:
        Serial.print("S1 ");
        digitalWrite(LED, LED_ON);
        COROUTINE_DELAY(500);
        Serial.print("S1a ");

        Serial.print("S0 ");
        digitalWrite(LED, LED_OFF);
        COROUTINE_DELAY(500);
        Serial.print("S0a ");
        break;

      default:
        Serial.print("S ");
        COROUTINE_DELAY(1000);
    }
  }
}

COROUTINE(blinkFast) {
  COROUTINE_LOOP() {
    switch (blinkState) {
      case BLINK_STATE_BOTH:
      case BLINK_STATE_FAST:
        Serial.print("F1 ");
        digitalWrite(LED, LED_ON);
        COROUTINE_DELAY(200);
        Serial.print("F1a ");

        Serial.print("F0 ");
        digitalWrite(LED, LED_OFF);
        COROUTINE_DELAY(200);
        Serial.print("F0a ");
        break;

      default:
        Serial.print("F ");
        COROUTINE_DELAY(400);
    }
  }
}

COROUTINE(button) {
  static int buttonState = HIGH;
  static int prevButtonState = HIGH;

  COROUTINE_LOOP() {
    buttonState = digitalRead(BUTTON_PIN);
    if (prevButtonState == HIGH && buttonState == LOW) {
      // primitive debouncing
      COROUTINE_DELAY(20);
      buttonState = digitalRead(BUTTON_PIN);
      if (prevButtonState == HIGH && buttonState == LOW) {
        blinkState++;
        if (blinkState >= 3) {
          blinkState = 0;
        }
        if (blinkState == BLINK_STATE_BOTH) {
          Serial.println("both");
        } else if (blinkState == BLINK_STATE_FAST) {
          Serial.println("fast");
        } else if (blinkState == BLINK_STATE_SLOW) {
          Serial.println("slow");
        }
      }
    }
    prevButtonState = buttonState;
    COROUTINE_DELAY(20);
  }
}

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  pinMode(LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
