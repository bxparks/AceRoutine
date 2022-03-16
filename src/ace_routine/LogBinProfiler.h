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

#ifndef ACE_ROUTINE_LOG_BIN_PROFILER_H
#define ACE_ROUTINE_LOG_BIN_PROFILER_H

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
 * of all profilers for all coroutines can be printed by a renderer. An
 * implementation of such a render is `LogBinRendererTemplate`
 * which prints the frequency distribution as a formatted table.
 *
 * @tparam T_COROUTINE class of the specific CoroutineTemplate instantiation,
 *    usually `Coroutine`
 */
template <typename T_COROUTINE>
class LogBinProfilerTemplate : public CoroutineProfiler {
  public:
    static const uint8_t kNumBins = 32;

  public:
    /** Constructor. */
    LogBinProfilerTemplate() {
      clear();
    }

    /** Clear the bins. */
    void clear() {
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
     * Create a new profiler on the heap and attach it to each coroutine in the
     * singly-linked list of coroutines defined by `root`.
     */
    static void createProfilers(T_COROUTINE** root) {
      for (Coroutine** p = root; (*p) != nullptr; p = (*p)->getNext()) {

        // Delete any existing profiler.
        auto* currentProfiler =
            (LogBinProfilerTemplate*) (*p)->getProfiler();
        if (currentProfiler) {
          delete currentProfiler;
        }

        // Attach new profiler.
        auto* profiler = new LogBinProfilerTemplate();
        (*p)->setProfiler(profiler);
      }
    }

    /** Delete the profilers created by createProfilers(). */
    static void deleteProfilers(T_COROUTINE** root) {
      for (Coroutine** p = root; (*p) != nullptr; p = (*p)->getNext()) {
        auto* profiler = (LogBinProfilerTemplate*) (*p)->getProfiler();
        if (profiler) {
          delete profiler;
          (*p)->setProfiler(nullptr);
        }
      }
    }

    /** Clear counters for all profilers in the coroutines defined by `root`. */
    static void clearProfilers(T_COROUTINE** root) {
      for (Coroutine** p = root; (*p) != nullptr; p = (*p)->getNext()) {
        auto* profiler = (LogBinProfilerTemplate*) (*p)->getProfiler();
        if (profiler) {
          profiler->clear();
        }
      }
    }

  public:
    uint16_t mBins[kNumBins];
};

using LogBinProfiler = LogBinProfilerTemplate<Coroutine>;

/**
  * Rollup bins before `startBin` into `startBin` and bins at or after
  * `endBin` into the last bin (at `endBin - 1`). This is useful to preserve
  * count information when printing only a subset of the `mBins[]` array. If
  * `endBin <= startBin`, this function does nothing. If `endBin < startBin -
  * 1`, there is only single interior bin, so everything gets rolled up into
  * the single bin. This is probably not useful, but at least it's
  * mathematically correct.
  */
inline void rollupExteriorBins(
    uint16_t dst[],
    const uint16_t src[],
    uint8_t numBins,
    uint8_t startBin,
    uint8_t endBin
) {
  endBin = (endBin > numBins) ? numBins : endBin;
  if (endBin <= startBin) return;

  // Rollup all bins at or below startBin into the startBin.
  uint32_t leftRollup = 0;
  for (uint8_t i = 0; i <= startBin; i++) {
    leftRollup += src[i];
  }
  if (leftRollup > UINT16_MAX) leftRollup = UINT16_MAX;
  dst[startBin] = leftRollup;

  // Copy the interior bins.
  for (uint8_t i = startBin + 1; i < endBin - 1; i++) {
    dst[i] = src[i];
  }

  // Rollup all bins at or above the last bin into the last bin.
  uint32_t rightRollup = (endBin - 1 == startBin) ? leftRollup : 0;
  for (uint8_t i = endBin - 1; i < numBins; i++) {
    rightRollup += src[i];
  }
  if (rightRollup > UINT16_MAX) rightRollup = UINT16_MAX;
  dst[endBin - 1] = rightRollup;
}

}

#endif
