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

/*
 * Most of these methods are in the .cpp file instead of remaining in the .h
 * file because they use the F() macro, which causes problems for ESP8266 if
 * they are used in an inline context.
 */

#include "CommandDispatcher.h"

namespace ace_routine {
namespace cli {

// Same whitespace characters used by isspace() in the standard C99 library.
const char CommandDispatcher::DELIMS[] = " \f\r\n\t\v";

void CommandDispatcher::printLineError(const char* line, uint8_t statusCode)
    const {
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

void CommandDispatcher::helpCommandHandler(
    Print& printer, int argc, const char** argv) const {
  if (argc == 2) {
    const char* cmd = argv[1];

    // check for "help help"
    if (strcmp(cmd, "help") == 0) {
      printer.println(F("Usage: help [command]"));
      return;
    }

    bool found = helpSpecific(printer, cmd);
    if (found) return;
    printer.print(F("Unknown command: "));
    printer.println(cmd);
  } else {
    printer.println(F("Commands:"));
    printer.println(F("  help [command]"));
    helpAll(printer);
  }
}

void CommandDispatcher::helpAll(Print& printer) const {
  for (uint8_t i = 0; i < mNumCommands; i++) {
    const CommandHandler* command = mCommands[i];
    printer.print("  ");
    printHelp(printer, command);
  }
}

bool CommandDispatcher::helpSpecific(Print& printer, const char* cmd) const {
  const CommandHandler* command = findCommand(cmd);
  if (command != nullptr) {
    printer.print(F("Usage: "));
    printHelp(printer, command);
    return true;
  }
  return false;
}

void CommandDispatcher::printHelp(
    Print& printer, const CommandHandler* command) {
  command->getName().printTo(printer);
  if (!command->getHelpString().isNull()) {
    printer.print(' ');
    command->getHelpString().printTo(printer);
    printer.println();
  } else {
    printer.println();
  }
}

void CommandDispatcher::runCommand(char* line) const {
  // Tokenize the line.
  int argc = tokenize(line, mArgv, mArgvSize);
  if (argc == 0) return;
  const char* cmd = mArgv[0];

  // Handle the built-in 'help' command.
  if (strcmp(cmd, "help") == 0) {
    helpCommandHandler(mPrinter, argc, mArgv);
    return;
  }

  findAndRunCommand(cmd, argc, mArgv);
}

void CommandDispatcher::findAndRunCommand(
    const char* cmd, int argc, const char** argv) const {
  const CommandHandler* command = findCommand(cmd);
  if (command != nullptr) {
    command->run(mPrinter, argc, argv);
    return;
  }

  mPrinter.print(F("Unknown command: "));
  mPrinter.println(cmd);
}

}
}
