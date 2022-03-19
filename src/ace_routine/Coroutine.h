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

#ifndef ACE_ROUTINE_COROUTINE_H
#define ACE_ROUTINE_COROUTINE_H

#include <stdint.h> // UINT16_MAX
#include <Print.h> // Print
#include <AceCommon.h> // PrintStr<>
#include "CoroutineProfiler.h"
#include "ClockInterface.h"
#include "compat.h" // PROGMEM

class __FlashStringHelper;
class AceRoutineTest_statusStrings;
class SuspendTest_suspendAndResume;

/**
 * @file Coroutine.h
 *
 * All coroutines are instances of the Coroutine base class. The COROUTINE()
 * macro creates these instances, and registers them to automatically run when
 * CoroutineScheduler::loop() is called.
 *
 * Various macros use macro overloading to implement a 1-argument and
 * a 2-argument version. See https://stackoverflow.com/questions/11761703 to
 * description of how that works.
 *
 * The computed goto is a GCC extension:
 * https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html
 * The __noinline__ and __noclone__ attributes make sure that label pointers are
 * always the same. I'm not 100% sure they are needed here, but they don't seem
 * to hurt.
 */

// https://stackoverflow.com/questions/295120
/** Macro that indicates a deprecation. */
#if defined(__GNUC__) || defined(__clang__)
  #define ACE_ROUTINE_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
  #define ACE_ROUTINE_DEPRECATED __declspec(deprecated)
#else
  #pragma message("WARNING: Implement ACE_ROUTINE_DEPRECATED for this compiler")
  #define ACE_ROUTINE_DEPRECATED
#endif

/**
 * Create a Coroutine instance named 'name'. Two forms are supported
 *
 *   - COROUTINE(name) {...}
 *   - COROUTINE(className, name) {...}
 *
 * The 1-argument form uses the Coroutine class as the base class of the
 * coroutine. The 2-argument form uses the user-provided className which must be
 * a subclass of Coroutine.
 *
 * The code in {} following this macro becomes the body of the
 * Coroutine::runCoroutine() method.
 */
#define COROUTINE(...) \
    GET_COROUTINE(__VA_ARGS__, COROUTINE2, COROUTINE1)(__VA_ARGS__)

/** Internal helper macro to allow overloading of the COROUTINE() macro. */
#define GET_COROUTINE(_1, _2, NAME, ...) NAME

/** Implement the 1-argument COROUTINE() macro. */
#define COROUTINE1(name) \
struct Coroutine_##name : ace_routine::Coroutine { \
  Coroutine_##name(); \
  int runCoroutine() override; \
} name; \
Coroutine_##name :: Coroutine_##name() { \
} \
int Coroutine_##name :: runCoroutine()

/** Implement the 2-argument COROUTINE() macro. */
#define COROUTINE2(className, name) \
struct className##_##name : className { \
  className##_##name(); \
  int runCoroutine() override; \
} name; \
className##_##name :: className##_##name() { \
} \
int className##_##name :: runCoroutine()

/**
 * Create an extern reference to a coroutine that is defined in another .cpp
 * file. The extern reference is needed before it can be used. Two forms are
 * supported:
 *
 *    - EXTERN_COROUTINE(name);
 *    - EXTERN_COROUTINE(className, name);
 */
#define EXTERN_COROUTINE(...) \
    GET_EXTERN_COROUTINE(\
        __VA_ARGS__, EXTERN_COROUTINE2, EXTERN_COROUTINE1)(__VA_ARGS__)

/**
 * Internal helper macro to allow overloading of the EXTERN_COROUTINE() macro.
 */
#define GET_EXTERN_COROUTINE(_1, _2, NAME, ...) NAME

/** Implement the 1-argument EXTERN_COROUTINE() macro. */
#define EXTERN_COROUTINE1(name) \
struct Coroutine_##name : ace_routine::Coroutine { \
  Coroutine_##name(); \
  int runCoroutine() override; \
}; \
extern Coroutine_##name name

/** Implement the 2-argument EXTERN_COROUTINE() macro. */
#define EXTERN_COROUTINE2(className, name) \
struct className##_##name : className { \
  className##_##name(); \
  int runCoroutine() override; \
}; \
extern className##_##name name

