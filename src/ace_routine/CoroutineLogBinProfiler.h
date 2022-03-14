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

#ifndef ACE_ROUTINE_COROUTINE_LOG_BIN_PROFILER_H
#define ACE_ROUTINE_COROUTINE_LOG_BIN_PROFILER_H

#include <stdint.h> // uint8_t, uint32_t
#include "Coroutine.h"
#include "CoroutineProfiler.h"

class Print;

namespace ace_routine {

/**
 * Class that maintains the frequency count of the elapsed time of
 * runCoroutine() in an array of bins where each bin is a log2() logarithm of
 * the elapsed time in microseconds. An instance of this class can be attached
 * to a given coroutine through the `Coroutine::setProfiler()` method.
 *
 * After sufficient number of samples are collected, the frequency distribution
 * of all profilers for all coroutines can be printed as a table using the
 * `printBinsTo()` static helper function. Each bin is printed as a 5-digit
 * number (since the bins use a `uint16_t` integer for the count). The number of
 * digits in the printed number is equivalent to the log10() of the frequency
 * count. (To see why that's true, imagine if each digit of the bin count was
 * replaced by a '*' character.) Therefore the table is a rough ASCII version of
 * a log-log graph of the frequency count.
 *
 * For example, here is the output of `printBinsTo()` for the sample program in
 * `examples/SoundManager`:
 *
 * @verbatim
 *             <2us  <4us  <8us <16us <32us <64us<128us<256us<512us  <1ms  <2ms
 * soundManag   621  2091     0     0     5     3     0     0     0     0     1
 * soundRouti  2162   553     1     0     1     2     2     1     0     0     0
 *
 * The `init()` method resets the frequency count of the bins.
 *
 * @endverbatim
 */
class CoroutineLogBinProfiler : public CoroutineProfiler {
  public:
    static const uint8_t kNumBins = 32;

  public:
    /** Constructor. */
    CoroutineLogBinProfiler();

    /** Clear the bins. */
    void init();

    void updateElapsedMicros(uint32_t micros) override;

    /**
     * Print the header related to this profiler.
     *
     * @param binStart start index of the bins (0-31)
     * @param binEnd end index (exclusive) of the bins (0-32)
     */
    void printHeaderTo(Print& printer, uint8_t binStart, uint8_t binEnd) const;

    /**
     * Print the bins of this profiler.
     *
     * @param binStart start index of the bins (0-31)
     * @param binEnd end index (exclusive) of the bins (0-32)
     */
    void printTo(Print& printer, uint8_t binStart, uint8_t binEnd) const;

    /**
     * Loop over all coroutines and print the ASCII version of a histogram.
     *
     * @param binStart start index of the bins (0-31)
     * @param binEnd end inex (exclusive) of the bins (0-32)
     */
    static void printBinsTo(
        Print& printer, Coroutine** root, uint8_t binStart, uint8_t binEnd);

  public:
    uint16_t mBins[kNumBins];
};

}

#endif
