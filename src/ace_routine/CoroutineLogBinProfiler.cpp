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

#include <Arduino.h>
#include <stdio.h> // memset()
#include <AceCommon.h> // printfTo()
#include "Coroutine.h"
#include "CoroutineLogBinProfiler.h"

using ace_common::printfTo;

namespace ace_routine {

CoroutineLogBinProfiler::CoroutineLogBinProfiler() {
  init();
}

void CoroutineLogBinProfiler::init() {
  memset(mBins, 0, sizeof(mBins));
}

void CoroutineLogBinProfiler::updateElapsedMicros(uint32_t micros) {
  uint8_t index = 0; // [0, 31]

  // The index is the bit number (0-31) of the most significant bit. So each bin
  // contains the number of samples whose elapsed micros is between 2^i and
  // 2^(i+1). Example, for Bin 3, and this bin contains the number of samples
  // which satsify (8us <= elapsed < 16us). The exception is Bin 0 because it
  // includes samples where elapsed = 0 as well, so the sample interval for Bin
  // 0 is (0 <= elapsed < 2), instead of (1 <= elapsed < 2).
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

// Labels for each bin in mBins.
static const char* kBinLabels[CoroutineLogBinProfiler::kNumBins] = {
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

void CoroutineLogBinProfiler::printHeaderTo(
    Print& printer, uint8_t binStart, uint8_t binEnd) const {
  if (binEnd <= binStart) return;

  binEnd = (binEnd > kNumBins) ? kNumBins : binEnd;
  for (uint8_t i = binStart; i < binEnd; i++) {
    ace_common::printfTo(printer, "%6s", kBinLabels[i]);
  }
}

void CoroutineLogBinProfiler::printTo(
    Print& printer, uint8_t binStart, uint8_t binEnd) const {
  if (binEnd <= binStart) return;

  binEnd = (binEnd > kNumBins) ? kNumBins : binEnd;
  for (uint8_t i = binStart; i < binEnd; i++) {
    uint16_t count = mBins[i];
    ace_common::printfTo(printer, "%6d", count);
  }

  uint32_t remaining = 0;
  for (uint8_t i = binEnd; i < kNumBins; i++) {
    remaining += mBins[i];
  }
  if (remaining > 0) {
    ace_common::printfTo(printer, "%6ld", (unsigned long) remaining);
  }
}

void CoroutineLogBinProfiler::printBinsTo(
    Print& printer, Coroutine** root, uint8_t binStart, uint8_t binEnd) {
  if (binEnd <= binStart) return;

  bool isHeaderPrinted = false;
  for (Coroutine** p = root; (*p) != nullptr; p = (*p)->getNext()) {
    CoroutineLogBinProfiler* profiler =
        (CoroutineLogBinProfiler*) (*p)->getProfiler();
    if (! profiler) continue;

    // Print header if needed.
    if (! isHeaderPrinted) {
      printfTo(printer, "%12.12s", "");
      profiler->printHeaderTo(printer, binStart, binEnd);
      printer.println();
      isHeaderPrinted = true;
    }

    // Print the bins. The number of digits is automatically a log10() of the
    // counts, so we should be able to visually scan the table and see which
    // coroutine is taking too long.
    printfTo(printer, "%12.12s", (*p)->getCName());
    profiler->printTo(printer, binStart, binEnd);
    printer.println();

    // TODO: It'd be useful to sum up the remaining buckets after binEnd, and
    // print the number of samples that weren't printed in the loop above.

    // Maybe control this using a boolean flag.
    profiler->init();
  }
}

}
