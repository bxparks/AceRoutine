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

#ifndef ACE_ROUTINE_ROUTINE_H
#define ACE_ROUTINE_ROUTINE_H

#include <stdint.h> // UINT16_MAX
#include <Print.h> // Print
#include "Flash.h" // ACE_ROUTINE_F()
#include "FCString.h"

/**
 * @file Routine.h
 *
 * All (co)routines are instances of the Routine base class. The ROUTINE() macro
 * creates these instances, and registers them to automatically run when
 * RoutineScheduler::loop() is called.
 *
 * Various macros use macro overloading to implement a 1-argument and
 * a 2-argument version. See
 * https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
 * to description of how that works.
 *
 * The computed goto is a GCC extension:
 * https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html
 * The __noinline__ and __noclone__ attributes make sure that label pointers are
 * always the same. I'm not 100% sure they are needed here, but they don't seem
 * to hurt.
 */

/**
 * Create a Routine instance named 'name'. Two forms are supported
 *
 *   - ROUTINE(name) {...}
 *   - ROUTINE(className, name) {...}
 *
 * The 1-argument form uses the Routine class as the base class of the
 * routine. The 2-argument form uses the user-provided className which must be
 * a subclass of Routine.
 *
 * The code in {} following this macro becomes the body of the
 * Routine::run() method.
 */
#define ROUTINE(...) \
    GET_ROUTINE(__VA_ARGS__, ROUTINE2, ROUTINE1)(__VA_ARGS__)

#define GET_ROUTINE(_1, _2, NAME, ...) NAME

#define ROUTINE1(name) \
struct Routine_##name : ace_routine::Routine { \
  Routine_##name(); \
  virtual int run() override \
    __attribute__((__noinline__,__noclone__)); \
} name; \
Routine_##name :: Routine_##name() { \
  init(ACE_ROUTINE_F(#name)); \
} \
int Routine_##name :: run()

#define ROUTINE2(className, name) \
struct className##_##name : className { \
  className##_##name(); \
  virtual int run() override \
    __attribute__((__noinline__,__noclone__)); \
} name; \
className##_##name :: className##_##name() { \
  init(ACE_ROUTINE_F(#name)); \
} \
int className##_##name :: run()

/**
 * Create an extern reference to a routine that is defined in another .cpp
 * file. The extern reference is needed before it can be used. Two forms are
 * supported:
 *
 *    - EXTERN_ROUTINE(name);
 *    - EXTERN_ROUTINE(className, name);
 */
#define EXTERN_ROUTINE(...) \
    GET_EXTERN_ROUTINE(\
        __VA_ARGS__, EXTERN_ROUTINE2, EXTERN_ROUTINE1)(__VA_ARGS__)

#define GET_EXTERN_ROUTINE(_1, _2, NAME, ...) NAME

#define EXTERN_ROUTINE1(name) \
struct Routine_##name : ace_routine::Routine { \
  Routine_##name(); \
  virtual int run() override \
    __attribute__((__noinline__,__noclone__)); \
}; \
extern Routine_##name name

#define EXTERN_ROUTINE2(className, name) \
struct className##_##name : className { \
  className##_##name(); \
  virtual int run() override \
    __attribute__((__noinline__,__noclone__)); \
}; \
extern className##_##name name

/** Mark the beginning of a routine. */
#define ROUTINE_BEGIN() \
    void* p = getJump(); \
    if (p != nullptr) { \
      goto *p; \
    }

/**
 * Mark the beginning of a routine loop. Can be used instead of
 * ROUTINE_BEGIN() at the beginning of a Routine.
 */
#define ROUTINE_LOOP() \
   ROUTINE_BEGIN(); \
   while (true) \

#define ROUTINE_YIELD_INTERNAL() \
    do { \
      __label__ jumpLabel; \
      setJump(&& jumpLabel); \
      return 0; \
      jumpLabel: ; \
    } while (false)

/** Yield execution to another routine. */
#define ROUTINE_YIELD() \
    do { \
      setYielding(); \
      ROUTINE_YIELD_INTERNAL(); \
      setRunning(); \
    } while (false)

/**
 * Yield until condition is true, then execution continues. This is
 * functionally equivalent to:
 *
 * @code
 *    while (!condition) ROUTINE_YIELD();
 * @endcode
 *
 * but the getStatus() during the waiting is set to kStatusAwaiting instead of
 * kStatusYielding. The current scheduler treats the two states the same, but
 * it's possible that a different scheduler may want to treat them differently.
 */
