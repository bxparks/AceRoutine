/*
 * A sketch that reads the serial port for a line and calls the event handler.
 *
 * WORK IN PROGRESS
 */

#include <Arduino.h>
#include <AceRoutine.h>
#include "ace_routine/cli/SerialReader.h"
#include "ace_routine/cli/CommandDispatcher.h"
using namespace ace_routine;
using namespace ace_routine::cli;

//---------------------------------------------------------------------------

#ifdef LED_BUILTIN
  const int LED = LED_BUILTIN;
#else
  // Some ESP32 boards do not LED_BUILTIN, sometimes they have more than 1.
  const int LED = 5;
#endif

const int LED_ON = HIGH;
const int LED_OFF = LOW;

// Use asymmetric delays for blinking to demonstrate how easy it is using
// COROUTINES.
int ledOnDelay = 100;
int ledOffDelay = 500;

COROUTINE(blinkLed) {
  COROUTINE_LOOP() {
    digitalWrite(LED, LED_ON);
    COROUTINE_DELAY(ledOnDelay);
    digitalWrite(LED, LED_OFF);
    COROUTINE_DELAY(ledOffDelay);
  }
}

//---------------------------------------------------------------------------

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else
extern char *__brkval;
#endif
 
/**
 * Return the amount of free memory. Taken from:
 *
 * - https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
 * - https://arduino.stackexchange.com/questions/30497
 * - https://github.com/mpflaga/Arduino-MemoryFree/blob/master/MemoryFree.cpp
 *
 * For a Nano, the original code returns 2252, which seems too high since it
 * has only 2048 of static RAM. Changed to always test for non-zero value of
 * __brkval, which gives 1553 which is far closer because the Arduino IDE says
 * that the sketch leaves 1605 bytes for RAM.
 */
int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#else
  return &top - (__brkval ? __brkval : __malloc_heap_start);
#endif
}

//---------------------------------------------------------------------------

SerialReader serialReader(Serial);

/** List the coroutines. */
void listCommandHandler(int /* argc */, const char** /* argv */) {
  CoroutineScheduler::list(&Serial);
}

/** Echo the command line arguments. */
void echoCommandHandler(int argc, const char** argv) {
 for (int i = 1; i < argc; i++) {
    Serial.print(argv[i]);
    Serial.print(' ');
  }
  Serial.println();
}

/** Print amount of free memory between stack and heap. */
void freeCommandHandler(int /* argc */, const char** /* argv */) {
	Serial.print(F("Free memory: "));
	Serial.println(freeMemory());
}

/** Change the blinking LED on and off delay parameters. */
void delayCommandHandler(int argc, const char** argv) {
  if (argc != 3) {
    Serial.println(F("Invalid number of arguments"));
    return;
  }
  const char* param = argv[1];
  const char* value = argv[2];
  if (strcmp(param, "on") == 0) {
    ledOnDelay = atoi(value);
  } else if (strcmp(param, "off") == 0) {
    ledOffDelay = atoi(value);
  } else {
    Serial.print(F("Unknown argument: "));
    Serial.println(param);
  }
}

static const CommandDispatcher::DispatchRecord dispatchTable[] = {
  {delayCommandHandler, "delay", "(on | off) millis"},
  {listCommandHandler, "list", nullptr},
  {freeCommandHandler, "free", nullptr},
  {echoCommandHandler, "echo", "args ..."},
};

CommandDispatcher dispatcher(
    serialReader,
    sizeof(dispatchTable) / sizeof(CommandDispatcher::DispatchRecord),
    dispatchTable);

//---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  if (!Serial) {} // micro/leonardo

  dispatcher.resume(); // insert into the scheduler
  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
