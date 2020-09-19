/*
 * HelloManualCoroutine. Same as HelloCoroutine but using explicit Coroutine
 * subclasses and manually creating the coroutine instances.
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

class BlinkLedCoroutine: public Coroutine {
  public:
    int runCoroutine() override {
      COROUTINE_LOOP() {
        digitalWrite(LED, LED_ON);
        COROUTINE_DELAY(100);
        digitalWrite(LED, LED_OFF);
        COROUTINE_DELAY(500);
      }
    }
};

class PrintHelloWorldCoroutine: public Coroutine {
  public:
    int runCoroutine() override {
      COROUTINE_LOOP() {
        Serial.print(F("Hello, "));
        Serial.flush();
        COROUTINE_DELAY(1000);
        Serial.println(F("World"));
        COROUTINE_DELAY_SECONDS(4);
      }
    }
};

BlinkLedCoroutine blinkLed;
PrintHelloWorldCoroutine printHelloWorld;

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000);
#endif
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro
  pinMode(LED, OUTPUT);

  // Register the coroutines manually since we are not using COROUTINE() macro.
  blinkLed.setupCoroutine("blinkLed");
  printHelloWorld.setupCoroutine("printHelloWorld");

  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
