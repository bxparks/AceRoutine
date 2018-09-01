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
 * This is a subclass of Coroutine, just like CommandDispatcher. The run()
 * method simply delegates to the underlying CommandDispatcher, so this class
 * can be used as a substitute for CommandDispatcher.
 * 
 * @param T string type of the command names, either 'char' or
 *    '__FlashStringHelper'
 * @param BUF_SIZE size of the input line buffer
 * @param ARGV_SIZE size of the command line argv token table
 */
template<typename T, uint8_t BUF_SIZE, uint8_t ARGV_SIZE>
class CommandManager: public Coroutine {
  public:
    
    /**
     * @param serial the serial port used to read commands and send output,
     * will normally be 'Serial', but can be set to something else.
     * @param tableSize maximum number of commands in the dispatch table
     */
    CommandManager(Stream& serial, uint8_t tableSize):
        mStreamReader(serial, mLineBuffer, BUF_SIZE),
        mDispatchTable(tableSize),
        mDispatcher(mStreamReader, serial, mDispatchTable, mArgv, ARGV_SIZE) {}

    /**
     * Add the given command handler, name and help string to the internal
     * dispatch table.
     */
    void add(CommandHandler command, const T* name, const T* help) {
      mDispatchTable.add(command, name, help);
    }

    virtual int run() override {
      return mDispatcher.run();
    }

  private:
    char mLineBuffer[BUF_SIZE];
    StreamReader mStreamReader;
    DispatchTable<T> mDispatchTable;
    const char* mArgv[ARGV_SIZE];
    CommandDispatcher<T> mDispatcher;
};

}
}

#endif