#define ROUTINE_AWAIT(condition) \
    do { \
      while (!(condition)) { \
        setAwaiting(); \
        ROUTINE_YIELD_INTERNAL(); \
      } \
      setRunning(); \
    } while (false)

/**
* Yield for delayMillis. A delayMillis of 0 is functionally equivalent to
* ROUTINE_YIELD(). To save memory, the delayMillis is stored as a uint16_t so
* the maximum delay is technically 65535 milliseconds. However, to avoid an
* edge-case when using RoutineSchedule, the practical maximum of 65534
* milliseconds is imposed by the Routine::delay() method.
*
* If you need to wait for longer than that, use a for-loop to call
* ROUTINE_DELAY() as many times as necessary.
*
* This could have been implemented using ROUTINE_AWAIT() but this macro matches
* the global delay(millis) function already provided by the Arduino API, and
* the separate kStatusDelaying allows the scheduler to perform some
* optimization.
*/
#define ROUTINE_DELAY(delayMillis) \
    do { \
      setDelay(delayMillis); \
      while (!isDelayExpired()) { \
        setDelaying(); \
        ROUTINE_YIELD_INTERNAL(); \
      } \
      setRunning(); \
    } while (false)

/**
 * Mark the end of a routine. Subsequent calls to Routine::run() will do
 * nothing.
 */
#define ROUTINE_END() \
    do { \
      __label__ jumpLabel; \
      setEnding(); \
      setJump(&& jumpLabel); \
      jumpLabel: ; \
      return 0; \
    } while (false)

namespace ace_routine {

/**
 * Base class of all routines. The actual routine code is an implementation
 * of the run() method.
 */
class Routine {
  public:
    // The execution recovery status of the routine, corresponding to the
    // ROUTINE_YIELD(), ROUTINE_DELAY(), ROUTINE_AWAIT() and ROUTINE_END()
    // macros.
    typedef uint8_t Status;
    static const Status kStatusSuspended = 0;
    static const Status kStatusRunning = 1;
    static const Status kStatusYielding = 2;
    static const Status kStatusAwaiting = 3;
    static const Status kStatusDelaying = 4;
    static const Status kStatusEnding = 5;
    static const Status kStatusTerminated = 6;

    /**
     * Get the pointer to the root pointer. Implemented as a function static to
     * fix the C++ static initialization problem, making it safe to use this in
     * other static contexts.
     */
    static Routine** getRoot();

    /**
     * Return the next pointer as a pointer to the pointer, similar to
     * getRoot(). This makes it much easier to manipulate a singly-linked list.
     * Also makes setNext() method unnecessary.
     */
    Routine** getNext() { return &mNext; }

    /** Human-readable name of the routine. */
    const FCString& getName() const { return mName; }

    /**
     * The body of the routine. The return value is never used. It exists
     * solely to prevent the various ROUTINE_YIELD(), ROUTINE_DELAY()
     * and ROUTINE_END() macros from accidentally compiling inside a nested
     * method.
     *
     * @return The return value is always ignored. This method is declared to
     * return an int to prevent the user from accidentally returning from this
     * method incorrectly. This method should always return using one of the
     * ROUTINE_YIELD(), ROUTINE_DELAY() or ROUTINE_END() macros, or not
     * return at all is using the ROUTINE_LOOP() macro.
     */
    virtual int run() = 0;

    /**
     * Returns the current millisecond clock. By default it returns the global
     * millis() function from Arduino but can be overridden for testing.
     */
    virtual unsigned long millis() const;

    /**
     * Suspend the routine at the next scheduler iteration. If the routine is
     * already in the process of ending or is already terminated, then this
     * method does nothing.
     */
    void suspend() {
      if (mStatus == kStatusTerminated || mStatus == kStatusEnding) return;
      mStatus = kStatusSuspended;
    }

    /**
     * Add a Suspended routine into the head of the scheduler linked list, and
     * change the state to Yielding. If the routine is in any other state, this
     * method does nothing.
     */
    void resume();

    /** Return the status of the routine. Used by the RoutineScheduler. */
    Status getStatus() const { return mStatus; }

