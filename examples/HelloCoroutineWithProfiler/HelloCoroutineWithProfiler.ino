/*
HelloCoroutineWithProfiler, a demo of the LogBinProfiler. This technique can be
used on resource constrained 8-bit environments which do not want to include the
overhead of the CoroutineScheduler.

Every 5 seconds, the, following will be printed on the Serial monitor.

**SparkFun Pro Micro**

* flash/ram = 6840/457

```
name         <16us <32us <64us<128us<256us<512us  <1ms  <2ms  <4ms  <8ms    >>
0x1DB        16921 52650     0     0     0     0     0     0     0     0     1
readPin      65535  1189     0     0     0     0     0     0     0     0     0
blinkLed     65535   830     0     0     0     0     0     0     0     0     0
{
"0x1DB":[16921,52650,0,0,0,0,0,0,0,0,1],
"readPin":[65535,1189,0,0,0,0,0,0,0,0,0],
"blinkLed":[65535,830,0,0,0,0,0,0,0,0,0]
}
```

**ESP8266**

```
name          <8us <16us <32us <64us<128us<256us<512us  <1ms  <2ms  <4ms    >>
0x3FFEE4D0   65535    32     0     0     0     0     0     0     0     0     1
readPin      65535   275     1     0     0     0     0     0     0     0     0
blinkLed     65535    48     3     0     0     0     0     0     0     0     0
{
"0x3FFEE4D0":[65535,32,0,0,0,0,0,0,0,0,1],
"readPin":[65535,275,1,0,0,0,0,0,0,0,0],
"blinkLed":[65535,48,3,0,0,0,0,0,0,0,0]
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
  #if defined(ARDUINO_ARCH_AVR)
    LogBinTableRenderer::printTo(
        Serial, 3 /*startBin*/, 14 /*endBin*/, false /*clear*/);
    LogBinJsonRenderer::printTo(
        Serial, 3 /*startBin*/, 14 /*endBin*/);
  #else
    LogBinTableRenderer::printTo(
        Serial, 2 /*startBin*/, 13 /*endBin*/, false /*clear*/);
    LogBinJsonRenderer::printTo(
        Serial, 2 /*startBin*/, 13 /*endBin*/);
  #endif

    COROUTINE_DELAY(5000);
  }
}

LogBinProfiler profiler1;
LogBinProfiler profiler2;
LogBinProfiler profiler3;

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

  // Manually attach the profilers to the coroutines. Replaces
  // LogBinProfilers::createProfilers().
  blinkLed.setProfiler(&profiler1);
  readPin.setProfiler(&profiler2);
  printProfiling.setProfiler(&profiler3);
}

void loop() {
  blinkLed.runCoroutineWithProfiler();
  readPin.runCoroutineWithProfiler();
  printProfiling.runCoroutineWithProfiler();
}
