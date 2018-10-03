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

#ifndef ACE_ROUTINE_COMMAND_DISPATCHER_H
#define ACE_ROUTINE_COMMAND_DISPATCHER_H

#include <Print.h> // Print
#include <AceRoutine.h>
#include "CommandHandler.h"
#include "InputLine.h"

class __FlashStringHelper;

namespace ace_routine {
namespace cli {

/**
 * A coroutine that reads lines from the Serial port, tokenizes the line on
 * whitespace boundaries, and calls the appropriate command handler to handle
 * the command. Command have the form "command arg1 arg2 ...", where the 'arg*'
 * can be any string.
 *
 * The calling code is expected to provide a mapping of the command string
 * (e.g. "list") to its command handler (CommandHandler). The CommandHandler is
 * called with the number of arguments (argc) and the array of tokens (argv),
 * just like the arguments of the C-language main() function.
 *
 * The template parameters can be either a 'char' for C-strings or
 * '__FlashStringHelper' for F() flash strings.
 */
class CommandDispatcher: public Coroutine {
  public:
    /**
     * Constructor.
     *
     * @param channel A channel from the StringLineReader to this coroutine.
     * @param printer The output object, normally the global Serial object.
     * @param commands Array of CommandHandler pointers.
     * @param numCommands number of commands.
     * @param argv Array of (const char*) that will be used to hold the word
     *        tokens of a command line string.
     * @param argvSize The size of the argv array. Tokens which are beyond this
     *        limit will be silently dropped.
     * @param prompt If not null, print a prompt and echo the command entered
     *        by the user. If null, don't print prompt and don't echo.
     */
    CommandDispatcher(
            Channel<InputLine>& channel,
            Print& printer,
            const CommandHandler* const* commands,
            uint8_t numCommands,
            const char** argv,
            uint8_t argvSize,
            const char* prompt):
        mChannel(channel),
        mPrinter(printer),
        mCommands(commands),
        mNumCommands(numCommands),
        mArgv(argv),
        mArgvSize(argvSize),
        mPrompt(prompt) {}

    /** Destructor. Used only in unit tests. */
    virtual ~CommandDispatcher() {}

    /**
     * Tokenize the line, separating tokens delimited by whitespace (space,
     * formfeed, carriage return, newline, tab, and vertical tab) and fill argv
     * with each token until argvSize is reached. Return the number of tokens
     * filled in.
     *
     * VisibleForTesting.
     */
    static uint8_t tokenize(char* line, const char** argv, uint8_t argvSize) {
      char* token = strtok(line, DELIMS);
      int argc = 0;
      while (token != nullptr && argc < argvSize) {
        argv[argc] = token;
        argc++;
        token = strtok(nullptr, DELIMS);
      }
      return argc;
    }

    virtual int runCoroutine() override {
      InputLine input;
      COROUTINE_LOOP() {
        if (mPrompt != nullptr) {
          mPrinter.print(mPrompt);
        }
        COROUTINE_CHANNEL_READ(mChannel, input);

        if (input.status == InputLine::kStatusOverflow) {
          printLineError(input.line, STATUS_BUFFER_OVERFLOW);
          continue;
        }

        if (mPrompt != nullptr) {
          mPrinter.print(input.line); // line includes the \n
        }
        runCommand(input.line);
      }
    }

    /**
     * Find the CommandHandler of the given command name.
     *
     * VisibleForTesting.
     *
     * NOTE: this is currently a linear O(N) scan which is good enough for
     * small number of commands. If we sorted the handlers, we could do a
     * binary search for O(log(N)) and handle larger number of commands.
     */
    const CommandHandler* findCommand(const char* cmd) const {
      for (uint8_t i = 0; i < mNumCommands; i++) {
        const CommandHandler* command = mCommands[i];
        if (command->getName().compareTo(FCString(cmd)) == 0) {
          return command;
        }
      }
      return nullptr;
    }

  protected:
    // Disable copy-constructor and assignment operator
    CommandDispatcher(const CommandDispatcher&) = delete;
    CommandDispatcher& operator=(const CommandDispatcher&) = delete;

    /** Print the input line that caused an error along with its status code. */
    void printLineError(const char* line, uint8_t statusCode) const;

    /** Handle the 'help' command. */
    void helpCommandHandler(Print& printer, int argc, const char** argv) const;

    /** Print help on all commands */
    void helpAll(Print& printer) const;

    /** Print helpString of specific cmd. Returns true if cmd was found. */
    bool helpSpecific(Print& printer, const char* cmd) const;

    /** Print help string for the given command. */
    static void printHelp(Print& printer, const CommandHandler* command);

    /** Tokenize the given line and run the command handler. */
    void runCommand(char* line) const;

    /** Find and run the given command. */
    void findAndRunCommand(const char* cmd, int argc, const char** argv) const;

    static const uint8_t STATUS_SUCCESS = 0;
    static const uint8_t STATUS_BUFFER_OVERFLOW = 1;
    static const uint8_t STATUS_FLUSH_TO_EOL = 2;
    static const char DELIMS[];

    Channel<InputLine>& mChannel;
    Print& mPrinter;
    const CommandHandler* const* const mCommands;
    uint8_t const mNumCommands;
    const char** const mArgv;
    uint8_t const mArgvSize;
    const char* const mPrompt;
};

}
}

#endif
