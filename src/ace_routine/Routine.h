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

#include <stdint.h>

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
  init(nullptr); \
} \
int Routine_##name :: run()

#define ROUTINE2(className, name) \
struct className##_##name : className { \
  className##_##name(); \
  virtual int run() override \
    __attribute__((__noinline__,__noclone__)); \
} name; \
className##_##name :: className##_##name() { \
  init(nullptr); \
} \
int className##_##name :: run()

/**
 * A debug version of ROUTINE that saves the human-readable name of the
 * routine in getName(), at the cost of extra flash and static memory. Meant
 * to be used for debugging and unit tests.
 *
 *   - ROUTINE_NAMED(name) {...}
 *   - ROUTINE_NAMED(className, name) {...}
 */
#define ROUTINE_NAMED(...) \
    GET_ROUTINE_NAMED(\
        __VA_ARGS__, ROUTINE_NAMED2, ROUTINE_NAMED1)(__VA_ARGS__)

#define GET_ROUTINE_NAMED(_1, _2, NAME, ...) NAME

#define ROUTINE_NAMED1(name) \
struct Routine_##name : ace_routine::Routine { \
  Routine_##name(); \
  virtual int run() override \
    __attribute__((__noinline__,__noclone__)); \
} name; \
Routine_##name :: Routine_##name() { \
  init(#name); \
} \
int Routine_##name :: run()

#define ROUTINE_NAMED2(className, name) \
struct className##_##name : className { \
  className##_##name(); \
  virtual int run() override \
    __attribute__((__noinline__,__noclone__)); \
} name; \
className##_##name :: className##_##name() { \
  init(#name); \
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

/** Mark the beginning of a routine loop. Alternative to ROUTINE_BEGIN(). */
#define ROUTINE_LOOP() \
   ROUTINE_BEGIN(); \
   while (true) \

/** Yield execution to another routine. */
#define ROUTINE_YIELD() \
    do { \
      __label__ jumpLabel; \
      setJump(&& jumpLabel); \
      setYield(); \
      return 0; \
      jumpLabel: ; \
    } while (false)

/**
* Yield for delayMillis. A delayMillis of 0 is functionally equivalent to
* ROUTINE_YIELD(). To save memory, the delayMillis is stored as a uint16_t so
* the maximum delay is 65535 milliseconds. If you need to wait for longer than
* that, use a for-loop to call ROUTINE_DELAY() as many times as necessary.
*/
#define ROUTINE_DELAY(delayMillis) \
    do { \
      __label__ jumpLabel; \
      setJump(&& jumpLabel); \
      setDelay(delayMillis); \
      return 0; \
      jumpLabel: ; \
    } while (false)

/**
 * Yield until condition is true, then execution continues.
 * This is functionally equivalent to:
 * @code
 *    while (!condition) ROUTINE_YIELD();
 * @endcode
 */
#define ROUTINE_AWAIT(condition) \
    do { \
      while (!(condition)) ROUTINE_YIELD(); \
    } while (false)

/** Mark the end of a routine. */
#define ROUTINE_END() \
    do { \
      setJump(nullptr); \
      setEnd(); \
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
    // ROUTINE_YIELD(), ROUTINE_DELAY(), and ROUTINE_END() macros.
    typedef uint8_t Status;
    static const Status kStatusYielding = 0;
    static const Status kStatusDelaying = 1;
    static const Status kStatusEnding = 3;
    static const Status kStatusTerminated = 4;

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

    /**
     * Human readable name of the routine. Normally, this will return nullptr
     * when using the ROUTINE() macro. If the ROUTINE_NAMED() macro is used,
     * this will return the human-readable name of the routine.
     */
    const char* getName() const { return mName; }

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

    /** Return the status of the routine. Used by the RoutineScheduler. */
    Status getStatus() const { return mStatus; }

    /** Return the start time of the routine delay. */
    uint16_t getDelayStart() const { return mDelayMillisStart; }

    /** Return the duration of the delay. */
    uint16_t getDelay() const { return mDelayMillisDuration; }

    /** The routine returned using ROUTINE_YIELD(). */
    bool isYielding() const { return mStatus == kStatusYielding; }

    /** The routine returned using ROUTINE_DELAY(). */
    bool isDelaying() const { return mStatus == kStatusDelaying; }

    /** The routine returned using ROUTINE_END(). */
    bool isEnding() const { return mStatus == kStatusEnding; }

    /**
     * The routine in an kStatusEnding state is set to kStatusTerminated when
     * removed from Scheduler queue.
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
     * Initialize the routine and add it to the linked list of routines.
     *
     * @param name The human-readable name of the routine will normally be a
     * nullptr when using the ROUTINE() macro. Use the ROUTINE_NAMED() macro to
     * set to the string name of the routine. This will cost both flash and
     * static memory. It does not seem worth adding extra code to use
     * __FlashStringHelper strings on AVR because we expect this to be used only
     * for debugging and testing.
     */
    void init(const char* name) {
      mName = name;
      insert();
    }

    /** Pointer to label where execute will start on the next call to run(). */
    void setJump(void* jumpPoint) { mJumpPoint = jumpPoint; }

    /** Pointer to label where execute will start on the next call to run(). */
    void* getJump() const { return mJumpPoint; }

    /** Implement the ROUTINE_YIELD() macro. */
    void setYield() { mStatus = kStatusYielding; }

    /** Implement the ROUTINE_DELAY() macro. */
    void setDelay(uint16_t delayMillisDuration) {
      mDelayMillisStart = millis();
      mDelayMillisDuration = delayMillisDuration;
      mStatus = kStatusDelaying;
    }

    /** Implement the ROUTINE_END() macro. */
    void setEnd() { mStatus = kStatusEnding; }

  private:
    // Disable copy-constructor and assignment operator
    Routine(const Routine&) = delete;
    Routine& operator=(const Routine&) = delete;

    /**
     * Insert the current routine into the singly linked list. The order of
     * C++ static initialization is undefined. If getName() is not null (using
     * the ROUTINE_NAMED() macro), the routine will be inserted using
     * getName() as the sorting key. This makes the ordering deterministic,
     * which is required for unit tests.
     */
    void insert();

    const char* mName = nullptr;
    Routine* mNext = nullptr;
    void* mJumpPoint = nullptr;
    Status mStatus = kStatusYielding;
    uint16_t mDelayMillisStart;
    uint16_t mDelayMillisDuration;
};

}

#endif
