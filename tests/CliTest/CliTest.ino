#line 2 "CliTest.ino"

#include <stdint.h> // uintptr_t
#include <Arduino.h> // Print
#include <AceRoutine.h>
#include <AUnitVerbose.h>
#include "ace_routine/cli/CommandManager.h"

using namespace ace_routine;
using namespace ace_routine::cli;
using namespace aunit;

// ---------------------------------------------------------------------------

const uint8_t BUF_SIZE = 64;
const uint8_t ARGV_SIZE = 4;
const char PROMPT[] = "> ";
const char LIST_COMMAND_NAME_F[] PROGMEM = "list";
const char LIST_COMMAND_HELP_STRING_F[] PROGMEM = "files ...";

class DummyCommand: public CommandHandler {
  public:
    DummyCommand(const char* name, const char* helpString):
      CommandHandler(name, helpString) {}

    DummyCommand(const __FlashStringHelper* name,
        const __FlashStringHelper* helpString):
      CommandHandler(name, helpString) {}

    virtual void run(Print& /* printer */, int /* argc */,
        const char** /* argv */) const override {}
};

static DummyCommand echoCommand("echo", "args ...");
static DummyCommand lsCommand("ls", "[flags] args ...");
static DummyCommand listCommand(
    (const __FlashStringHelper*) LIST_COMMAND_NAME_F,
    (const __FlashStringHelper*) LIST_COMMAND_HELP_STRING_F);
static const CommandHandler* const COMMANDS[] = {
  &echoCommand,
  &lsCommand,
  &listCommand,
};
static uint8_t const NUM_COMMANDS = sizeof(COMMANDS) / sizeof(CommandHandler*);

static CommandManager<BUF_SIZE, ARGV_SIZE> commandManager(
    COMMANDS, NUM_COMMANDS, Serial, PROMPT);

class CommandDispatcherTest: public TestOnce {
  protected:
    void assertArgvEquals(const char** argv, const char** expected,
        uint8_t size) {
      for (uint8_t i = 0; i < size; i++) {
        assertEqual(argv[i], expected[i]);
      }
    }
};

// ---------------------------------------------------------------------------

testF(CommandDispatcherTest, tokenize) {
  const char* expected[] = { "a", "b", "c", "d" };
  const char* argv[ARGV_SIZE];

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

test(findCommand) {
  const CommandDispatcher* dispatcher = commandManager.getDispatcher();

  // "echo" command uses normal C strings
  const CommandHandler* echoCommandResult = dispatcher->findCommand("echo");
  assertEqual((uintptr_t) echoCommandResult, (uintptr_t) &echoCommand);
  assertEqual(echoCommandResult->getName().compareTo(FCString("echo")), 0);
  assertEqual(echoCommandResult->getHelpString().compareTo(
      FCString("args ...")), 0);

  // "list" command uses flash strings
  const CommandHandler* listCommandResult = dispatcher->findCommand("list");
  assertEqual((uintptr_t) listCommandResult, (uintptr_t) &listCommand);
  assertEqual(listCommandResult->getName().compareTo(FCString("list")), 0);
  assertEqual(listCommandResult->getHelpString().compareTo(
      FCString("files ...")), 0);

  // this should not be found
  const CommandHandler* notFound = dispatcher->findCommand("NOTFOUND");
  assertEqual((uintptr_t) notFound, (uintptr_t) nullptr);
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
