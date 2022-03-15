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
 * 2, 10)` for `examples/SoundManager` looks like this:
 *
 * @verbatim
 *               <8us <16us <32us <64us<128us<256us<512us  <1ms  <2ms    >>
 * soundManager  1418     0     0     1     0     0     0     0     1     0
 * soundRoutine  1417     0     0     1     1     1     0     0     0     0
 * @endverbatim
 *
 * The bins below the startBin are rolled into the first bin. The bins above the
 * (endBin-1) are rolled into the last bin with the label `>>`. There must be at
 * least 2 rendering bins (endBin >= startBin + 2) otherwise nothing is printed.
 *
 * The `init()` method resets the frequency count of the bins.
 *
 * @tparam T_COROUTINE class of the specific CoroutineTemplate instantiation,
 *    usually `Coroutine`
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
     * @param startBin start index of the bins (0-31)
     * @param endBin end inex (exclusive) of the bins (0-32)
     * @param clear call init() after printing (default true)
     */
    void printTableTo(
        Print& printer,
        uint8_t startBin,
        uint8_t endBin,
        bool clear = true
    ) {
      if (endBin <= startBin) return;

      bool isHeaderPrinted = false;
      for (Coroutine** p = mRoot; (*p) != nullptr; p = (*p)->getNext()) {
        auto* profiler = (Profiler*) (*p)->getProfiler();
        if (! profiler) continue;

        // Print header if needed.
        if (! isHeaderPrinted) {
          ace_common::printfTo(printer, "%12.12s", "");
          printHeaderTo(printer, startBin, endBin);
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
        printBinsTo(printer, profiler, startBin, endBin);
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
     * @param startBin start index of the bins (0-31)
     * @param endBin end index (exclusive) of the bins (0-32)
     */
    void printHeaderTo(Print& printer, uint8_t startBin, uint8_t endBin) const {
      endBin = (endBin > Profiler::kNumBins) ? Profiler::kNumBins : endBin;
      if (endBin < startBin + 2) return;

      for (uint8_t i = startBin; i < endBin - 1; i++) {
        ace_common::printfTo(printer, "%6.6s", kBinLabels[i]);
      }
      ace_common::printfTo(printer, "%6.6s", ">>");
    }

    /**
     * Print the bins of this profiler as a single line of text with each bin
     * printed as a 5-digit number in a 6-character box. If there are any
     * remaining counts after the `endBin`, the cummulative sum of the remaining
     * bins are printed in another 6-character box.
     *
     * @param startBin start index of the bins (0-31)
     * @param endBin end index (exclusive) of the bins (0-32)
     */
    void printBinsTo(
        Print& printer,
        Profiler* profiler,
        uint8_t startBin,
        uint8_t endBin) const {

      endBin = (endBin > Profiler::kNumBins) ? Profiler::kNumBins : endBin;
      if (endBin < startBin + 2) return;

      // Rollup all bins below startBin into the first bin.
      uint32_t underCount = 0;
      for (uint8_t i = 0; i <= startBin; i++) {
        underCount += profiler->mBins[i];
      }
      if (underCount > UINT16_MAX) underCount = UINT16_MAX;
      ace_common::printfTo(printer, "%6u", underCount);

      // Print interior bins.
      for (uint8_t i = startBin + 1; i < endBin - 1; i++) {
        uint16_t count = profiler->mBins[i];
        ace_common::printfTo(printer, "%6u", count);
      }

      // Rollup all bins at or above endBin into the last bin.
      uint32_t overCount = 0;
      for (uint8_t i = endBin - 1; i < Profiler::kNumBins; i++) {
        overCount += profiler->mBins[i];
      }
      if (overCount > UINT16_MAX) overCount = UINT16_MAX;
      ace_common::printfTo(printer, "%6u", overCount);
    }

  private:
    /** Labels for each bin in CoroutineLogBinProfiler::mBins. */
    static const char* const kBinLabels[Profiler::kNumBins];

    T_COROUTINE** mRoot;
};

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
  "<33ms",
  "<66ms",
  "<131ms",
  "<262ms",
  "<524ms",
  "<1s",
  "<2s",
  "<4s",
  "<8s",
  "<17s",
  "<34s",
  "<67s",
  "<134s",
  "<268s",
  "<537s",
  "<1074s",
  "<2147s",
  "<4295s",
};

using CoroutineLogBinRenderer = CoroutineLogBinRendererTemplate<Coroutine>;

}

#endif
