/*
 * A demo sketch for the src/ace_routine/cli/ classes. It reads the serial port
 * for a command, parses the command line, then calls the appropriate command
 * handler.
 *
 * Run the sketch, then type 'help' on the serial port. The following
 * commands are supported:
 *  - `help [command]` - list the known commands
 *  - `list` - list the coroutines managed by the CoroutineScheduler
 *  - `free` - print free memory
 *  - `echo [args ...]` - echo the arguments
 *  - `delay (on | off) millis` - set LED blink on or off delay
 */

#include <Arduino.h>
#include <AceRoutine.h>
#include "ace_routine/cli/CommandManager.h"

using namespace ace_routine;
using namespace ace_routine::cli;

//---------------------------------------------------------------------------

// Select USE_F_STRING to use FlashStrings (const __FlashString*) which
// stores the strings in flash memory. Useful for AVR boards with
// small static RAM.
//
// Select USE_C_STRING to use C-strings (const char*) for all others because
// they have more memory. ESP8266 has some support for Flash strings but it's
// buggy, conflicts with Flash strings in templates. Easier to avoid.
#define USE_C_STRING 1
#define USE_F_STRING 2

#if defined(AVR) || defined(__arm__) || defined(ESP32)
  #define STRING_MODE USE_F_STRING
  #define FF(x) F(x)
#elif defined(ESP8266)
  #define STRING_MODE USE_C_STRING
  #define FF(x) (x)
#else
  #error Unsupported board
#endif

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
#elif defined(ESP8266)
extern "C" {
#include "user_interface.h"
}
#else
extern char *__brkval;
#endif

/**
 * Return the amount of free memory. For AVR and Teensy, see:
 *
 * - https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
 * - https://arduino.stackexchange.com/questions/30497
 * - https://github.com/mpflaga/Arduino-MemoryFree/blob/master/MemoryFree.cpp
 *
 * For a Nano, the original code returns 2252, which seems too high since it
 * has only 2048 of static RAM. Changed to always test for non-zero value of
 * __brkval, which gives 1553 which seems more accurate because the Arduino
 * IDE says that the sketch leaves 1605 bytes for RAM.
 *
 * For ESP8266, see:
 * - https://github.com/esp8266/Arduino/issues/81
 *
 * For ESP32, see:
 * - https://techtutorialsx.com/2017/12/17/esp32-arduino-getting-the-free-heap/
 */
unsigned long freeMemory() {
#ifdef __arm__
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(ESP8266)
  return system_get_free_heap_size();
#elif defined(ESP32)
  return ESP.getFreeHeap();
#else
  char top;
  return &top - (__brkval ? __brkval : __malloc_heap_start);
#endif
}

//---------------------------------------------------------------------------

/** List the coroutines known by the CoroutineScheduler. */
void listCommand(Print& printer, int /* argc */, const char** /* argv */) {
  CoroutineScheduler::list(printer);
}

/** Echo the command line arguments. */
void echoCommand(Print& printer, int argc, const char** argv) {
 for (int i = 1; i < argc; i++) {
    printer.print(argv[i]);
    printer.print(' ');
  }
  printer.println();
}

/** Print amount of free memory between stack and heap. */
void freeCommand(Print& printer, int /* argc */, const char** /* argv */) {
  printer.print(FF("Free memory: "));
  printer.println(freeMemory());
}

/** Change the blinking LED on and off delay parameters. */
void delayCommand(Print& printer, int argc, const char** argv) {
  if (argc != 3) {
    printer.println(FF("Invalid number of arguments"));
    return;
  }
  const char* param = argv[1];
  const char* value = argv[2];
  if (strcmp(param, "on") == 0) {
    ledOnDelay = atoi(value);
  } else if (strcmp(param, "off") == 0) {
    ledOffDelay = atoi(value);
  } else {
    printer.print(FF("Unknown argument: "));
    printer.println(param);
  }
}

const uint8_t TABLE_SIZE = 4;
const uint8_t BUF_SIZE = 64;
const uint8_t ARGV_SIZE = 10;
const char PROMPT[] = "$ ";

#if STRING_MODE == USE_C_STRING
CommandManager<char, BUF_SIZE, ARGV_SIZE>
    commandManager(Serial, TABLE_SIZE, PROMPT);
#else
CommandManager<__FlashStringHelper, BUF_SIZE, ARGV_SIZE>
    commandManager(Serial, TABLE_SIZE, PROMPT);
#endif

//---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  while (!Serial); // micro/leonardo

  // add commands to CommandManager
#if STRING_MODE == USE_C_STRING
  commandManager.add(delayCommand, "delay", "(on | off) millis");
  commandManager.add(listCommand, "list", nullptr);
  commandManager.add(freeCommand, "free", nullptr);
  commandManager.add(echoCommand, "echo", "args ...");
#else
  commandManager.add(delayCommand, F("delay"), F("(on | off) millis"));
  commandManager.add(listCommand, F("list"), nullptr);
  commandManager.add(freeCommand, F("free"), nullptr);
  commandManager.add(echoCommand, F("echo"), F("args ..."));
#endif

  commandManager.resume(); // insert into the scheduler
  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
