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

const uint8_t MAX_COMMANDS = 4;
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

class CommandDispatcherTest: public TestOnce {
  protected:
    virtual void setup() override {
      mCommandManager = new CommandManager<ARGV_SIZE, BUF_SIZE, ARGV_SIZE>(
          Serial, PROMPT);
      mCommandManager->add(&mEchoCommand);
      mCommandManager->add(&mLsCommand);
      mCommandManager->add(&mListCommand);
      mCommandManager->setupCommands();
    }

    virtual void teardown() override {
      delete mCommandManager;
    }

    void assertArgvEquals(const char** argv, const char** expected,
        uint8_t size) {
      for (uint8_t i = 0; i < size; i++) {
        assertEqual(argv[i], expected[i]);
      }
    }

    CommandManager<ARGV_SIZE, BUF_SIZE, ARGV_SIZE>* mCommandManager;
    DummyCommand mEchoCommand = DummyCommand("echo", "args ...");
    DummyCommand mLsCommand = DummyCommand("ls", "[flags] args ...");
    DummyCommand mListCommand = DummyCommand(
        (const __FlashStringHelper*) LIST_COMMAND_NAME_F,
        (const __FlashStringHelper*) LIST_COMMAND_HELP_STRING_F);
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

testF(CommandDispatcherTest, findCommand) {
  const CommandDispatcher* dispatcher = mCommandManager->getDispatcher();

  // "echo" command uses normal C strings
  const CommandHandler* echoCommand = dispatcher->findCommand("echo");
  assertEqual((uintptr_t) echoCommand, (uintptr_t) &mEchoCommand);
  assertEqual(echoCommand->getName().compareTo(FCString("echo")), 0);
  assertEqual(echoCommand->getHelpString().compareTo(FCString("args ...")), 0);

  // "list" command uses flash strings
  const CommandHandler* listCommand = dispatcher->findCommand("list");
  assertEqual((uintptr_t) listCommand, (uintptr_t) &mListCommand);
  assertEqual(listCommand->getName().compareTo(FCString("list")), 0);
  assertEqual(listCommand->getHelpString().compareTo(FCString("files ...")), 0);

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
