/*
MIT License

Copyright (c) 2018 Brian T. Park

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "CommandDispatcher.h"

namespace ace_routine {
namespace cli {

const char CommandDispatcher::DELIMS[] = " \t\n";

void CommandDispatcher::printLineError(const char* line, uint8_t statusCode) {
  if (statusCode == STATUS_BUFFER_OVERFLOW) {
    mPrinter.print(F("BufferOverflow: "));
    mPrinter.println(line);
  } else if (statusCode == STATUS_FLUSH_TO_EOL) {
    mPrinter.print(F("FlushToEOL: "));
    mPrinter.println(line);
  } else {
    mPrinter.print(F("UnknownError: "));
    mPrinter.print(statusCode);
    mPrinter.print(F(": "));
    mPrinter.println(line);
  }
}

/** Handle the 'help' command. */
void CommandDispatcher::helpCommandHandler(
    Print& printer, int argc, const char** argv) {
  if (argc == 2) {
    const char* cmd = argv[1];
    if (strcmp(cmd, "help") == 0) {
      printer.println(F("Usage: help [command]"));
      return;
    }

    for (uint8_t i = 0; i < mNumCommands; i++) {
      const DispatchRecord* record = &mDispatchTable[i];
      if (strcmp(record->name, cmd) == 0) {
        printer.print(F("Usage: "));
        printer.print(cmd);
        printer.print(' ');
        printer.println(record->helpString);
        return;
      }
    }
    printer.print(F("Unknown command: "));
    printer.println(cmd);
  } else {
    printer.println(F("Usage: help [command]"));
    printer.print(F("Commands: help "));
    for (uint8_t i = 0; i < mNumCommands; i++) {
      const DispatchRecord* record = &mDispatchTable[i];
      printer.print(record->name);
      printer.print(' ');
    }
    printer.println();
  }
}

/** Tokenize the given line and run the command handler. */
void CommandDispatcher::runCommand(char* line) {
  // Tokenize the line.
  int argc = tokenize(line, mArgv, mArgvSize);
  if (argc == 0) return;
  const char* cmd = mArgv[0];

  // Handle the built-in 'help' command.
  if (strcmp(cmd, "help") == 0) {
    helpCommandHandler(mPrinter, argc, mArgv);
    return;
  }

  // Dispatch to the matching command handler if found.
  const DispatchRecord* record = findCommand(mDispatchTable, mNumCommands, cmd);
  if (record != nullptr) {
    record->command(mPrinter, argc, mArgv);
    return;
  }

  mPrinter.print(F("Unknown command: "));
  mPrinter.println(cmd);
}

const DispatchRecord* CommandDispatcher::findCommand(
    const DispatchRecord* dispatchTable,
    uint8_t numCommands, const char* cmd) {
  for (uint8_t i = 0; i < numCommands; i++) {
    const DispatchRecord* record = &dispatchTable[i];
    if (strcmp(record->name, cmd) == 0) {
      return record;
    }
  }
  return nullptr;
}

uint8_t CommandDispatcher::tokenize(char* line, const char** argv,
    uint8_t argvSize) {
  char* token = strtok(line, DELIMS);
  int argc = 0;
  while (token != nullptr && argc < argvSize) {
    argv[argc] = token;
    argc++;
    token = strtok(nullptr, DELIMS);
  }
  return argc;
}

int CommandDispatcher::runRoutine() {
  bool isError;
  char* line;
  COROUTINE_LOOP() {
    COROUTINE_AWAIT(mSerialReader.getLine(&isError, &line));

    if (isError) {
      printLineError(line, STATUS_BUFFER_OVERFLOW);
      while (isError) {
        COROUTINE_AWAIT(mSerialReader.getLine(&isError, &line));
        printLineError(line, STATUS_FLUSH_TO_EOL);
      }
      continue;
    }

    runCommand(line);
  }
}

}
}
