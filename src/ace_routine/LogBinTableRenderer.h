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

#ifndef ACE_ROUTINE_LOG_BIN_TABLE_RENDERER_H
#define ACE_ROUTINE_LOG_BIN_TABLE_RENDERER_H

#include <stdint.h> // uint8_t, uint32_t
#include <Arduino.h> // Print
#include <AceCommon.h> // printPad5To()
#include "Coroutine.h" // Coroutine
#include "CoroutineProfiler.h"

namespace ace_routine {

/**
 * Print the information in the LogBinProfiler for each Coroutine
 * in a human-readable table. Each bin is printed as a 5-digit number, since the
 * bins use a `uint16_t` integer for the count. The number of digits in the
 * printed number is equivalent to the log10() of the frequency count. To see
 * why that's true, imagine if each digit of the bin count was replaced by a '*'
 * character. Therefore the table is a rough ASCII version of a log-log graph of
 * the frequency count.
 *
 * For example, the output of `printTo(Serial, 2, 10)` for
 * `examples/SoundManager` looks like this:
 *
 * @verbatim
 * name          <8us <16us <32us <64us<128us<256us<512us  <1ms  <2ms    >>
 * soundManager  1418     0     0     1     0     0     0     0     1     0
 * soundRoutine  1417     0     0     1     1     1     0     0     0     0
 * @endverbatim
 *
 * The bins below the startBin are rolled into the first bin. The bins above the
 * (endBin-1) are rolled into the last bin with the label `>>`. There must be at
 * least 2 rendering bins (endBin >= startBin + 2) otherwise nothing is printed.
 *
 * @tparam T_COROUTINE class of the specific CoroutineTemplate instantiation,
 *    usually `Coroutine`
 */
template <typename T_COROUTINE>
class LogBinRendererTemplate {
  public:
    /** Typedef of the LogBinProfiler supported by this class. */
    using Profiler = LogBinProfilerTemplate<T_COROUTINE>;

    /** Constructor. */
    LogBinRendererTemplate(T_COROUTINE** root)
        : mRoot(root)
    {}

    /**
     * Loop over all coroutines and print the ASCII version of the frequency
     * distribution. This assumes that all the coroutines are using the same
     * CoroutineProfiler class, so we can use any of the profilers to print the
     * header, and the numbers will be lined up properly.
     *
     * @param printer destination of output, usually `Serial`
     * @param startBin start index of the bins (0-31)
     * @param endBin end index (exclusive) of the bins (0-32)
     * @param clear call LogBinProfiler::clear() after printing
     *        (default true)
     * @param rollup roll-up exterior bins into the first and last bins
     *        (default true)
     */
    void printTo(
        Print& printer,
        uint8_t startBin,
        uint8_t endBin,
        bool clear = true,
        bool rollup = true
    ) {
      if (endBin <= startBin) return;

      uint16_t bufBins[Profiler::kNumBins];

      bool isHeaderPrinted = false;
      for (Coroutine** p = mRoot; (*p) != nullptr; p = (*p)->getNext()) {
        auto* profiler = (Profiler*) (*p)->getProfiler();
        if (! profiler) continue;

        // Print header if needed.
        if (! isHeaderPrinted) {
          printer.print(F("name        "));
          printHeaderTo(printer, startBin, endBin);
          printer.println();
          isHeaderPrinted = true;
        }

        // Print the coroutine name, truncated to 12 characters. The next column
        // is a 5-digit number formatted into a 6-character block, so there will
        // always be a space between the coroutine name and the next number.
        (*p)->printNameTo(printer, 12);

        // Roll up the exterior bins in to the first and last bins if requested.
        const uint16_t* bins;
        if (rollup) {
          rollupExteriorBins(bufBins, profiler->mBins, Profiler::kNumBins,
              startBin, endBin);
          bins = bufBins;
        } else {
          bins = profiler->mBins;
        }

        // Print the bins. The number of digits is automatically a log10() of
        // the counts, so we should be able to visually scan the table and see
        // which coroutine is taking too long.
        printBinsTo(printer, bins, Profiler::kNumBins, startBin, endBin);
        printer.println();

        if (clear) {
          profiler->clear();
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
    static void printHeaderTo(
        Print& printer, uint8_t startBin, uint8_t endBin) {

      endBin = (endBin > Profiler::kNumBins) ? Profiler::kNumBins : endBin;
      if (endBin <= startBin) return; // needed if startBin = endBin = 0

      for (uint8_t i = startBin; i < endBin - 1; i++) {
        uint8_t labelLength = strlen(kBinLabels[i]);
        for (uint8_t i = 0; i < (6 - labelLength); i++) {
          printer.print(' ');
        }
        printer.print(kBinLabels[i]);
      }
      printer.print(F("    >>"));
    }

    /**
     * Print the bins of this profiler as a single line of text with each bin
     * printed as a 5-digit number in a 6-character box. If there are any
     * remaining counts after the `endBin`, the cummulative sum of the remaining
     * bins are printed in another 6-character box.
     *
     * @param printer usual `Serial`
     * @param bins pointer to array of bins
     * @param startBin start index of the bins (0-31)
     * @param endBin end index (exclusive) of the bins (0-32)
     */
    static void printBinsTo(
        Print& printer,
        const uint16_t bins[],
        uint8_t numBins,
        uint8_t startBin,
        uint8_t endBin) {

      endBin = (endBin > numBins) ? numBins : endBin;
      for (uint8_t i = startBin; i < endBin; i++) {
        uint16_t count = bins[i];
        printer.print(' ');
        ace_common::printPad5To(printer, count);
      }
    }

  private:
    /** Labels for each bin in LogBinProfiler::mBins. */
    static const char* const kBinLabels[Profiler::kNumBins];

    T_COROUTINE** mRoot;
};

template <typename T_COROUTINE>
const char* const LogBinRendererTemplate<T_COROUTINE>::kBinLabels[
    LogBinProfilerTemplate<T_COROUTINE>::kNumBins] = {
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

using LogBinTableRenderer = LogBinRendererTemplate<Coroutine>;

}

#endif
