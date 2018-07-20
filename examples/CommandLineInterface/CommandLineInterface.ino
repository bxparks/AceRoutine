/*
 * A sketch that reads the serial port for a command, parses the command line,
 * then calls the appropriate command handler.
 *
 * Run the sketch, then type 'help' on the serial port.
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
class ListCommandHandler: public CommandHandler {
  public:
    ListCommandHandler(): CommandHandler("list", nullptr) {}

    virtual void run(int /* argc */, const char** /* argv */) const override {
      CoroutineScheduler::list(&Serial);
    }
};

/** Echo the command line arguments. */
class EchoCommandHandler: public CommandHandler {
  public:
    EchoCommandHandler(): CommandHandler("echo", "args ...") {}

    virtual void run(int argc, const char** argv) const override {
     for (int i = 1; i < argc; i++) {
        Serial.print(argv[i]);
        Serial.print(' ');
      }
      Serial.println();
    }
};

/** Print amount of free memory between stack and heap. */
class FreeCommandHandler: public CommandHandler {
  public:
    FreeCommandHandler(): CommandHandler("free", nullptr) {}

    virtual void run(int /* argc */, const char** /* argv */) const override {
      Serial.print(F("Free memory: "));
      Serial.println(freeMemory());
    }
};

/** Change the blinking LED on and off delay parameters. */
class DelayCommandHandler: public CommandHandler {
  public:
    DelayCommandHandler(): CommandHandler("delay", "(on | off) millis") {}

    virtual void run(int argc, const char** argv) const override {
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
};

DelayCommandHandler delayCommandHandler;
ListCommandHandler listCommandHandler;
FreeCommandHandler freeCommandHandler;
EchoCommandHandler echoCommandHandler;

static const CommandHandler* commands[] = {
  &delayCommandHandler,
  &listCommandHandler,
  &freeCommandHandler,
  &echoCommandHandler,
};

static const int BUF_SIZE = 64;
char lineBuffer[BUF_SIZE];
SerialReader serialReader(Serial, lineBuffer, BUF_SIZE);

static const int8_t ARGV_SIZE = 10;
const char* argv[ARGV_SIZE];
CommandDispatcher dispatcher(
    serialReader,
    commands,
    sizeof(commands) / sizeof(CommandHandler*),
    argv,
    ARGV_SIZE);

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
