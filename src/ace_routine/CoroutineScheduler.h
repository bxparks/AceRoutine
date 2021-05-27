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

#if ACE_ROUTINE_DEBUG == 1
  #include <Arduino.h> // Serial, Print
#endif
#include "Coroutine.h"

class Print;

namespace ace_routine {

/**
 * Class that manages instances of the `Coroutine` class, and executes them
 * in a round-robin fashion. This is expected to be used as a singleton.
 */
template <typename T_COROUTINE>
class CoroutineSchedulerTemplate {
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
    CoroutineSchedulerTemplate(const CoroutineSchedulerTemplate&) = delete;
    CoroutineSchedulerTemplate& operator=(const CoroutineSchedulerTemplate&) =
        delete;

    /** Return the singleton CoroutineScheduler. */
    static CoroutineSchedulerTemplate* getScheduler() {
      static CoroutineSchedulerTemplate singletonScheduler;
      return &singletonScheduler;
    }

    /** Constructor. */
    CoroutineSchedulerTemplate() = default;

    /**
     * Set up the Scheduler.
     *
     * Prior to v1.2, this would perform a pre-scan through the linked list to
     * remove coroutines which were Suspended. But this caused a cycle in the
     * list if resume() was called immediately after the suspend(). For v1.2 and
     * onwards, we keep all coroutines in the linked list no matter the state,
     * which makes the state management and linked-list management a lot
     * simpler.
     */
    void setupScheduler() {
      mCurrent = T_COROUTINE::getRoot();
    }

    /** Run the current coroutine. */
    void runCoroutine() {
      // If reached the end, start from the beginning again.
      if (*mCurrent == nullptr) {
        mCurrent = T_COROUTINE::getRoot();
        // Return if the list is empty. Checking for a null getRoot() inside the
        // if-statement is deliberate, since it optimizes the common case where
        // the linked list is not empty.
        if (*mCurrent == nullptr) {
          return;
        }
      }

    #if ACE_ROUTINE_DEBUG == 1
      Serial.print(F("Processing "));
      (*mCurrent)->getName().printTo(Serial);
      Serial.println();
    #endif

      // Handle the coroutine's dispatch back to the last known internal status.
      switch ((*mCurrent)->getStatus()) {
        case T_COROUTINE::kStatusYielding:
          (*mCurrent)->runCoroutine();
          break;

        case T_COROUTINE::kStatusDelaying:
          // Check isDelayExpired() here to optimize away an extra call into the
          // Coroutine::runCoroutine(). Everything would still work if we just
          // dispatched into the Coroutine::runCoroutine() because that method
          // checks isDelayExpired() as well.
          if ((*mCurrent)->isDelayExpired()) {
            (*mCurrent)->runCoroutine();
          }
          break;

        case T_COROUTINE::kStatusEnding:
          // mark it terminated
          (*mCurrent)->setTerminated();
          break;

        default:
          // For all other cases, just skip to the next coroutine.
          break;
      }

      // Go to the next coroutine
      mCurrent = (*mCurrent)->getNext();
    }


    /** List all the routines in the linked list to the printer. */
    void listCoroutines(Print& printer) {
      for (T_COROUTINE** p = T_COROUTINE::getRoot(); (*p) != nullptr;
          p = (*p)->getNext()) {
        printer.print(F("Coroutine "));
        (*p)->getName().printTo(printer);
        printer.print(F("; status: "));
        (*p)->statusPrintTo(printer);
        printer.println();
      }
    }

    // The current coroutine is represented by a pointer to a pointer. This
    // allows the root node to be treated the same as all the other nodes, and
    // simplifies the code that traverses the singly-linked list.
    T_COROUTINE** mCurrent = nullptr;
};

using CoroutineScheduler = CoroutineSchedulerTemplate<Coroutine>;

}

#endif