    /** Check if delay time is over. */
    bool isDelayExpired() {
      uint16_t elapsedMillis = millis() - mDelayStartMillis;
      return elapsedMillis >= mDelayDurationMillis;
    }

    /** The routine is currently running. True only within the routine. */
    bool isRunning() const { return mStatus == kStatusRunning; }

    /** The routine returned using ROUTINE_YIELD(). */
    bool isYielding() const { return mStatus == kStatusYielding; }

    /** The routine returned using ROUTINE_AWAIT(). */
    bool isAwaiting() const { return mStatus == kStatusAwaiting; }

    /** The routine returned using ROUTINE_DELAY(). */
    bool isDelaying() const { return mStatus == kStatusDelaying; }

    /** The routine returned using ROUTINE_END(). */
    bool isEnding() const { return mStatus == kStatusEnding; }

    /** The routine was suspended with a call to suspend(). */
    bool isSuspended() const { return mStatus == kStatusSuspended; }

    /**
     * The routine was in isEnding() state and has been removed from the
     * RoutineScheduler queue. This method works only if the Routine is
     * executed using the RoutineScheduler. If the Routine is invoked directly,
     * then use isEnding().
     */
    bool isTerminated() const { return mStatus == kStatusTerminated; }

    /**
     * Indicate that the Routine has been removed from the Scheduler queue.
     * Should be used only by the RoutineScheduler.
     */
    void setTerminated() { mStatus = kStatusTerminated; }

  protected:
    /** Constructor. */
    Routine() {}

    /**
     * Initialize the routine, set it to Yielding state, and add it to the
     * linked list of routines.
     *
     * @param name The name of the routine as a human-readable string.
     */
    void init(const char* name) {
      mName = FCString(name);
      mStatus = kStatusYielding;
      insertSorted();
    }

    /** Same as init(const char*) except using flash string type. */
    void init(const __FlashStringHelper* name) {
      mName = FCString(name);
      mStatus = kStatusYielding;
      insertSorted();
    }

    /** Pointer to label where execute will start on the next call to run(). */
    void setJump(void* jumpPoint) { mJumpPoint = jumpPoint; }

    /** Pointer to label where execute will start on the next call to run(). */
    void* getJump() const { return mJumpPoint; }

    /** Set the kStatusRunning state. */
    void setRunning() { mStatus = kStatusRunning; }

    /** Set the kStatusDelaying state. */
    void setYielding() { mStatus = kStatusYielding; }

    /** Set the kStatusAwaiting state. */
    void setAwaiting() { mStatus = kStatusAwaiting; }

    /** Set the kStatusDelaying state. */
    void setDelaying() { mStatus = kStatusDelaying; }

    /**
     * Configure the delay timer. The maximum duration is (UINT16_MAX-1) (i.e.
     * 65534) to avoid an edge-case when using the RoutineScheduler to optimize
     * the ROUTINE_DELAY() macro. If UINT16_MAX is given, the duration is set
     * to (UINT16_MAX-1).
     */
    void setDelay(uint16_t delayMillisDuration) {
      mDelayStartMillis = millis();
      mDelayDurationMillis = (delayMillisDuration == UINT16_MAX)
          ? UINT16_MAX - 1
          : delayMillisDuration;
    }

    /** Set the kStatusEnding state. */
    void setEnding() { mStatus = kStatusEnding; }

  private:
    // Disable copy-constructor and assignment operator
    Routine(const Routine&) = delete;
    Routine& operator=(const Routine&) = delete;

    /**
     * Insert the current routine into the singly linked list. The order of
     * C++ static initialization is undefined, but if getName() is not null
     * (which will normally be the case when using the ROUTINE() macro), the
     * routine will be inserted using getName() as the sorting key. This makes
     * the ordering deterministic, which is required for unit tests.
     *
     * The insertion algorithm is O(N) per insertion, for a total complexity
     * of O(N^2). That's probably good enough for a "small" number of routines,
     * where small is around O(100). If a large number of routines are
     * inserted, then this method needs to be optimized.
     */
    void insertSorted();

    FCString mName;
    Routine* mNext = nullptr;
    void* mJumpPoint = nullptr;
    Status mStatus = kStatusSuspended;
    uint16_t mDelayStartMillis;
    uint16_t mDelayDurationMillis;
};

}

#endif
