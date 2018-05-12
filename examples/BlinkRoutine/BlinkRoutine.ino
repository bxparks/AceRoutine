/*
 * This sketch uses the 'blinkSlow' routine to blink the LED slowly.
 * The 'blinkFast' routine blinks the LED quickly.
 * The 'button' routine scans the D2 pin for a button press. Each time
 * the button is pressed, the blink mode cycles from "both", "fast" only,
 * to "slow" only, then "both" again.
 *
 * Communication between the routines happens through the 'blinkState' global
 * variable. See BlinkCustomRoutine to see how this communication can happen
 * through methods on custom Routine classes.
 */

#include <AceRoutine.h>
using namespace ace_routine;

const int LED = LED_BUILTIN;
const int LED_ON = HIGH;
const int LED_OFF = LOW;
const int BUTTON_PIN = 2;

const int BLINK_STATE_BOTH = 0;
const int BLINK_STATE_FAST = 1;
const int BLINK_STATE_SLOW = 2;
int blinkState = BLINK_STATE_BOTH;

ROUTINE(blinkSlow) {
  ROUTINE_LOOP() {
    switch (blinkState) {
      case BLINK_STATE_BOTH:
      case BLINK_STATE_SLOW:
        Serial.print("S1 ");
        digitalWrite(LED, LED_ON);
        ROUTINE_DELAY(500);
        Serial.print("S1a ");

        Serial.print("S0 ");
        digitalWrite(LED, LED_OFF);
        ROUTINE_DELAY(500);
        Serial.print("S0a ");
        break;

      default:
        Serial.print("S ");
        ROUTINE_DELAY(1000);
    }
  }
}

ROUTINE(blinkFast) {
  ROUTINE_LOOP() {
    switch (blinkState) {
      case BLINK_STATE_BOTH:
      case BLINK_STATE_FAST:
        Serial.print("F1 ");
        digitalWrite(LED, LED_ON);
        ROUTINE_DELAY(300);
        Serial.print("F1a ");

        Serial.print("F0 ");
        digitalWrite(LED, LED_OFF);
        ROUTINE_DELAY(300);
        Serial.print("F0a ");
        break;

      default:
        Serial.print("F ");
        ROUTINE_DELAY(600);
    }
  }
}

ROUTINE(button) {
  static int buttonState;
  static int prevButtonState;

  ROUTINE_LOOP() {
    buttonState = digitalRead(BUTTON_PIN);
    if (prevButtonState == HIGH && buttonState == LOW) {
      // primitive debouncing
      ROUTINE_DELAY(20);
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
    ROUTINE_DELAY(20);
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  pinMode(LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  RoutineScheduler::setup();
}

void loop() {
  RoutineScheduler::loop();
}