/** Mark the beginning of a coroutine. */
#define COROUTINE_BEGIN() \
    void* p = this->getJump(); \
    if (p != nullptr) { \
      goto *p; \
    }

/**
 * Mark the beginning of a coroutine loop. Can be used instead of
 * COROUTINE_BEGIN() at the beginning of a Coroutine.
 */
#define COROUTINE_LOOP() \
   COROUTINE_BEGIN(); \
   while (true) \

/**
 * Implement the common logic for COROUTINE_YIELD(), COROUTINE_AWAIT(),
 * COROUTINE_DELAY().
 */
#define COROUTINE_YIELD_INTERNAL() \
    do { \
      __label__ jumpLabel; \
      this->setJump(&& jumpLabel); \
      return 0; \
      jumpLabel: ; \
    } while (false)

/** Yield execution to another coroutine. */
#define COROUTINE_YIELD() \
    do { \
      this->setYielding(); \
      COROUTINE_YIELD_INTERNAL(); \
      this->setRunning(); \
    } while (false)

/**
 * Yield until condition is true, then execution continues. This is
 * functionally equivalent to:
 *
 * @code
 *    while (!condition) COROUTINE_YIELD();
 * @endcode
 *
 * but potentially slightly more efficient.
 */
#define COROUTINE_AWAIT(condition) \
    do { \
      this->setYielding(); \
      do { \
        COROUTINE_YIELD_INTERNAL(); \
      } while (!(condition)); \
      this->setRunning(); \
    } while (false)

/**
 * Yield for delayMillis. A delayMillis of 0 is functionally equivalent to
 * COROUTINE_YIELD(). To save memory, the delayMillis is stored as a uint16_t
 * but the actual maximum is limited to 32767 millliseconds. See
 * setDelayMillis() for the reason for this limitation.
 *
 * If you need to wait for longer than that, use a for-loop to call
 * COROUTINE_DELAY() as many times as necessary.
 *
 * This could have been implemented using COROUTINE_AWAIT() but this macro
 * matches the global delay(millis) function already provided by the Arduino
 * API. Also having a separate kStatusDelaying state allows the
 * CoroutineScheduler to be slightly more efficient by avoiding the call to
 * Coroutine::runCoroutine() if the delay has not expired.
 */
#define COROUTINE_DELAY(delayMillis) \
    do { \
      this->setDelayMillis(delayMillis); \
      this->setDelaying(); \
      do { \
        COROUTINE_YIELD_INTERNAL(); \
      } while (!this->isDelayExpired()); \
      this->setRunning(); \
    } while (false)

/** Yield for delayMicros. Similiar to COROUTINE_DELAY(delayMillis). */
#define COROUTINE_DELAY_MICROS(delayMicros) \
    do { \
      this->setDelayMicros(delayMicros); \
      this->setDelaying(); \
      do { \
        COROUTINE_YIELD_INTERNAL(); \
      } while (!this->isDelayMicrosExpired()); \
      this->setRunning(); \
    } while (false)

/**
 * Yield for delaySeconds. Similar to COROUTINE_DELAY(delayMillis).
 *
 * The accuracy of the delay interval in units of seconds is not perfectly
 * accurate. The current implementation uses the builtin millis() to infer the
 * "seconds". The millis() function returns a value that overflows after
 * 4,294,967.296 seconds. Therefore, the last inferred second just before
 * overflowing contains only 0.296 seconds instead of a full second. A delay
 * which straddles this overflow will return 0.704 seconds earlier than it
 * should.
 *
 * On microcontrollers without hardware integer division instruction, (i.e. AVR,
 * SAMD21, ESP8266), the division by 1000 is relatively slow and consumes
 * significant amount of flash memory (100-150 bytes on AVR).
 */
#define COROUTINE_DELAY_SECONDS(delaySeconds) \
    do { \
      this->setDelaySeconds(delaySeconds); \
      this->setDelaying(); \
      do { \
        COROUTINE_YIELD_INTERNAL(); \
      } while (!this->isDelaySecondsExpired()); \
      this->setRunning(); \
    } while (false)

