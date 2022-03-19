/*
MIT License

Copyright (c) 2022 Brian T. Park

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

#ifndef ACE_ROUTINE_COROUTINE_PROFILER_H
#define ACE_ROUTINE_COROUTINE_PROFILER_H

#include <stdint.h> // uint32_t

namespace ace_routine {

/**
 * An interface class for profiling classes that can track the elapsed time
 * consumed by `Coroutine::runCoroutine()`.
 */
class CoroutineProfiler {
  public:
    /** Use default constructor. */
    CoroutineProfiler() = default;

    /**
     * The destructor is NON-virtual on AVR processors because adding a virtual
     * destructor causes flash consumption to increase by 600 bytes, even if the
     * profiler is never used. If profiling is enabled on 8-bit processors, I
     * expect the number of coroutines in the system to be relatively small, so
     * it seems reasonable to create instances of the CoroutineProfiler
     * statically at the global scope, and attach them to the Coroutines using
     * the `Coroutine::setProfiler()` method. This means that this destructor
     * will never be called polymorphically.
     *
     * On 32-bit processors, the destructor is virtual because I expect that the
     * application will use a larger number of coroutines, so it will be much
     * easier to use to the `LogBinProfiler::createProfilers()` and
     * `LogBinProfiler::deleteProfilers()` helper methods to create the
     * profilers on the heap. This means that the destructor may be called
     * polymorphically and a virtual destructor is needed for correctness.
     * It is expected that 32-bit processors will have far more flash memory and
     * static RAM than 8-bit processors, so the additional resource consumption
     * will hopefully not be too burdensome. If it is, then 32-bit processors
     * have the option of using the same technique as 8-bit processors, and use
     * the `Coroutine::setProfiler()` to attach statically created instances
     * instead.
     */
  #if defined(ARDUINO_ARCH_AVR)
    ~CoroutineProfiler() = default;
  #else
    virtual ~CoroutineProfiler() = default;
  #endif

    /**
     * Process the completion of the runCoroutine() method which took
     * `micros` microseconds.
     */
    virtual void updateElapsedMicros(uint32_t micros) = 0;
};

}

#endif
