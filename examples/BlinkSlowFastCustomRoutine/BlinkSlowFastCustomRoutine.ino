/*
 * This sketch is the same as BlinkCoroutine except that it uses the 2-argument
 * version of COROUTINE() to use custom Coroutine classes. The custom classes
 * are able to support additional variables and methods which are used by the
 * coroutines to communicate with each other.
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
const int BUTTON_PIN = 2;

class BlinkCoroutine : public Coroutine {
  public:
    void enable(bool enable) { enabled = enable; }

  protected:
    bool enabled = 0;
};

COROUTINE(BlinkCoroutine, blinkSlow) {
  COROUTINE_LOOP() {
    if (enabled) {
      digitalWrite(LED, LED_ON);
      Serial.print("S1 ");
      COROUTINE_DELAY(500);
      Serial.print("S1a ");

      Serial.print("S0 ");
      digitalWrite(LED, LED_OFF);
      COROUTINE_DELAY(500);
      Serial.print("S0a ");
    } else {
      Serial.print("S ");
      COROUTINE_DELAY(1000);
    }
  }
}

COROUTINE(BlinkCoroutine, blinkFast) {
  COROUTINE_LOOP() {
    if (enabled) {
      Serial.print("F1 ");
      digitalWrite(LED, LED_ON);
      COROUTINE_DELAY(300);
      Serial.print("F1a ");

      Serial.print("F0 ");
      digitalWrite(LED, LED_OFF);
      COROUTINE_DELAY(300);
      Serial.print("F0a ");
    } else {
      Serial.print("F ");
      COROUTINE_DELAY(600);
    }
  }
}

class ButtonCoroutine : public Coroutine {
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

int ButtonCoroutine::blinkState = BLINK_STATE_BOTH;

COROUTINE(ButtonCoroutine, button) {
  COROUTINE_BEGIN();

  configureBlinkers();
  COROUTINE_YIELD();

  while (true ) {
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
        configureBlinkers();
      }
    }
    prevButtonState = buttonState;
    COROUTINE_DELAY(20);
  }

  COROUTINE_END();
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  pinMode(LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