/**
 * Mark the end of a coroutine. Subsequent calls to Coroutine::runCoroutine()
 * will do nothing.
 */
#define COROUTINE_END() \
    do { \
      __label__ jumpLabel; \
      this->setEnding(); \
      this->setJump(&& jumpLabel); \
      jumpLabel: ; \
      return 0; \
    } while (false)

namespace ace_routine {

/** A lookup table from Status integer to human-readable strings. */
extern const __FlashStringHelper* const sStatusStrings[] PROGMEM;

// Forward declaration of CoroutineSchedulerTemplate<T>
template <typename T> class CoroutineSchedulerTemplate;

/**
 * Base class of all coroutines. The actual coroutine code is an implementation
 * of the virtual runCoroutine() method.
 *
 * @tparam T_CLOCK class that provides micros(), millis() and seconds()
 *    functions, usually `ClockInterface` but can be something else for testing
 *    purposes.
 * @tparam T_DELAY type used to store the mDelayStart and mDelayDuration,
 *    usually `uint16_t`. It may be possible to create a variant set of
 *    Coroutine32 and CoroutineScheduler32 classes which use a `uint32_t`
 *    instead of a `uint16_t`. This would probably allow the
 *    `COROUTINE_DELAY()`, `COROUTINE_DELAY_MICROS()` and
 *    `COROUTINE_DELAY_SECONDS()` macros to accept 32-bit integers instead of
 *    16-bits. I have not tested this possibility at all.
 */
template <typename T_CLOCK, typename T_DELAY>
class CoroutineTemplate {
  friend class CoroutineSchedulerTemplate<CoroutineTemplate<T_CLOCK, T_DELAY>>;
  friend class ::AceRoutineTest_statusStrings;
  friend class ::SuspendTest_suspendAndResume;

  public:
    /** Coroutine name is a `const char*` c-string. */
    static const uint8_t kNameTypeCString = 0;

    /** Coroutine name is a `const __FlashStringHelper*` f-string. */
    static const uint8_t kNameTypeFString = 1;

  public:
    /**
     * The body of the coroutine. The COROUTINE macro creates a subclass of
     * this class and puts the body of the coroutine into this method.
     *
     * @return The return value is always ignored. This method is declared to
     * return an int to prevent the user from accidentally returning from this
     * method using an explicit 'return' statement instead of through one of
     * the macros (e.g. COROUTINE_YIELD(), COROUTINE_DELAY(), COROUTINE_AWAIT()
     * or COROUTINE_END()).
     */
    virtual int runCoroutine() = 0;

    /**
     * Perform coroutine initialization. This is intended to be called directly
     * from the global `setup()` function, or through the
     * `CoroutineScheduler::setupCoroutines()` method which should also be
     * called from the global `setup()` function.
     *
     * If your coroutines do not override this method, hence do not need to
     * perform any setup, then you should *not* call
     * `CoroutineScheduler::setupCoroutines()` to avoid consuming unnecessary
     * flash memory. On AVR processors, each `Coroutine::setupCoroutine()` seems
     * to consume at least 50-60 bytes of flash memory overhead per coroutine.
     * On 32-bit processors, the overhead seems to be only about 30-40 bytes per
     * coroutine.
     */
    virtual void setupCoroutine() {}

    /**
     * This is a variant of runCoroutine() which measures the execution time of
     * runCoroutine() and updates the attached profiler if it exists.
     *
     * On 8-bit processors, memory consumption can be reduced by calling the
     * `Coroutine::runCoroutine()` method directly in the global `loop()`
     * function, instead of using the `CoroutineScheduler`. In such an
     * environment, the end-user can enable profiling by manually changing the
     * calls to `Coroutine::runCoroutine()` to
     * `Coroutine::runCoroutineWithProfiler()`, and recompiling the program.
     */
    int runCoroutineWithProfiler() {
      if (mProfiler) {
        uint32_t startMicros = coroutineMicros();
        runCoroutine();
        uint32_t elapsedMicros = coroutineMicros() - startMicros;
        mProfiler->updateElapsedMicros(elapsedMicros);
        return 0;
      } else {
        return runCoroutine();
      }
    }

