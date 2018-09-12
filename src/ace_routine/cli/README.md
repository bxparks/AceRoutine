# Command Line Interface

These classes implement a non-blocking command line interface on the Serial
port. In ther words, you can implement a primitive "shell" for the Arduino.

These classes were initially an experiment to validate the `AceRoutine` macros
and classes but they seem to be useful as an independent library. They may be
moved to a separate project/repository later.

Version: (2018-07-20)

## Usage

The basic steps for adding a command line interface to an Arduino sketch
using the `cli/` library is the following:

1. Create a `CommandHandler` function for each command.
1. Create a `CommandManager` object that defines size parameters of various
   internal buffers.
1. Register the command in the CommandManager in the global `setup()` method
   using `CommandManager::add(handler, name, helpString)` for each command.
1. Insert the `CommandManager` into the `CoroutineScheduler` by calling
   `commandManager.resume();` just before `CoroutineScheduler::setup()`.
1. Run the `CoroutineScheduler::loop` in the global `loop()` method to
   run the `CommandManager` as a coroutine.

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

### CommandManager

The `CommandManager` is a templatized convenience class that wraps the
`CommandDispatcher` along with all of the various buffers and resources that the
dispatcher needs. It greatly simplies the creation of the `CommandDispatcher`.

Using the template parameters, the `CommandManager` creates almost all of its
resources statically (i.e. the sizes are known at compile-time). However, it
creates a fixed array of command records in the `DispatchTable` on the heap. No
further heap allocation is performed for the lifetime of this object.

### Structure of Client Calling Code

An Arduino `.ino` file that uses the CLI classes to implement a commmand line
shell will look something like this:

```
#include <AceRoutine.h>
#include <ace_routine/cli/CommandManager.h>

using namespace ace_routine;
using namespace ace_routine::cli;

// Define the command handlers.
void newCommand(Print& printer, int argc, const char** argv) {
  ...
}

void anotherCommand(Print& printer, int argc, const char** argv) {
  ...
}

// Create the dispatch table of commands (using C-strings)
const DispatchRecordC dispatchTable[] = {

// Create the CommandManager
const uint8_t BUF_SIZE = 64; // maximum size of an input line
const uint8_t ARGV_SIZE = 10; // maximum number of tokens in command
const uint8_t TABLE_SIZE = 4; // maximum number of commands
const char PROMPT[] = "> ";
CommandManager<__FlashStringHelper, BUF_SIZE, ARGV_SIZE>
    commandManager(Serial, TABLE_SIZE, PROMPT);

void setup() {
  Serial.begin(115200);
  while (!Serial); // micro/leonardo

  commandManager.add(newCommand, "name-of-command", "help-string");
  commandManager.add(anotherCommand, "name-of-other-command", "help-string");
  ...
  commandManager.resume(); // insert into the scheduler
  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
```

## Example

See [examples/CommandLineShell/](../../../examples/CommandLineShell/)
for an demo program that implements 5 commands:

* `help [command]`
* `list`
* `free`
* `echo [args ...]`
* `delay (on | off) millis`
