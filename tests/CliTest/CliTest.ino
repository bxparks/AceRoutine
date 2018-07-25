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

static const DispatchRecordC DISPATCH_TABLE[] = {
  {dummyCommand, "echo", "args ..."},
  {dummyCommand, "ls", "[flags] args ..."},
};
const uint8_t NUM_COMMANDS = sizeof(DISPATCH_TABLE) / sizeof(DispatchRecordC);

test(CommandDispatcherC_findCommand) {
  const DispatchRecordC* record = CommandDispatcherC::findCommand(
      DISPATCH_TABLE, NUM_COMMANDS, "echo");
  assertEqual((uintptr_t) record->command, (uintptr_t) dummyCommand);
  assertEqual((uintptr_t) record->name, (uintptr_t) "echo");
  assertEqual((uintptr_t) record->helpString, (uintptr_t) "args ...");

  record = CommandDispatcherC::findCommand(
      DISPATCH_TABLE, NUM_COMMANDS, "NOTFOUND");
  assertEqual((uintptr_t) record, (uintptr_t) nullptr);
}

const char ECHO_COMMAND[] PROGMEM = "echo";
const char ECHO_HELP_STRING[] PROGMEM = "args ...";
const char LS_COMMAND[] PROGMEM = "ls";
const char LS_HELP_STRING[] PROGMEM = "[flags] args ...";

static const DispatchRecordF DISPATCH_TABLE_F[] = {
  {dummyCommand,
      ACE_ROUTINE_FPSTR(ECHO_COMMAND),
      ACE_ROUTINE_FPSTR(ECHO_HELP_STRING)},
  {dummyCommand,
      ACE_ROUTINE_FPSTR(LS_COMMAND),
      ACE_ROUTINE_FPSTR(LS_HELP_STRING)},
};
const uint8_t NUM_COMMANDS_F =
    sizeof(DISPATCH_TABLE_F) / sizeof(DispatchRecordF);

test(CommandDispatcherF_findCommand) {
  const DispatchRecordF* record = CommandDispatcherF::findCommand(
      DISPATCH_TABLE_F, NUM_COMMANDS, "echo");
  assertEqual((uintptr_t) record->command, (uintptr_t) dummyCommand);
  assertEqual((uintptr_t) record->name, (uintptr_t) ECHO_COMMAND);
  assertEqual((uintptr_t) record->helpString, (uintptr_t) ECHO_HELP_STRING);

  record = CommandDispatcherF::findCommand(
      DISPATCH_TABLE_F, NUM_COMMANDS, "NOTFOUND");
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
