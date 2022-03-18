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
#include "CoroutineProfiler.h"

class Print;

namespace ace_routine {

/**
 * Class that manages instances of the `Coroutine` class, and executes them
 * in a round-robin fashion. This is expected to be used as a singleton.
 *
 * Design Notes:
 *
 * Originally, this class was intended to be more substantial, for example,
 * I imagined that different scheduling algorithms could be implemented,
 * allowing coroutines to have different priorities. To ensure that there was
 * only a single instance of the `CoroutineScheduler`, a singleton pattern was
 * used. The `getScheduler()` method fixes the problem that C++ does not
 * guarantee the order of static initialization of resources defined in
 * different files.
 *
 * It seemed cumbersome to require the client code to go through the
 * `getScheduler()` method to access the singleton instance. So each public
 * instance method of the `CoroutineScheduler` is wrapped in a more
 * user-friendly static method to make it easier to use. For example,
 * `CoroutineScheduler::setup()` is a shorthand for
 * `CoroutineScheduler::getScheduler()->setupScheduler()`.
 *
 * As the library matured, I started to put more emphasis on keeping the runtime
 * overhead of the library as small as possible, especially on 8-bit AVR
 * processors, to allow a Coroutine instance to be a viable alternative to
 * writing a normal C/C++ function with complex internal finite state
 * machines. It turned out that the simple round-robin scheduling algorithm
 * currently implemented by `CoroutineScheduler` is good enough, and this class
 * remained quite simple.
 *
 * With its current functionality, the `CoroutineSchedule` does not need to be
 * a singleton because the information that stores the singly-linked list of
 * Coroutines is actually stored in the `Coroutine` class, not the
 * `CoroutineScheduler` class. Because it does not need to be singleton, the
 * `getScheduler()` method is not really required, and we could have just
 * allowed the end-user to explicitly create an instance of `CoroutineScheduler`
 * and use it like a normal object.
 *
 * However, once the API of `CoroutineScheduler` with its static wrapper methods
 * was released to the public, backwards compatibility meant that I could not
 * remove this extra layer of indirection. Fortunately, the none of these
 * methods are virtual, so the extra level of indirection consumes very little
 * overhead, even on 8-bit AVR processors.
 */
template <typename T_COROUTINE>
class CoroutineSchedulerTemplate {
  public:
    /** Set up the scheduler. Should be called from the global setup(). */
    static void setup() { getScheduler()->setupScheduler(); }

    /** Set up the coroutines by calling their setupCoroutine() methods. */
    static void setupCoroutines() {
      getScheduler()->setupCoroutinesInternal();
    }

    /**
     * Run the current coroutine using the current scheduler. This method
     * returns when the underlying Coroutine suspends execution, which allows
     * the system loop() to return to do systems processing.
     * Everyone must cooperate to make the whole thing work.
     */
    static void loop() { getScheduler()->runCoroutine(); }

    /**
     * Run the current coroutine using the current scheduler with the coroutine
     * profiler enabled. This method returns when the underlying Coroutine
     * suspends execution, which allows the system loop() to return to do
     * systems processing. Everyone must cooperate to make the whole thing work.
     */
    static void loopWithProfiler() {
      getScheduler()->runCoroutineWithProfiler();
    }

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

    /** Setup each coroutine by calling its setupCoroutine() function. */
    void setupCoroutinesInternal() {
      for (T_COROUTINE** p = T_COROUTINE::getRoot();
          (*p) != nullptr;
          p = (*p)->getNext()) {

        (*p)->setupCoroutine();
      }
    }

    /**
     * Run the current coroutine without the overhead of the profiler by calling
     * Coroutine::runCoroutine().
     */
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

      // Handle the coroutine's dispatch back to the last known internal status.
      switch ((*mCurrent)->getStatus()) {
        case T_COROUTINE::kStatusYielding:
        case T_COROUTINE::kStatusDelaying:
          // The coroutine itself knows whether it is yielding or delaying, and
          // its continuation context determines whether to call
          // Coroutine::isDelayExpired(), Coroutine::isDelayMicrosExpired(), or
          // Coroutine::isDelaySecondsExpired().
          (*mCurrent)->runCoroutine();
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

    /*
     * Run the current coroutine with profiling enabled by calling
     * Coroutine::runCoroutineWithProfiler().
     */
    void runCoroutineWithProfiler() {
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

      // Handle the coroutine's dispatch back to the last known internal status.
      switch ((*mCurrent)->getStatus()) {
        case T_COROUTINE::kStatusYielding:
        case T_COROUTINE::kStatusDelaying:
          // The coroutine itself knows whether it is yielding or delaying, and
          // its continuation context determines whether to call
          // Coroutine::isDelayExpired(), Coroutine::isDelayMicrosExpired(), or
          // Coroutine::isDelaySecondsExpired().
          //
          // This version calls `Coroutine::runCoroutineWithProfiler()` to
          // enable the profiler.
          (*mCurrent)->runCoroutineWithProfiler();
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
        (*p)->printNameTo(printer);
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
