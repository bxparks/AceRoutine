#line 2 "CliTest.ino"

#include <stdint.h> // uintptr_t
#include <Arduino.h> // Print
#include <AceRoutine.h>
#include <AUnitVerbose.h>
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

void dummyCommand(Print& /* printer */, int /* argc */,
    const char** /* argv */) {}

static const DispatchRecord dispatchTable[] = {
  {dummyCommand, "echo", "args ..."},
  {dummyCommand, "ls", "[flags] args ..."},
};
const uint8_t NUM_COMMANDS = sizeof(dispatchTable) / sizeof(DispatchRecord);

test(findHandler) {
  const DispatchRecord* record = CommandDispatcher::findCommand(
      dispatchTable, NUM_COMMANDS, "echo");
  assertEqual((uintptr_t) record->command, (uintptr_t) dummyCommand);
  assertEqual(record->name, "echo");
  assertEqual(record->helpString, "args ...");

  record = CommandDispatcher::findCommand(
      dispatchTable, NUM_COMMANDS, "NOTFOUND");
  assertEqual((uintptr_t) record, (uintptr_t) nullptr);
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
