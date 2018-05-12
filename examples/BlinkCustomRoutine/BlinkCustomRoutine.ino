/*
 * This sketch is the same as BlinkRoutine except that it uses the 2-argument
 * version of ROUTINE() to create routines which using custom Routine
 * classes. The custom classes are able to support additional variables and
 * methods that can be used by the routines to communicate with each other.
 */

#include <AceRoutine.h>
using namespace ace_routine;

const int LED = LED_BUILTIN;
const int LED_ON = HIGH;
const int LED_OFF = LOW;
const int BUTTON_PIN = 2;

class BlinkRoutine : public Routine {
  public:
    void enable(bool enable) { enabled = enable; }

  protected:
    bool enabled = 0;
};

ROUTINE(BlinkRoutine, blinkSlow) {
  ROUTINE_LOOP() {
    if (enabled) {
      digitalWrite(LED, LED_ON);
      Serial.print("S1 ");
      ROUTINE_DELAY(500);
      Serial.print("S1a ");

      Serial.print("S0 ");
      digitalWrite(LED, LED_OFF);
      ROUTINE_DELAY(500);
      Serial.print("S0a ");
    } else {
      Serial.print("S ");
      ROUTINE_DELAY(1000);
    }
  }
}

ROUTINE(BlinkRoutine, blinkFast) {
  ROUTINE_LOOP() {
    if (enabled) {
      Serial.print("F1 ");
      digitalWrite(LED, LED_ON);
      ROUTINE_DELAY(300);
      Serial.print("F1a ");

      Serial.print("F0 ");
      digitalWrite(LED, LED_OFF);
      ROUTINE_DELAY(300);
      Serial.print("F0a ");
    } else {
      Serial.print("F ");
      ROUTINE_DELAY(600);
    }
  }
}

class ButtonRoutine : public Routine {
  protected:
    static const int BLINK_STATE_BOTH = 0;
    static const int BLINK_STATE_FAST = 1;
    static const int BLINK_STATE_SLOW = 2;
    static int blinkState;

    void configureBlinkers() {
      if (blinkState == BLINK_STATE_BOTH) {
        Serial.println("both");
        blinkFast.enable(true);
        blinkSlow.enable(true);
      } else if (blinkState == BLINK_STATE_FAST) {
        Serial.println("fast");
        blinkSlow.enable(false);
        blinkFast.enable(true);
      } else if (blinkState == BLINK_STATE_SLOW) {
        Serial.println("slow");
        blinkSlow.enable(true);
        blinkFast.enable(false);
      }
    }

    int buttonState = BLINK_STATE_BOTH;
    int prevButtonState = HIGH;
};

int ButtonRoutine::blinkState = BLINK_STATE_BOTH;

ROUTINE(ButtonRoutine, button) {
  ROUTINE_BEGIN();

  configureBlinkers();
  ROUTINE_YIELD();

  while (true ) {
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
        configureBlinkers();
      }
    }
    prevButtonState = buttonState;
    ROUTINE_DELAY(20);
  }

  ROUTINE_END();
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
