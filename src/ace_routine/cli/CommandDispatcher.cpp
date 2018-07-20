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
    Serial.print(F("BufferOverflow: "));
    Serial.println(line);
  } else if (statusCode == STATUS_FLUSH_TO_EOL) {
    Serial.print(F("FlushToEOL: "));
    Serial.println(line);
  } else {
    Serial.print(F("UnknownError: "));
    Serial.print(statusCode);
    Serial.print(": ");
    Serial.println(line);
  }
}

/** Handle the 'help' command. */
void CommandDispatcher::helpCommandHandler(int argc, const char** argv) {
  if (argc == 2) {
    const char* cmd = argv[1];
    if (strcmp(cmd, "help") == 0) {
      Serial.println(F("Usage: help [command]"));
      return;
    }

    for (uint8_t i = 0; i < mNumCommands; i++) {
      const CommandHandler* handler = mComandHandlers[i];
      if (strcmp(handler->getName(), cmd) == 0) {
        Serial.print("Usage: ");
        Serial.print(cmd);
        Serial.print(' ');
        Serial.println(handler->getHelpString());
        return;
      }
    }
    Serial.print(F("Unknown command: "));
    Serial.println(cmd);
  } else {
    Serial.println(F("Usage: help [command]"));
    Serial.print(F("Commands: help "));
    for (uint8_t i = 0; i < mNumCommands; i++) {
      const CommandHandler* handler = mComandHandlers[i];
      Serial.print(handler->getName());
      Serial.print(' ');
    }
    Serial.println();
  }
}

/** Tokenize the given line and run the command handler. */
void CommandDispatcher::runCommand(char* line) {
  // Tokenize the line.
  const char* argv[ARGV_SIZE];
  int argc = tokenize(line, ARGV_SIZE, argv);
  if (argc == 0) return;
  const char* cmd = argv[0];

  // The 'help' command is built-in.
  if (strcmp(cmd, "help") == 0) {
    helpCommandHandler(argc, argv);
    return;
  }

  // Look for an entry in the handlers.
  // NOTE: this is currently a linear scan O(N) which is good enough for
  // small number of commands. If we sorted the handlers, we could do
  // a binary search for O(log(N)) and handle large number of commands.
  for (uint8_t i = 0; i < mNumCommands; i++) {
    const CommandHandler* handler = mComandHandlers[i];
    if (strcmp(handler->getName(), cmd) == 0) {
      handler->run(argc, argv);
      return;
    }
  }

  Serial.print(F("Unknown command: "));
  Serial.println(cmd);
}

/** Tokenize the line, returning the number of tokens. */
int CommandDispatcher::tokenize(char* line, int argvSize, const char** argv) {
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
