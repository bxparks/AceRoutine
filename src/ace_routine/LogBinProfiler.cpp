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

#include "LogBinProfiler.h"

namespace ace_routine {
namespace internal {

void rollupExteriorBins(
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
}
