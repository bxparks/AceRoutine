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

#ifndef ACE_ROUTINE_COMMAND_MANAGER_H
#define ACE_ROUTINE_COMMAND_MANAGER_H

#include "CommandDispatcher.h"

namespace ace_routine {
namespace cli {

/**
 * A convenience wrapper around a CommandDispatcher that hides complexity of
 * creating, initializing and injecting the resources needed by the
 * CommandDispatcher. It is not strictly necessary to use this, but the setup
 * is much easier using this class.
 *
 * This is a subclass of Coroutine, just like CommandDispatcher. The
 * runCoroutine() method simply delegates to the underlying CommandDispatcher,
 * so this class can be used as a substitute for CommandDispatcher. The
 * setupCoroutine() method should be called to initialize the name of the
 * coroutine and insert it into the CoroutineScheduler.
 *
 * Example usage:
 *
 * @code
 * const uint8_t TABLE_SIZE = 4;
 * const uint8_t BUF_SIZE = 64;
 * const uint8_t ARGV_SIZE = 5;
 * const char PROMPT[] = "$ ";
 *
 * CommandManager<TABLE_SIZE, BUF_SIZE, ARGV_SIZE> commandManager(
 *     Serial, PROMPT);
 *
 * void setup() {
 *   commandManager.add(commandHandler1);
 *   commandManager.add(commandHandler2);
 *   ...
 *   commandManager.setupCommands();
 *
 *   commandManager.setupCoroutine("commandManager");
 *   CoroutineScheduler::setup();
 * }
 * @endcode
 *
 * @param MAX_COMMANDS Maximum number of commands.
 * @param BUF_SIZE Size of the input line buffer.
 * @param ARGV_SIZE Size of the command line argv token list.
 */
template<uint8_t MAX_COMMANDS, uint8_t BUF_SIZE, uint8_t ARGV_SIZE>
class CommandManager: public Coroutine {
  public:

    /**
     * Constructor.
     *
     * @param serial The serial port used to read commands and send output,
     *        will normally be 'Serial', but can be set to something else.
     * @param prompt If not null, print a prompt and echo the command entered
     *        by the user. If null, don't print the prompt and don't echo the
     *        input from the user.
     */
    CommandManager(Stream& serial, const char* prompt = nullptr):
        mSerial(serial),
        mPrompt(prompt),
        mStreamReader(serial, mLineBuffer, BUF_SIZE) {}

    virtual ~CommandManager() {
      if (mDispatcher) {
        delete mDispatcher;
      }
    }

    /** Add the given command handler to the list of commands. */
    void add(const CommandHandler* command) {
      if (mNumCommands < MAX_COMMANDS) {
        mCommands[mNumCommands++] = command;
      }
    }

    void setupCommands() {
      mDispatcher = new CommandDispatcher(mStreamReader, mSerial, mCommands,
          mNumCommands, mArgv, ARGV_SIZE, mPrompt);
    }

    virtual int runCoroutine() override {
      return mDispatcher->runCoroutine();
    }

    /** Return the CommandDispatcher. VisibleForTesting. */
    const CommandDispatcher* getDispatcher() const { return mDispatcher; }

  private:
    Stream& mSerial;
    const char* const mPrompt;
    StreamReader mStreamReader;
    const CommandHandler* mCommands[MAX_COMMANDS];
    char mLineBuffer[BUF_SIZE];
    const char* mArgv[ARGV_SIZE];

    CommandDispatcher* mDispatcher = nullptr;
    uint8_t mNumCommands = 0;
};

}
}

#endif
