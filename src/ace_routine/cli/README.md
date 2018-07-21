# Command Line Interface

These classes implement a non-blocking command line interface on the Serial
port. In ther words, you can implement a primitive "shell" for the Arduino.

* `StreamReader` - a class that provided non-blocking methods for
  reading various types of input tokens from the `Serial` port. Supported tokens
  include whole lines (terminated by newline), words (separated by whitespace),
  and integers.
* `CommandDispatcher` - a class that uses the `StreamReader` and
  the `AceRoutine` library to read command lines from the `Serial` port, parse
  the command line string, then dispatch to the appropriate `CommandHandler` to
  perform the associated command.

These classes were initially an experiment to validate the `AceRoutine` macros
and classes but they seem to be useful as an independent library. They may be
moved to a separate project/repository later.

Version: (2018-07-20)

## Usage

See [examples/CommandLineInterface/](../../../examples/CommandLineInterface/)
for an demo program that implements 5 commands:

* `help [command]`
* `list`
* `free`
* `echo [args ...]`
* `delay (on | off) millis`

The basic process is:

1. Create a `StreamReader`, giving it the buffers that it needs.
1. Create a `DispatchTable` containing the list of commands whose signature
   matches `CommandHandler`.
1. Create a `CommanDispatcher` which reads a whole line from the Serial port,
   and dispatches to the appropriate command.
1. Run the `CommandDispatcher` as an `AceRoutine` coroutine in the global
   `loop()`.

The main `*.ino` file will look like this:

```
#include <AceRoutine.h>
#include <ace_routine/cli/StreamReader.h>
#include <ace_routine/cli/CommandDispatcher.h>

using namespace ace_routine;
using namespace ace_routine::cli;

// Create the StreamReader with buffers.
const int BUF_SIZE = 64;
char lineBuffer[BUF_SIZE];
StreamReader streamReader(Serial, lineBuffer, BUF_SIZE);

// Define the command handlers.
void newCommand(Print& printer, int argc, const char** argv) {
  ...
}

void anotherCommand(Print& printer, int argc, const char** argv) {
  ...
}

// Create the dispatch table of commands.
const DispatchRecord dispatchTable[] = {
  {&newCommand, "name-of-command", "help-string"},
  {&anotherCommand, "name-of-other-command", "help-string"},
  ...
};
const uint8_t NUM_COMMANDS = sizeof(dispatchTable) / sizeof(DispatchRecord);

// Create CommandDispatcher with buffers.
const int8_t ARGV_SIZE = 10;
const char* argv[ARGV_SIZE];
CommandDispatcher dispatcher(streamReader, Serial,
    dispatchTable, NUM_COMMANDS argv, ARGV_SIZE);

void setup() {
  Serial.begin(115200);
  while (!Serial); // micro/leonardo

  dispatcher.resume(); // insert into the scheduler
  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
```
