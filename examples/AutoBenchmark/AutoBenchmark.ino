/*
 * This sketch attempts to find how much overhead is introduced by the context
 * switching performed by the CoroutineScheduler among the various coroutines.
 *
 * For the Teensy ARM microcontrollers, the benchmark numbers seem highly
 * dependent on compiler optimizer settings.
 */

#include <AceRoutine.h>
using namespace ace_routine;

const unsigned long DURATION = 5000;

unsigned long counter = 0;

COROUTINE(counterA) {
  COROUTINE_LOOP() {
    counter++;
    COROUTINE_YIELD();
  }
}

COROUTINE(counterB) {
  COROUTINE_LOOP() {
    counter++;
    COROUTINE_YIELD();
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  CoroutineScheduler::setup();
  CoroutineScheduler::list(Serial);

  Serial.print(F("sizeof(Coroutine): "));
  Serial.println(sizeof(Coroutine));
  Serial.print(F("sizeof(CoroutineScheduler): "));
  Serial.println(sizeof(CoroutineScheduler));

  Serial.println(
      F("------------+------+------+"));
  Serial.println(
      F(" AceRoutine | base | diff |"));
  Serial.println(
      F("------------+------+------+"));

  doBaseline();
  float baseline = DURATION * 1000.0 / counter;

  doAceRoutine();
  float aceCoroutine = DURATION * 1000.0 / counter;

  printStats(baseline, aceCoroutine);
  Serial.println(
      F("------------+------+------+"));
}

void printStats(float baseline, float aceCoroutine) {
  char buf[100];
  float diff = aceCoroutine - baseline;
  sprintf(buf, "      %2d.%02d |%2d.%02d |%2d.%02d |",
      (int)aceCoroutine, (int)(aceCoroutine*100)%100,
      (int)baseline, (int)(baseline*100)%100,
      (int)diff, (int)(diff*100)%100);
  Serial.println(buf);
}

void doAceRoutine() {
  counter = 0;
  unsigned long start = millis();
  while (millis() - start < DURATION) {
    CoroutineScheduler::loop();
    yield();
  }
}

void doBaseline() {
  counter = 0;
  unsigned long start = millis();
  while (millis() - start < DURATION) {
    counter++;
    yield();
  }
}

void loop() {
}
