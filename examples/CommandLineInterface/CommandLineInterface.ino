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
#include "ace_routine/cli/StreamReader.h"
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
  printer.print(F("Free memory: "));
  printer.println(freeMemory());
}

/** Change the blinking LED on and off delay parameters. */
void delayCommand(Print& printer, int argc, const char** argv) {
  if (argc != 3) {
    printer.println(F("Invalid number of arguments"));
    return;
  }
  const char* param = argv[1];
  const char* value = argv[2];
  if (strcmp(param, "on") == 0) {
    ledOnDelay = atoi(value);
  } else if (strcmp(param, "off") == 0) {
    ledOffDelay = atoi(value);
  } else {
    printer.print(F("Unknown argument: "));
    printer.println(param);
  }
}

// Select USE_C_STRING to use C-strings (const char*)
//
// Select USE_F_STRING to use FlashStrings (const __FlashString*) which
// stores the strings in flash memory, saving 48 bytes out of
// 457 bytes of statis RAM in this example.
#define USE_C_STRING 1
#define USE_F_STRING 2
#define STRING_MODE USE_F_STRING

#if STRING_MODE == USE_C_STRING

const DispatchRecordC dispatchTable[] = {
  {delayCommand, "delay", "(on | off) millis"},
  {listCommand, "list", nullptr},
  {freeCommand, "free", nullptr},
  {echoCommand, "echo", "args ..."},
};

const uint8_t numCommands = sizeof(dispatchTable) / sizeof(DispatchRecordC);

#else

/**
 * The F() macro works only inside methods, so we use this static function
 * to create and hold the dispatch table. Returns the pointer to the table
 * and sets the *numCommands to the number of entries.
 */
static const DispatchRecordF* getDispatchTable(uint8_t* numCommands) {
  static const DispatchRecordF dispatchTable[] = {
    {delayCommand, F("delay"), F("(on | off) millis")},
    {listCommand, F("list"), nullptr},
    {freeCommand, F("free"), nullptr},
    {echoCommand, F("echo"), F("args ...")},
  };

  *numCommands = sizeof(dispatchTable) / sizeof(DispatchRecordF);

  return dispatchTable;
}

uint8_t numCommands;
const DispatchRecordF* dispatchTable = getDispatchTable(&numCommands);

#endif

// Create an instance of the StreamReader.
const int BUF_SIZE = 64;
char lineBuffer[BUF_SIZE];
StreamReader streamReader(Serial, lineBuffer, BUF_SIZE);


// Create an instance of the CommandDispatcher using either CStrings or
// FStrings.
const int8_t ARGV_SIZE = 10;
const char* argv[ARGV_SIZE];
#if STRING_MODE == USE_C_STRING
  CommandDispatcherC dispatcher(streamReader, Serial,
      dispatchTable, numCommands, argv, ARGV_SIZE);
#else
  CommandDispatcherF dispatcher(streamReader, Serial,
      dispatchTable, numCommands, argv, ARGV_SIZE);
#endif

//---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  while (!Serial); // micro/leonardo

  dispatcher.resume(); // insert into the scheduler
  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
