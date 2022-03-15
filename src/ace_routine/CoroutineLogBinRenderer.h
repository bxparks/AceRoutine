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

#ifndef ACE_ROUTINE_COROUTINE_LOG_BIN_RENDERER_H
#define ACE_ROUTINE_COROUTINE_LOG_BIN_RENDERER_H

#include <stdint.h> // uint8_t, uint32_t
#include <Arduino.h> // Print
#include <AceCommon.h> // printfTo()
#include "Coroutine.h" // Coroutine
#include "CoroutineProfiler.h"

namespace ace_routine {

/**
 * Print the information in the CoroutineLogBinProfiler for each Coroutine
 * in a human-readable table. For example, the output of `printTableTo(Serial,
 * 0, 9)` for `examples/SoundManager` looks like this:
 *
 * @verbatim
 *               <2us  <4us  <8us <16us <32us <64us<128us<256us<512us
 * soundManager   621  2091     0     0     5     3     0     0     0     1
 * soundRoutine  2162   553     1     0     1     2     2     1     0
 *
 * The `init()` method resets the frequency count of the bins.
 *
 * @endverbatim

 */
template <typename T_COROUTINE>
class CoroutineLogBinRendererTemplate {
  public:
    /** Typedef of the CoroutineLogBinProfiler supported by this class. */
    using Profiler = CoroutineLogBinProfilerTemplate<T_COROUTINE>;

    /** Constructor. */
    CoroutineLogBinRendererTemplate(T_COROUTINE** root)
        : mRoot(root)
    {}

    /**
     * Loop over all coroutines and print the ASCII version of a histogram. This
     * assumes that all the coroutines are using the same CoroutineProfiler
     * class, so we can use any of the profilers to print the header, and the
     * numbers will be lined up properly.
     *
     * @param printer destination of output, usually `Serial`
     * @param binStart start index of the bins (0-31)
     * @param binEnd end inex (exclusive) of the bins (0-32)
     * @param clear call init() after printing (default true)
     */
    void printTableTo(
        Print& printer,
        uint8_t binStart,
        uint8_t binEnd,
        bool clear = true
    ) {
      if (binEnd <= binStart) return;

      bool isHeaderPrinted = false;
      for (Coroutine** p = mRoot; (*p) != nullptr; p = (*p)->getNext()) {
        auto* profiler = (Profiler*) (*p)->getProfiler();
        if (! profiler) continue;

        // Print header if needed.
        if (! isHeaderPrinted) {
          ace_common::printfTo(printer, "%12.12s", "");
          printHeaderTo(printer, binStart, binEnd);
          printer.println();
          isHeaderPrinted = true;
        }

        // Print the coroutine name, truncated to 12 characters. The next column
        // is a 5-digit number formatted into a 6-character block, so there will
        // always be a space between the coroutine name and the next number.
        (*p)->printNameTo(printer, 12);

        // Print the bins. The number of digits is automatically a log10() of
        // the counts, so we should be able to visually scan the table and see
        // which coroutine is taking too long.
        printBinsTo(printer, profiler, binStart, binEnd);
        printer.println();

        if (clear) {
          profiler->init();
        }
      }
    }

  private:
    /**
     * Print the header related to this profiler.
     *
     * @param printer destination of output
     * @param binStart start index of the bins (0-31)
     * @param binEnd end index (exclusive) of the bins (0-32)
     */
    void printHeaderTo(Print& printer, uint8_t binStart, uint8_t binEnd) const {
      if (binEnd <= binStart) return;

      binEnd = (binEnd > Profiler::kNumBins) ? Profiler::kNumBins : binEnd;
      for (uint8_t i = binStart; i < binEnd; i++) {
        ace_common::printfTo(printer, "%6.6s", kBinLabels[i]);
      }
    }

    /**
     * Print the bins of this profiler as a single line of text with each bin
     * printed as a 5-digit number in a 6-character box. If there are any
     * remaining counts after the `binEnd`, the cummulative sum of the remaining
     * bins are printed in another 6-character box.
     *
     * @param binStart start index of the bins (0-31)
     * @param binEnd end index (exclusive) of the bins (0-32)
     */
    void printBinsTo(
        Print& printer,
        Profiler* profiler,
        uint8_t binStart,
        uint8_t binEnd) const {
      if (binEnd <= binStart) return;

      binEnd = (binEnd > Profiler::kNumBins) ? Profiler::kNumBins : binEnd;
      for (uint8_t i = binStart; i < binEnd; i++) {
        uint16_t count = profiler->mBins[i];
        ace_common::printfTo(printer, "%6u", count);
      }

      uint32_t remaining = 0;
      for (uint8_t i = binEnd; i < Profiler::kNumBins; i++) {
        remaining += profiler->mBins[i];
      }
      if (remaining > 0) {
        ace_common::printfTo(printer, "%6lu", (unsigned long) remaining);
      }
    }

  private:
    static const char* const kBinLabels[Profiler::kNumBins];

    T_COROUTINE** mRoot;
};

// Labels for each bin in mBins.
template <typename T_COROUTINE>
const char* const CoroutineLogBinRendererTemplate<T_COROUTINE>::kBinLabels[
    CoroutineLogBinProfilerTemplate<T_COROUTINE>::kNumBins] = {
  "<2us",
  "<4us",
  "<8us",
  "<16us",
  "<32us",
  "<64us",
  "<128us",
  "<256us",
  "<512us",
  "<1ms",
  "<2ms",
  "<4ms",
  "<8ms",
  "<16ms",
  "<32ms",
  "<64ms",
  "<128ms",
  "<256ms",
  "<512ms",
  "<1s",
  "<2s",
  "<4s",
  "<8s",
  "<16s",
  "<32s",
  "<64s",
  "<128s",
  "<256s",
  "<512s",
  "<1024s",
  "<2048s",
  "<4096s",
};

using CoroutineLogBinRenderer = CoroutineLogBinRendererTemplate<Coroutine>;

}

#endif
