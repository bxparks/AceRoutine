#line 2 "CliTest.ino"

#include <stdint.h> // uintptr_t
#include <Arduino.h>
#include <AceRoutine.h>
#include <AUnitVerbose.h>
#include "ace_routine/cli/SerialReader.h"
#include "ace_routine/cli/CommandDispatcher.h"

using namespace ace_routine;
using namespace ace_routine::cli;
using namespace aunit;

// ---------------------------------------------------------------------------

const int ARGV_SIZE = 4;
const char* argv[ARGV_SIZE];

class CommandDispatcherTest: public TestOnce {
  protected:
    void assertArgvEquals(const char** argv, const char** expected,
        uint8_t size) {
      for (uint8_t i = 0; i < size; i++) {
        assertEqual(argv[i], expected[i]);
      }
    }
};

testF(CommandDispatcherTest, tokenize) {
  const char* expected[] = { "a", "b", "c", "d" };

  char BLANK[] = "\n";
  uint8_t count = CommandDispatcher::tokenize(BLANK, argv, ARGV_SIZE);
  assertEqual(count, 0);

  // 4 tokens
  char LINE[] = "a b c d";
  count = CommandDispatcher::tokenize(LINE, argv, ARGV_SIZE);
  assertEqual(count, 4);
  assertArgvEquals(argv, expected, count);

  // 5 tokens get truncated to 4
  char LINE5[] = "a b c d e";
  count = CommandDispatcher::tokenize(LINE5, argv, ARGV_SIZE);
  assertEqual(count, 4);
  assertArgvEquals(argv, expected, count);
}

// ---------------------------------------------------------------------------

class DummyHandler: public CommandHandler {
  public:
    DummyHandler(const char* name, const char* helpString):
        CommandHandler(name, helpString) {}

    virtual void run(int /* argc */, const char** /* argv */) const override {}
};

DummyHandler handler1("echo", "args ...");
DummyHandler handler2("ls", "[flags] args ...");

static const CommandHandler* commands[] = {
  &handler1,
  &handler2,
};
const uint8_t NUM_COMMANDS = sizeof(commands) / sizeof(CommandHandler*);

test(findHandler) {
  const CommandHandler* handler = CommandDispatcher::findHandler(
      commands, NUM_COMMANDS, "echo");
  assertEqual((uintptr_t) &handler1, (uintptr_t) handler);
  assertEqual(handler->getName(), "echo");
  assertEqual(handler->getHelpString(), "args ...");

  handler = CommandDispatcher::findHandler(
      commands, NUM_COMMANDS, "NOTFOUND");
  assertEqual((uintptr_t) handler, (uintptr_t) nullptr);
}

// ---------------------------------------------------------------------------

void setup() {
  delay(1000); // some boards reboot twice
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro
}

void loop() {
  TestRunner::run();
}
