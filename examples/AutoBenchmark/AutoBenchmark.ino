/*
 * This sketch attempts to find how much overhead is introduced by the context
 * switching performed by the RoutineScheduler among the various routines.
 *
 * For the Teensy ARM microcontrollers, the benchmark numbers seem highly
 * dependent on compiler optimizer settings.
 */

#include <AceRoutine.h>
using namespace ace_routine;

const unsigned long DURATION = 5000;

unsigned long counter = 0;

ROUTINE(counterA) {
  ROUTINE_LOOP() {
    counter++;
    ROUTINE_YIELD();
  }
}

ROUTINE(counterB) {
  ROUTINE_LOOP() {
    counter++;
    ROUTINE_YIELD();
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  RoutineScheduler::setup();
  RoutineScheduler::list(&Serial);

  Serial.print(F("sizeof(Routine): "));
  Serial.println(sizeof(Routine));
  Serial.print(F("sizeof(RoutineScheduler): "));
  Serial.println(sizeof(RoutineScheduler));

  Serial.println(
      F("------------+------+------+"));
  Serial.println(
      F(" AceRoutine | base | diff |"));
  Serial.println(
      F("------------+------+------+"));

  doBaseline();
  float baseline = DURATION * 1000.0 / counter;

  doAceRoutine();
  float aceRoutine = DURATION * 1000.0 / counter;

  printStats(baseline, aceRoutine);
  Serial.println(
      F("------------+------+------+"));
}

void printStats(float baseline, float aceRoutine) {
  char buf[100];
  float diff = aceRoutine - baseline;
  sprintf(buf, "      %2d.%02d |%2d.%02d |%2d.%02d |",
      (int)aceRoutine, (int)(aceRoutine*100)%100,
      (int)baseline, (int)(baseline*100)%100,
      (int)diff, (int)(diff*100)%100);
  Serial.println(buf);
}

void doAceRoutine() {
  counter = 0;
  unsigned long start = millis();
  while (millis() - start < DURATION) {
    RoutineScheduler::loop();
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