    /**
     * Return the type of the name string, either kNameTypeCString or
     * kNameTypeFString.
     */
    uint8_t getNameType() const { return mNameType; }

    /** Set the name of the coroutine to the given c-string. */
    void setName(const char* name) {
      mNameType = kNameTypeCString;
      mName = name;
    }

    /** Set the name of the coroutine to the given f-string. */
    void setName(const __FlashStringHelper* name) {
      mNameType = kNameTypeFString;
      mName = name;
    }

    /** Get name of the coroutine assuming it's a c-string. Nullable. */
    const char* getCName() const { return (const char*) mName; }

    /** Get name of the coroutine assuming it's an f-string. Nullable. */
    const __FlashStringHelper* getFName() const {
      return (const __FlashStringHelper*) mName;
    }

    /**
     * Print name to the given Printer. If the name is null, then print the
     * hexadecimal representation of the pointer to the coroutine.
     *
     * @param printer destination of output, usually `Serial`
     * @param maxLen truncate or pad to maxLen if given
     */
    void printNameTo(Print& printer, uint8_t maxLen = 0) const {
      // Need to go through this contortion because vsnprintf() does not support
      // flash string parameters, so I can't use something like "%12.12s" with
      // a flash string.
      ace_common::PrintStr<64> pname;

      if (mName == nullptr) {
        pname.print("0x");
        pname.print((uintptr_t) this, 16);
      } else if (mNameType == kNameTypeCString) {
        pname.print((const char*) mName);
      } else {
        pname.print((const __FlashStringHelper*) mName);
      }

      // Print name, truncated to maxLen or padded to maxLen.
      if (maxLen) {
        if (pname.length() < maxLen) {
          printer.write(pname.cstr());
          for (uint8_t i = pname.length(); i < maxLen; i++) {
            printer.write(' ');
          }
        } else {
          // NOTE: Must cast to (const uint8_t*) because the ATtiny85 core does
          // not have a version of write() that accepts a (const char*), in
          // contrast to every other Arduino Core.
          printer.write((const uint8_t*) pname.cstr(), maxLen);
        }
      } else {
        printer.write(pname.cstr());
      }
    }

    /**
     * Suspend the coroutine at the next scheduler iteration. If the coroutine
     * is already in the process of ending or is already terminated, then this
     * method does nothing. A coroutine cannot use this method to suspend
     * itself, it can only suspend some other coroutine. Currently, there is no
     * ability for a coroutine to suspend itself. I think that would require the
     * addition of a COROUTINE_SUSPEND() macro. Also, this method works only if
     * the CoroutineScheduler::loop() is used because the suspend functionality
     * is implemented by the CoroutineScheduler.
     */
    void suspend() {
      if (isDone()) return;
      mStatus = kStatusSuspended;
    }

    /**
     * Add a Suspended coroutine into the head of the scheduler linked list,
     * and change the state to Yielding. If the coroutine is in any other
     * state, this method does nothing. This method works only if the
     * CoroutineScheduler::loop() is used.
     */
    void resume() {
      if (mStatus != kStatusSuspended) return;

      // We lost the original state of the coroutine when suspend() was called
      // but the coroutine will automatically go back into the original state
      // when Coroutine::runCoroutine() is called because COROUTINE_YIELD(),
      // COROUTINE_DELAY() and COROUTINE_AWAIT() are written to restore their
      // status.
      mStatus = kStatusYielding;
    }

    /**
     * Reset the coroutine to its initial state. Only the Coroutine base-class
     * state is reset to the original state. If the subclass runCoroutine()
     * uses any static variables (for example, a loop counter), you must reset
     * those variables manually as well, since this library does not have any
     * knowledge about them.
     *
     * It is expected that this method will be called from outside the
     * runCoroutine() method. If it is called within the method, I'm not sure
     * what will happen. I think the coroutine will abandon the current
     * continuation point, and start executing from the beginning of the
     * Coroutine upon the next iteration.
     */
    void reset() {
      mStatus = kStatusYielding;
      mJumpPoint = nullptr;
    }

