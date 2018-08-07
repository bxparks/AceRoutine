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

#ifndef ACE_ROUTINE_COROUTINE_SCHEDULER_H
#define ACE_ROUTINE_COROUTINE_SCHEDULER_H

#include <Print.h>
#include "Coroutine.h"

namespace ace_routine {

/**
 * Class that manages instances of the `Coroutine` class, and executes them
 * in a round-robin fashion.
 */
class CoroutineScheduler {
  public:
    /** Set up the scheduler. Should be called from the global setup(). */
    static void setup() { getScheduler()->setupScheduler(); }

    /**
     * Run the current coroutine using the current scheduler. This method
     * returns when the underlying Coroutine suspends execution, which allows
     * the system loop() to return to do systems processing, such as WiFi.
     * Everyone must cooperate to make the whole thing work.
     */
    static void loop() { getScheduler()->runCoroutine(); }

    /**
     * Print out the known coroutines to the printer (usually Serial). Note that
     * if this method is never called, the linker will strip out the code. If
     * Serial is never configured in setup(), then this method causes no
     * additional flash memory consumption.
     */
    static void list(Print& printer) {
      getScheduler()->listCoroutines(printer);
    }

  private:
    // Disable copy-constructor and assignment operator
    CoroutineScheduler(const CoroutineScheduler&) = delete;
    CoroutineScheduler& operator=(const CoroutineScheduler&) = delete;

    /** Return the singleton CoroutineScheduler. */
    static CoroutineScheduler* getScheduler();

    /** Constructor. */
    CoroutineScheduler() {}

    /** Set up the Scheduler. */
    void setupScheduler();

    /** Run the current coroutine. */
    void runCoroutine();

    /** List all the routines in the linked list to the printer. */
    void listCoroutines(Print& printer);

    // The current coroutine is represented by a pointer to a pointer. This
    // allows the root node to be treated the same as all the other nodes, and
    // simplifies the code that traverses the singly-linked list.
    Coroutine** mCurrent = nullptr;
};

}

#endif
