/*
MIT License

Copyright (c) 2021 Brian T. Park

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

#ifndef ACE_ROUTINE_CLOCK_INTERFACE_H
#define ACE_ROUTINE_CLOCK_INTERFACE_H

#include <stdint.h>
#include <Arduino.h>

namespace ace_routine {

/**
 * A utility class (all methods are static) that provides a layer of indirection
 * to Arduino clock functions (millis() and micros()). This thin layer of
 * indirection allows injection of a different ClockInterface for testing
 * purposes. For normal code, the compiler will optimize away the extra function
 * call.
 */
class ClockInterface {
  public:
    /** Get the current millis. */
    static unsigned long millis() { return ::millis(); }

    /** Get the current micros. */
    static unsigned long micros() { return ::micros(); }

    /**
     * Get the current seconds. This is derived by dividing millis() by 1000,
     * which works pretty well until the `unsigned long` rolls over at
     * 4294967296 milliseconds. At that last second (4294967), this function
     * returns the next second (0) a little bit too early. More precisely, it
     * rolls over to 0 seconds 704 milliseconds too early. If the
     * COROUTINE_DELAY_SECONDS() is large enough, this inaccuracy should not
     * matter too much.
     *
     * The other problem with this function is that on 8-bit processors without
     * a hardware division instruction, the software long division by 1000 is
     * very expensive in both memory and CPU. The flash memory increases by
     * about 150 bytes on AVR processors. Therefore, the
     * COROUTINE_DELAY_SECONDS() should be used sparingly.
     */
    static unsigned long seconds() { return ::millis() / 1000; }
};

}

#endif
