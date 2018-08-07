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

The basic steps for adding a command line interface to an Arduino sketch
using the `cli/` library is the following:

1. Create a `StreamReader`, giving it the buffers that it needs.
1. Create a `DispatchTable` containing the list of commands whose signature
   matches `CommandHandler`.
1. Create a `CommanDispatcher` which reads a whole line from the Serial port,
   and dispatches to the appropriate command.
1. Run the `CommandDispatcher` as an `AceRoutine` coroutine in the global
   `loop()`.

### Command Dispatcher

The `CommandDispatcher` is a subclass of the `Coroutine` class and implements a
coroutine in the `run()` method. This is a manually created coroutine,
not managed by the `COROUTINE()` macro, so the
the `resume()` method must be called to add it to the `CoroutineScheduler`.

The calling client is expected to create one of 2 subclasses of
`CommandDispatcher`:
* `CommandDispatcherC`: accepts an array of `DispatchRecordC` objects which
  hold normal C-strings (i.e. `const char*`)
* `CommandDispatcherF`: accepts an array of `DispatchRecordF` objects which
  hold
  [PROGMEM](https://arduino.cc/reference/en/language/variables/utilities/progmem/)
  strings stored in flash memory (i.e. `const __FlashStringHelper*`)

On devices with limited static RAM like AVR boards, using flash strings can be
critical to allowing the program to run.

### Command Handler and Arguments

The `CommandHandler` typedef is a pointer to a user-defined function that has
the following signature:
```
typedef void (*CommandHandler)(Print& printer, int argc, const char** argv);
```

* `printer` is the output device, which will normally be the global `Serial`
  object
* `argc` is the number of `argv` arguments
* `argv` is the array of `cont char*` pointers, each pointing to the words
  of the command line delimited by whitespaces. These are identical to
  the `argc` and `argv` parameters passed to the C-language `main(argc, argv)`
  function. For example, `argv[0]` is the name of the command, and `argv[1]`
  is the first argument after the command (if it exists).

### Memory Allocation

Both the `StreamReader` and `CommandDispatcher` perform no memory allocations
internally (no `malloc()`, no `new` operator) to avoid memory problems. All
data structures used by these classes must be pre-allocated by the calling code.
Normally they will be created at static initialization time (before the global
`setup()` method is called), but the calling code is allowed to create them on
the heap if it wants or needs to.

### Structure of Client Calling Code

An Arduino `.ino` file that uses the CLI classes to implement a commmand line
shell will look something like this:

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

// Create the dispatch table of commands (using C-strings)
const DispatchRecordC dispatchTable[] = {
  {&newCommand, "name-of-command", "help-string"},
  {&anotherCommand, "name-of-other-command", "help-string"},
  ...
};
const uint8_t NUM_COMMANDS = sizeof(dispatchTable) / sizeof(DispatchRecordC);

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

## Example

See [examples/CommandLineInterface/](../../../examples/CommandLineInterface/)
for an demo program that implements 5 commands:

* `help [command]`
* `list`
* `free`
* `echo [args ...]`
* `delay (on | off) millis`
