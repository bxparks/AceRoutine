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
#include <stdio.h> // memset()
#include "Coroutine.h"
#include "CoroutineProfiler.h"


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
 * @tparam T_COROUTINE class of the specific CoroutineTemplate instantiation,
 *    usually `Coroutine`
 */
template <typename T_COROUTINE>
class CoroutineLogBinProfilerTemplate : public CoroutineProfiler {
  public:
    static const uint8_t kNumBins = 32;

  public:
    /** Constructor. */
    CoroutineLogBinProfilerTemplate() {
      init();
    }

    /** Clear the bins. */
    void init() {
      memset(mBins, 0, sizeof(mBins));
    }

    /**
     * Update the count for the calculated elapsed time bin. The bin index is
     * the bit number (0-31) of the most significant bit. So each bin contains
     * the number of samples whose elapsed micros is between 2^i and 2^(i+1).
     * Example, for Bin 3, and this bin contains the number of samples which
     * satsify (8us <= elapsed < 16us). The exception is Bin 0 because it
     * includes samples where elapsed is 0 as well, so the sample interval for
     * Bin 0 is (0 <= elapsed < 2), instead of (1 <= elapsed < 2).
     */
    void updateElapsedMicros(uint32_t micros) override {
      uint8_t index = 0; // [0, 31]

      while (true) {
        micros >>= 1;
        if (micros == 0) break;
        index++;
      }
      uint16_t count = mBins[index];
      if (count < UINT16_MAX) {
        mBins[index]++;
      }
    }

    /**
     * Create a profiler on the heap for each coroutine in the singly-linked
     * list of coroutines defined by `root`.
     */
    static void createProfilers(T_COROUTINE** root) {
      for (Coroutine** p = root; (*p) != nullptr; p = (*p)->getNext()) {
        auto* profiler = new CoroutineLogBinProfilerTemplate();
        (*p)->setProfiler(profiler);
      }
    }

    /** Delete the profilers created by createProfilers(). */
    static void deleteProfilers(T_COROUTINE** root) {
      for (Coroutine** p = root; (*p) != nullptr; p = (*p)->getNext()) {
        auto* profiler = (CoroutineLogBinProfilerTemplate*) (*p)->getProfiler();
        if (profiler) {
          delete profiler;
          (*p)->setProfiler(nullptr);
        }
      }
    }

    /** Init all profilers for all coroutines defined by `root`. */
    static void initProfilers(T_COROUTINE** root) {
      for (Coroutine** p = root; (*p) != nullptr; p = (*p)->getNext()) {
        auto* profiler = (CoroutineLogBinProfilerTemplate*) (*p)->getProfiler();
        if (profiler) {
          profiler->init();
        }
      }
    }

  public:
    uint16_t mBins[kNumBins];
};

using CoroutineLogBinProfiler = CoroutineLogBinProfilerTemplate<Coroutine>;

}

#endif
