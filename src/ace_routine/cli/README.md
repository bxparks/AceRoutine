# Command Line Interface

These classes which implement a non-blocking command line interface on the
Serial port.

* `SerialReader` - a class that provided non-blocking methods for
  reading various input tokens from the `Serial` port. Supported tokens include
  lines (terminated by newline), words (separated by whitespace), and
  integers.
* `CommandDispatcher` - a class that uses the `SerialReader` and
  the `AceRoutine` library to read command lines from the `Serial` port,
  parse the command line, then dispatch to the appropriate `CommandHandler`
  object to perform the associated command.

These classes were initially created to experiment on and validate the
`AceRoutine` API but they seem to be useful as an independent library. They may
be moved to a separate project/repository later.

Version: (2018-07-20)

## Usage

See 
[examples/CommandLineInterface/CommandLineInterface.ino](../../../examples/CommandLineInterface/CommandLineInterface.ino)
for an example program that implements 5 commands:
* `help [command]`
* `list`
* `free`
* `echo [args ...]`
* `delay (on | off) millis`

```
#include <AceRoutine.h>
#include <ace_routine/cli/SerialReader.h>
#include <ace_routine/cli/CommandDispatcher.h>

using namespace ace_routine;
using namespace ace_routine::cli;

// Create the SerialReader with buffers.
const int BUF_SIZE = 64;
char lineBuffer[BUF_SIZE];
SerialReader serialReader(Serial, lineBuffer, BUF_SIZE);

// Define the command handlers.
class NewCommandHandler: public CommandHandler {
  public:
    NewCommandHandler(const char* name, const char* helpString):
        CommandHandler(name, helpString) {}
    
    virtual void run(int argc, const char** argv) const override {
      // insert body of command
      ...
    }
};

// Create the command handler instance, giving its name and helpString.
NewCommandHandler newCommandHandler("name-of-command", "help-string");

// Create a table of commands.
const CommandHandler* commands[] = {
  &newCommandHandler,
  ...
};

const uint8_t NUM_COMMANDS = sizeof(commands) / sizeof(CommandHandler*);

// Create CommandDispatcher with buffers.
const int8_t ARGV_SIZE = 10;
const char* argv[ARGV_SIZE];
CommandDispatcher dispatcher(
    serialReader, commands, NUM_COMMANDS argv, ARGV_SIZE);


void setup() {
  Serial.begin(115200);
  if (!Serial) {} // micro/leonardo

  dispatcher.resume(); // insert into the scheduler
  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
```
