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

#ifndef ACE_ROUTINE_LOG_BIN_JSON_RENDERER_H
#define ACE_ROUTINE_LOG_BIN_JSON_RENDERER_H

#include <stdint.h> // uint8_t, uint32_t
#include <Arduino.h> // Print
#include "Coroutine.h" // Coroutine
#include "CoroutineProfiler.h"
#include "LogBinProfiler.h" // rollupExteriorBins()

namespace ace_routine {

/**
 * Print the `LogBinProfiler` bins as a JSON array. For example, here
 * is the output from `examples/SoundManager`:
 *
 * @verbatim
 * {
 * "soundManager":[1411,0,2,0,0,0,0,0,0,0],
 * "soundRoutine":[1411,0,1,1,0,0,0,0,0,0]
 * }
 * @endverbatim
 */
template <typename T_COROUTINE>
class LogBinJsonRendererTemplate {
  public:
    /** Typedef of the LogBinProfiler supported by this class. */
    using Profiler = LogBinProfilerTemplate<T_COROUTINE>;

    /**
     * Loop over all coroutines and print the bin counts as JSON.
     *
     * @param printer destination of output, usually `Serial`
     * @param startBin start index of the bins (0-31)
     * @param endBin end index (exclusive) of the bins (0-32)
     * @param clear call LogBinProfiler::clear() after printing
     *        (default true)
     * @param rollup roll-up exterior bins into the first and last bins
     *        (default true)
     */
    static void printTo(
        Print& printer,
        uint8_t startBin,
        uint8_t endBin,
        bool clear = true,
        bool rollup = true
    ) {
      uint16_t bufBins[Profiler::kNumBins];

      printer.println('{');
      bool lineNeedsTrailingComma = false;
      T_COROUTINE** root = T_COROUTINE::getRoot();
      for (T_COROUTINE** p = root; (*p) != nullptr; p = (*p)->getNext()) {
        auto* profiler = (Profiler*) (*p)->getProfiler();
        if (! profiler) continue;

        // Roll up the exterior bins in to the first and last bins if requested.
        const uint16_t* bins;
        if (rollup) {
          internal::rollupExteriorBins(
              bufBins, profiler->mBins, Profiler::kNumBins, startBin, endBin);
          bins = bufBins;
        } else {
          bins = profiler->mBins;
        }

        if (lineNeedsTrailingComma) printer.println(',');
        lineNeedsTrailingComma = true;

        printer.print('"');
        (*p)->printNameTo(printer);
        printer.print("\":[");

        bool elementNeedsTrailingComma = false;
        for (uint8_t i = startBin; i < endBin; i++) {
          if (elementNeedsTrailingComma) printer.print(',');
          elementNeedsTrailingComma = true;
          printer.print(bins[i]);
        }
        printer.print(']');

        if (clear) {
          profiler->clear();
        }
      }
      printer.println();
      printer.println('}');
    }
};

using LogBinJsonRenderer = LogBinJsonRendererTemplate<Coroutine>;

}

#endif