    /** Check if delay millis time is over. */
    bool isDelayExpired() const {
      T_DELAY nowMillis = coroutineMillis();
      T_DELAY elapsed = nowMillis - mDelayStart;
      return elapsed >= mDelayDuration;
    }

    /** Check if delay micros time is over. */
    bool isDelayMicrosExpired() const {
      T_DELAY nowMicros = coroutineMicros();
      T_DELAY elapsed = nowMicros - mDelayStart;
      return elapsed >= mDelayDuration;
    }

    /** Check if delay seconds time is over. */
    bool isDelaySecondsExpired() const {
      T_DELAY nowSeconds = coroutineSeconds();
      T_DELAY elapsed = nowSeconds - mDelayStart;
      return elapsed >= mDelayDuration;
    }

    /** The coroutine was suspended with a call to suspend(). */
    bool isSuspended() const { return mStatus == kStatusSuspended; }

    /** The coroutine returned using COROUTINE_YIELD(). */
    bool isYielding() const { return mStatus == kStatusYielding; }

    /** The coroutine returned using COROUTINE_DELAY(). */
    bool isDelaying() const { return mStatus == kStatusDelaying; }

    /** The coroutine is currently running. True only within the coroutine. */
    bool isRunning() const { return mStatus == kStatusRunning; }

    /**
     * The coroutine returned using COROUTINE_END(). In most cases, isDone() is
     * recommended instead because it works when coroutines are executed
     * manually or through the CoroutineScheduler.
     */
    bool isEnding() const { return mStatus == kStatusEnding; }

    /**
     * The coroutine was terminated by the scheduler with a call to
     * setTerminated(). In most cases, isDone() should be used instead
     * because it works when coroutines are executed manually or through the
     * CoroutineScheudler.
     */
    bool isTerminated() const { return mStatus == kStatusTerminated; }

    /**
     * The coroutine is either Ending or Terminated. This method is recommended
     * over isEnding() or isTerminated() because it works when the coroutine is
     * executed either manually or through the CoroutineScheduler.
     */
    bool isDone() const {
      return mStatus == kStatusEnding || mStatus == kStatusTerminated;
    }

    /**
     * Deprecated method that does nothing. Starting v1.3, the setup into the
     * singly-linked list is automatically performed by the constructor and
     * this method no longer needs to be called manually. This method is
     * retained for backwards compatibility.
     */
    void setupCoroutine(const char* /*name*/) ACE_ROUTINE_DEPRECATED {}

    /**
     * Deprecated method that does nothing. Starting v1.3, the setup into the
     * singly-linked list is automatically performed by the constructor and
     * this method no longer needs to be called manually. This method is
     * retained for backwards compatibility.
     */
    void setupCoroutine(const __FlashStringHelper* /*name*/)
        ACE_ROUTINE_DEPRECATED {}

    /** Set the profiler. */
    void setProfiler(CoroutineProfiler* profiler) { mProfiler = profiler; }

    /** Get the profiler. Nullable. */
    CoroutineProfiler* getProfiler() const { return mProfiler; }

    /**
     * Get the pointer to the root pointer. Implemented as a function static to
     * fix the C++ static initialization problem, making it safe to use this in
     * other static contexts.
     */
    static CoroutineTemplate** getRoot() {
      // Use a static variable inside a function to solve the static
      // initialization ordering problem.
      static CoroutineTemplate* root;
      return &root;
    }

    /**
     * Return the next pointer as a pointer to the pointer, similar to
     * getRoot(). This makes it much easier to manipulate a singly-linked list.
     * Also makes setNext() method unnecessary. Should be used only by
     * CoroutineScheduler.
     */
    CoroutineTemplate** getNext() { return &mNext; }

  protected:
    /**
     * The execution status of the coroutine, corresponding to the
     * COROUTINE_YIELD(), COROUTINE_DELAY(), COROUTINE_AWAIT() and
     * COROUTINE_END() macros.
     *
     * The finite state diagram looks like this:
     *
     * @verbatim
     *          Suspended
     *          ^       ^
     *         /         \
     *        /           \
     *       v             \
     * Yielding          Delaying
     *      ^               ^
     *       \             /
     *        \           /
     *         \         /
     *          v       v
     *           Running
     *              |
     *              |
     *              v
     *           Ending
     *              |
     *              |
     *              v
     *         Terminated
     * @endverbatim
     */
    typedef uint8_t Status;

