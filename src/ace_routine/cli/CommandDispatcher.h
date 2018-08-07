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

#include <Arduino.h> // Print
#include <AceRoutine.h>
#include "StreamReader.h"

namespace ace_routine {
namespace cli {

/** Signature for a command handler. */
typedef void (*CommandHandler)(Print& printer, int argc, const char** argv);

/**
 * A record of the command name and its handler. The helpString is the
 * "usage" string, excluding the name of the command itself to save space.
 * The name of the command will be automatically added by the 'help
 * command' handler.
 */
struct DispatchRecordC {
  const CommandHandler command;
  const char* name;
  const char* helpString;
};

/**
 * Same as DispatchRecordC but uses FlashStrings instead of (const char*) to
 * save static RAM on AVR boards.
 */
struct DispatchRecordF {
  const CommandHandler command;
  const __FlashStringHelper* name;
  const __FlashStringHelper* helpString;
};

/**
 * Base-class of a coroutine that reads lines from the Serial port, tokenizes
 * the line on whitespace boundaries, and calls the appropriate command handler
 * to handle the command. Command have the form "command arg1 arg2 ...", where
 * the 'arg*' can be any string.
 *
 * The calling code is expected to provide a mapping of the command string
 * (e.g. "list") to its command handler (CommandHandler). The CommandHandler is
 * called with the number of arguments (argc) and the array of tokens (argv),
 * just like the arguments of the C-language main() function.
 *
 * Use the CommandDispatcherC subclass if the DispatchRecordC class with
 * C-strings is used.
 *
 * Use the CommandDispatcherF subclass if the DispatchRecordF class with
 * FlashStrings are used.
 */
class CommandDispatcher: public Coroutine {
  public:
    /**
     * Constructor.
     *
     * @param streamReader An instance of StreamReader.
     * @param printer The output object, normally the global Serial object.
     * @param numCommands Number of entries in the dispatchTable.
     * @param argv Array of (const char*) that will be used to hold the word
     * tokens of a command line string.
     * @param argvSize The size of the argv array. Tokens which are beyond this
     * limit will be silently dropped.
     */
    CommandDispatcher(
            StreamReader& streamReader,
            Print& printer,
            uint8_t numCommands,
            const char** argv,
            uint8_t argvSize):
        mStreamReader(streamReader),
        mPrinter(printer),
        mNumCommands(numCommands),
        mArgv(argv),
        mArgvSize(argvSize) {}

    /**
     * Tokenize the line, and fill argv with each token until argvSize is
     * reached. Return the number of tokens. VisibleForTesting.
     */
    static uint8_t tokenize(char* line, const char** argv, uint8_t argvSize);

  protected:
    // Disable copy-constructor and assignment operator
    CommandDispatcher(const CommandDispatcher&) = delete;
    CommandDispatcher& operator=(const CommandDispatcher&) = delete;

    static const uint8_t STATUS_SUCCESS = 0;
    static const uint8_t STATUS_BUFFER_OVERFLOW = 1;
    static const uint8_t STATUS_FLUSH_TO_EOL = 2;
    static const char DELIMS[];

    /** Print the error caused by the given line. */
    void printLineError(const char* line, uint8_t statusCode);

    /** Handle the 'help' command. */
    void helpCommandHandler(Print& printer, int argc, const char** argv);

    /** Print generic help. */
    virtual void helpGeneric(Print& printer) = 0;

    /** Print helpString of specific cmd. Returns true if cmd was found. */
    virtual bool helpSpecific(Print& printer, const char* cmd) = 0;

    /** Tokenize the given line and run the command handler. */
    void runCommand(char* line);

    /** Find and run the given command. */
    virtual void findAndRunCommand(
        const char* cmd, int argc, const char** argv) = 0;

    virtual int run() override;

    StreamReader& mStreamReader;
    Print& mPrinter;
    const uint8_t mNumCommands;
    const char** const mArgv;
    const uint8_t mArgvSize;
};

/** A CommandDispatcher that takes DispatchRecordC records using C-strings. */
class CommandDispatcherC: public CommandDispatcher {
  public:
    /**
     * Constructor.
     *
     * @param streamReader An instance of StreamReader.
     * @param printer The output object, normally the global Serial object.
     * @param dispatchTable An array of DispatchRecords.
     * @param numCommands Number of entries in the dispatchTable.
     * @param argv Array of (const char*) that will be used to hold the word
     * tokens of a command line string.
     * @param argvSize The size of the argv array. Tokens which are beyond this
     * limit will be silently dropped.
     */
    CommandDispatcherC(
            StreamReader& streamReader,
            Print& printer,
            const DispatchRecordC* dispatchTable,
            uint8_t numCommands,
            const char** argv,
            uint8_t argvSize):
        CommandDispatcher(streamReader, printer, numCommands, argv, argvSize),
        mDispatchTable(dispatchTable) {}

    /**
     * Find the CommandHandler of the given command name. VisibleForTesting.
     *
     * NOTE: this is currently a linear O(N) scan which is good enough for
     * small number of commands. If we sorted the handlers, we could do a
     * binary search for O(log(N)) and handle larger number of commands.
     */
    static const DispatchRecordC* findCommand(
        const DispatchRecordC* dispatchTable,
        uint8_t numCommands, const char* cmd);

  private:
    /** Print generic help. */
    virtual void helpGeneric(Print& printer) override;

    /** Print helpString of specific cmd. Returns true if cmd was found. */
    virtual bool helpSpecific(Print& printer, const char* cmd) override;

    /** Find and run the given command. */
    virtual void findAndRunCommand(
        const char* cmd, int argc, const char** argv) override;

    const DispatchRecordC* const mDispatchTable;
};

/**
 * A CommandDispatcher that takes DispatchRecordF records using
 * FlashStrings.
 */
class CommandDispatcherF: public CommandDispatcher {
  public:
    /**
     * Constructor.
     *
     * @param streamReader An instance of StreamReader.
     * @param printer The output object, normally the global Serial object.
     * @param dispatchTable An array of DispatchRecords.
     * @param numCommands Number of entries in the dispatchTable.
     * @param argv Array of (const char*) that will be used to hold the word
     * tokens of a command line string.
     * @param argvSize The size of the argv array. Tokens which are beyond this
     * limit will be silently dropped.
     */
    CommandDispatcherF(
            StreamReader& streamReader,
            Print& printer,
            const DispatchRecordF* dispatchTable,
            uint8_t numCommands,
            const char** argv,
            uint8_t argvSize):
        CommandDispatcher(streamReader, printer, numCommands, argv, argvSize),
        mDispatchTable(dispatchTable) {}

    /**
     * Same as findCommand() except use DispatchTableF instead of DispatchTable.
     */
    static const DispatchRecordF* findCommand(
        const DispatchRecordF* dispatchTable,
        uint8_t numCommands, const char* cmd);

  private:
    /** Print generic help. */
    virtual void helpGeneric(Print& printer) override;

    /** Print helpString of specific cmd. Returns true if cmd was found. */
    virtual bool helpSpecific(Print& printer, const char* cmd) override;

    /** Find and run the given command. */
    virtual void findAndRunCommand(
        const char* cmd, int argc, const char** argv) override;

    const DispatchRecordF* const mDispatchTable;
};

}
}

#endif
