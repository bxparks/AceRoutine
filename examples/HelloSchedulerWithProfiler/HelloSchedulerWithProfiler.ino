/*
HelloSchedulerWithProfiler, a demo of the LogBinProfiler. The CoroutineScheduler
will automatically use the Profiler if it is defined on the Coroutine.

Every 5 seconds, the following will be printed on the Serial monitor.

**SparkFun Pro Micro**

* flash/ram = 7454/271

```
name         <16us <32us <64us<128us<256us<512us  <1ms  <2ms  <4ms  <8ms    >>
0x1DB        16898 52688     0     0     0     0     0     0     0     0     1
readPin      65535  1128     0     0     0     0     0     0     0     0     0
blinkLed     65535   800     0     0     0     0     0     0     0     0     0
{
"0x1DB":[16898,52688,0,0,0,0,0,0,0,0,1],
"readPin":[65535,1128,0,0,0,0,0,0,0,0,0],
"blinkLed":[65535,800,0,0,0,0,0,0,0,0,0]
}
```

**ESP8266**

```
name          <8us <16us <32us <64us<128us<256us<512us  <1ms  <2ms  <4ms    >>
0x3FFEE4D0   65535    37     2     0     0     0     0     0     0     0     1
readPin      65535   274     0     0     0     0     0     0     0     0     0
blinkLed     65535    47     0     0     0     0     0     0     0     0     0
{
"0x3FFEE4D0":[65535,37,2,0,0,0,0,0,0,0,1],
"readPin":[65535,274,0,0,0,0,0,0,0,0,0],
"blinkLed":[65535,47,0,0,0,0,0,0,0,0,0]
}
```
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

const int PIN = 2;
const int LED_ON = HIGH;
const int LED_OFF = LOW;

COROUTINE(blinkLed) {
  COROUTINE_LOOP() {
    digitalWrite(LED, LED_ON);
    COROUTINE_DELAY(100);
    digitalWrite(LED, LED_OFF);
    COROUTINE_DELAY(500);
  }
}

COROUTINE(readPin) {
  COROUTINE_LOOP() {
    (void) digitalRead(PIN);
    COROUTINE_DELAY(20);
  }
}

// A coroutine that prints the profiling information every 5 seconds. It will
// include information about itself.
COROUTINE(printProfiling) {
  COROUTINE_LOOP() {
    LogBinTableRenderer tableRenderer(Coroutine::getRoot());
    LogBinJsonRenderer jsonRenderer(Coroutine::getRoot());

  #if defined(ARDUINO_ARCH_AVR)
    tableRenderer.printTo(
        Serial, 3 /*startBin*/, 14 /*endBin*/, false /*clear*/);
    jsonRenderer.printTo(
        Serial, 3 /*startBin*/, 14 /*endBin*/);
  #else
    tableRenderer.printTo(
        Serial, 2 /*startBin*/, 13 /*endBin*/, false /*clear*/);
    jsonRenderer.printTo(
        Serial, 2 /*startBin*/, 13 /*endBin*/);
  #endif

    COROUTINE_DELAY(5000);
  }
}

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  pinMode(LED, OUTPUT);
  pinMode(PIN, INPUT);

  // Coroutine names can be either C-string or F-string.
  blinkLed.setName("blinkLed");
  readPin.setName(F("readPin"));

  // Create a profiler on the heap for every coroutine.
  LogBinProfiler::createProfilers(Coroutine::getRoot());

  // Setup the scheduler.
  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