    /**
     * Coroutine has been suspended using suspend() and the scheduler should
     * remove it from the queue upon the next iteration. We don't distinguish
     * whether the coroutine is still in the queue or not with this status. We
     * can add that later if we need to.
     */
    static const Status kStatusSuspended = 0;

    /** Coroutine returned using the COROUTINE_YIELD() statement. */
    static const Status kStatusYielding = 1;

    /** Coroutine returned using the COROUTINE_DELAY() statement. */
    static const Status kStatusDelaying = 2;

    /** Coroutine is currenly running. True only within the coroutine itself. */
    static const Status kStatusRunning = 3;

    /** Coroutine executed the COROUTINE_END() statement. */
    static const Status kStatusEnding = 4;

    /** Coroutine has ended and no longer in the scheduler queue. */
    static const Status kStatusTerminated = 5;

    /** Constructor. Automatically insert self into singly-linked list. */
    CoroutineTemplate() {
      insertAtRoot();
    }

    /**
     * Destructor. Non-virtual.
     *
     * A virtual destructor increases the flash memory consumption on 8-bit AVR
     * processors by 500-600 bytes because it pulls in the free() and malloc()
     * functions. On the 32-bit SAMD21, the flash memory increases by by about
     * 350 bytes. On other 32-bit processors (STM32, ESP8266, ESP32, Teensy
     * 3.2), the flash memory increase is modest, about 50-150 bytes.
     *
     * Since a Coroutine is expected to be created statically, instead of the
     * heap, a non-virtual destructor is good enough.
     */
    ~CoroutineTemplate() = default;

    /** Return the status of the coroutine. Used by the CoroutineScheduler. */
    Status getStatus() const { return mStatus; }

    /** Print the human-readable string of the Status. */
    void statusPrintTo(Print& printer) {
      printer.print((const __FlashStringHelper*)
          pgm_read_ptr(&sStatusStrings[mStatus]));
    }

    /**
     * Pointer to label where execute will start on the next call to
     * runCoroutine().
     */
    void setJump(void* jumpPoint) { mJumpPoint = jumpPoint; }

    /**
     * Pointer to label where execute will start on the next call to
     * runCoroutine().
     */
    void* getJump() const { return mJumpPoint; }

    /** Set the kStatusRunning state. */
    void setRunning() { mStatus = kStatusRunning; }

    /** Set the kStatusDelaying state. */
    void setYielding() { mStatus = kStatusYielding; }

    /** Set the kStatusDelaying state. */
    void setDelaying() { mStatus = kStatusDelaying; }

    /** Set the kStatusEnding state. */
    void setEnding() { mStatus = kStatusEnding; }

    /**
     * Set status to indicate that the Coroutine has been removed from the
     * Scheduler queue. Should be used only by the CoroutineScheduler.
     */
    void setTerminated() { mStatus = kStatusTerminated; }

    /**
     * Configure the delay timer for delayMillis.
     *
     * The maximum duration is set to (UINT16_MAX / 2) (i.e. 32767
     * milliseconds) if given a larger value. This makes the longest allowable
     * time between two successive calls to isDelayExpired() for a given
     * coroutine to be 32767 (UINT16_MAX - UINT16_MAX / 2 - 1) milliseconds,
     * which should be long enough for all practical use-cases. (The '- 1'
     * comes from an edge case where isDelayExpired() evaluates to be true in
     * the CoroutineScheduler::runCoroutine() but becomes to be false in the
     * COROUTINE_DELAY() macro inside Coroutine::runCoroutine()) because the
     * clock increments by 1 millisecond.)
     */
    void setDelayMillis(T_DELAY delayMillis) {
      mDelayStart = coroutineMillis();

      // If delayMillis is a compile-time constant, the compiler seems to
      // completely optimize away this bounds checking code.
      mDelayDuration = (delayMillis >= UINT16_MAX / 2)
          ? UINT16_MAX / 2
          : delayMillis;
    }

