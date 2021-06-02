/*
 * This sketch is the same as BlinkSlowFastRoutine except that it uses custom
 * Coroutine classes. The custom classes are able to support additional
 * variables and methods which allow the coroutines to communicate with each
 * other.
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

/** A coroutine that blinks the LED fast. */
class BlinkFastCoroutine : public Coroutine {
  public:
    void enable(bool enable) { enabled = enable; }

    int runCoroutine() override {
      COROUTINE_LOOP() {
        if (enabled) {
          Serial.print("F1 ");
          digitalWrite(LED, LED_ON);
          COROUTINE_DELAY(200);
          Serial.print("F1a ");

          Serial.print("F0 ");
          digitalWrite(LED, LED_OFF);
          COROUTINE_DELAY(200);
          Serial.print("F0a ");
        } else {
          Serial.print("F ");
          COROUTINE_DELAY(400);
        }
      }
    }

  private:
    bool enabled = false;
};

/** A coroutine that blinks the LED slowly. */
class BlinkSlowCoroutine : public Coroutine {
  public:
    void enable(bool enable) { enabled = enable; }

    int runCoroutine() override {
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

  private:
    bool enabled = false;
};

/**
 * A coroutine that reads a digitalRead(), performs a primitive debouncing
 * by waiting 20 ms in a non-blocking manner, so that the other coroutines
 * continue to run while waiting. Clicking on a button (BUTTON_PIN) cycles
 * through the LED blinking pattern:
 *
 *   * Blink both
 *   * Blink fast only
 *   * Blink slow only
 *
 *  You probably shouldn't use a button debouncing code like this in your real
 *  project. Consider using https://github.com/bxparks/AceButton instead.
 */
class ButtonCoroutine : public Coroutine {
  public:
    ButtonCoroutine(
      BlinkFastCoroutine& blinkFastCoroutine,
      BlinkSlowCoroutine& blinkSlowCoroutine
    ):
      blinkFast(blinkFastCoroutine),
      blinkSlow(blinkSlowCoroutine)
    {}

    int runCoroutine() override {
      COROUTINE_BEGIN();

      configureBlinkers();
      COROUTINE_YIELD();

      while (true) {
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

  protected:
    static const int BLINK_STATE_SLOW = 0;
    static const int BLINK_STATE_FAST = 1;
    static const int BLINK_STATE_BOTH = 2;

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

  private:
    BlinkFastCoroutine& blinkFast;
    BlinkSlowCoroutine& blinkSlow;

    int buttonState = HIGH;
    int prevButtonState = HIGH;
    int blinkState = BLINK_STATE_BOTH;
};

// Create instances of the coroutines, and hook them up
BlinkFastCoroutine blinkFast;
BlinkSlowCoroutine blinkSlow;
ButtonCoroutine buttonCoroutine(blinkFast, blinkSlow);

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
