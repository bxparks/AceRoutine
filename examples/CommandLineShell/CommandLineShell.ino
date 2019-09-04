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

#if defined(UNIX_HOST_DUINO)
  #include <unistd.h>
#endif

using namespace ace_routine;
using namespace ace_routine::cli;

// Every board except ESP32 defines SERIAL_PORT_MONITOR..
#if defined(ESP32)
  #define SERIAL_PORT_MONITOR Serial
#endif

//---------------------------------------------------------------------------

#ifdef LED_BUILTIN
  const int LED = LED_BUILTIN;
#else
  // Some ESP32 boards do not define LED_BUILTIN. Sometimes they have more than
  // 1. Replace this with the proper pin number.
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
 * Return the amount of free memory. See various tutorials and references:
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
 *
 * For Unix:
 * - https://stackoverflow.com/questions/2513505
 */
unsigned long freeMemory() {
#if defined(ARDUINO_ARCH_AVR)
  char top;
  return &top - (__brkval ? __brkval : __malloc_heap_start);
#elif defined(ARDUINO_ARCH_SAMD)
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(TEENSYDUINO)
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(ESP8266)
  return system_get_free_heap_size();
#elif defined(ESP32)
  return ESP.getFreeHeap();
#elif defined(UNIX_HOST_DUINO)
  long pages = sysconf(_SC_PHYS_PAGES);
  long page_size = sysconf(_SC_PAGE_SIZE);
  return pages * page_size;
#else
  #error Unsupported platform
#endif
}

//---------------------------------------------------------------------------

/** List the coroutines known by the CoroutineScheduler. */
class ListCommand: public CommandHandler {
  public:
    ListCommand():
      CommandHandler(F("list"), nullptr) {}

    virtual void run(Print& printer, int /* argc */,
        const char* const* /* argv */) const override {
      CoroutineScheduler::list(printer);
    }
};

/** Echo the command line arguments. */
class EchoCommand: public CommandHandler {
  public:
    EchoCommand():
      CommandHandler(F("echo"), F("args ...")) {}

    virtual void run(Print& printer, int argc, const char* const* argv)
        const override {
     for (int i = 1; i < argc; i++) {
        printer.print(argv[i]);
        printer.print(' ');
      }
      printer.println();
    }
};

/** Print amount of free memory between stack and heap. */
class FreeCommand: public CommandHandler {
  public:
    FreeCommand():
        CommandHandler(F("free"), nullptr) {}

    virtual void run(Print& printer, int /* argc */,
        const char* const* /* argv */) const override {
      printer.print(F("Free memory: "));
      printer.println(freeMemory());
    }
};

/**
 * Change the blinking LED on and off delay parameters. If no argument given,
 * simply print out the current values. Demonstrates the use of
 * SHIFT_ARGC_ARGV() macro and the isArgEqual() helper method.
 */
class DelayCommand: public CommandHandler {
  public:
    DelayCommand():
        CommandHandler(F("delay"), F("[(on | off) millis]")) {}

    virtual void run(Print& printer, int argc, const char* const* argv)
        const override {
      if (argc == 1) {
        printer.print(F("LED_ON delay: "));
        printer.println(ledOnDelay);
        printer.print(F("LED_OFF delay: "));
        printer.println(ledOffDelay);
        return;
      }

      if (argc < 3) {
        printer.println(F("Invalid number of arguments"));
        return;
      }

      SHIFT_ARGC_ARGV(argc, argv);
      if (isArgEqual(argv[0], F("on"))) {
        SHIFT_ARGC_ARGV(argc, argv);
        ledOnDelay = atoi(argv[0]);
      } else if (isArgEqual(argv[0], F("off"))) {
        SHIFT_ARGC_ARGV(argc, argv);
        ledOffDelay = atoi(argv[0]);
      } else {
        printer.print(F("Unknown argument: "));
        printer.println(argv[0]);
      }
    }
};

DelayCommand delayCommand;
ListCommand listCommand;
FreeCommand freeCommand;
EchoCommand echoCommand;
static const CommandHandler* const COMMANDS[] = {
  &delayCommand,
  &listCommand,
  &freeCommand,
  &echoCommand
};
static const uint8_t NUM_COMMANDS = sizeof(COMMANDS) / sizeof(CommandHandler*);

static const uint8_t BUF_SIZE = 64;
static const uint8_t ARGV_SIZE = 5;
static const char PROMPT[] = "$ ";

CommandManager<BUF_SIZE, ARGV_SIZE> commandManager(
    COMMANDS, NUM_COMMANDS, SERIAL_PORT_MONITOR, PROMPT);

//---------------------------------------------------------------------------

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000);
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // micro/leonardo
  pinMode(LED, OUTPUT);

  commandManager.setupCoroutine(F("commandManager"));
  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