    /**
     * Configure the delay timer for delayMicros. Similar to seDelayMillis(),
     * the maximum delay is 32767 micros.
     */
    void setDelayMicros(T_DELAY delayMicros) {
      mDelayStart = coroutineMicros();

      // If delayMicros is a compile-time constant, the compiler seems to
      // completely optimize away this bounds checking code.
      mDelayDuration = (delayMicros >= UINT16_MAX / 2)
          ? UINT16_MAX / 2
          : delayMicros;
    }

    /**
     * Configure the delay timer for delaySeconds. Similar to seDelayMillis(),
     * the maximum delay is 32767 seconds.
     */
    void setDelaySeconds(T_DELAY delaySeconds) {
      mDelayStart = coroutineSeconds();

      // If delaySeconds is a compile-time constant, the compiler seems to
      // completely optimize away this bounds checking code.
      mDelayDuration = (delaySeconds >= UINT16_MAX / 2)
          ? UINT16_MAX / 2
          : delaySeconds;
    }

    /**
     * Returns the current millisecond clock. By default it returns the global
     * millis() function from Arduino but can be overridden by providing a
     * different T_CLOCK template parameter.
     */
    static unsigned long coroutineMillis() {
      return T_CLOCK::millis();
    }

    /**
     * Returns the current microseconds clock. By default it returns the global
     * micros() function from Arduino but can be overridden by providing a
     * different T_CLOCK template parameter.
     */
    static unsigned long coroutineMicros() {
      return T_CLOCK::micros();
    }

    /**
     * Returns the current clock in unit of seconds, truncated to the lower
     * 16-bits. This is an approximation of (millis / 1000). It does not need
     * to be perfectly accurate because COROUTINE_DELAY_SECONDS() is not
     * guaranteed to be precise.
     */
    static unsigned long coroutineSeconds() {
      return T_CLOCK::seconds();
    }

  private:
    // Disable copy-constructor and assignment operator
    CoroutineTemplate(const CoroutineTemplate&) = delete;
    CoroutineTemplate& operator=(const CoroutineTemplate&) = delete;

    /**
     * Insert the current coroutine at the root of the singly linked list. This
     * is the most efficient and becomes the default with v1.2 because the
     * ordering of the coroutines in the CoroutineScheduler is no longer an
     * externally defined property.
     */
    void insertAtRoot() {
      CoroutineTemplate** root = getRoot();
      mNext = *root;
      *root = this;
    }

  protected:
    /** Pointer to the next coroutine in a singly-linked list. */
    CoroutineTemplate* mNext = nullptr;

    /** Address of the label used by the computed-goto. */
    void* mJumpPoint = nullptr;

    /**
     * Name of the coroutine, either (const char*) or (const
     * __FlashStringHelper*). (Optional)
     */
    const void* mName = nullptr;

    /** String type of the coroutine mName. */
    uint8_t mNameType = kNameTypeCString;

    /** Run-state of the coroutine. */
    Status mStatus = kStatusYielding;

    /**
     * Start time provided by COROUTINE_DELAY(), COROUTINE_DELAY_MICROS(), or
     * COROUTINE_DELAY_SECONDS(). The unit of this number is context dependent,
     * milliseconds, microseconds, or seconds.
     */
    T_DELAY mDelayStart;

    /**
     * Delay time specified by COROUTINE_DELAY(), COROUTINE_DELAY_MICROS() or,
     * COROUTINE_DELAY_SECONDS(). The unit of this number is context dependent,
     * milliseconds, microseconds, or seconds.
     */
    T_DELAY mDelayDuration;

    /** Pointer to a profiler instance, either static or on the heap. */
    CoroutineProfiler* mProfiler = nullptr;
};

/**
 * A concrete template instance of CoroutineTemplate that uses ClockInterface
 * which uses the built-in millis() or micros() function. This becomes the base
 * class of all user-defined coroutines created using the COROUTINE() macro or
 * through manual subclassing of this class.
 */
using Coroutine = CoroutineTemplate<ClockInterface, uint16_t>;

}

#endif
