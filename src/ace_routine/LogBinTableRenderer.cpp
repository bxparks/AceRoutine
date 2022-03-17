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
#include "LogBinTableRenderer.h"

namespace ace_routine {
namespace internal {

// The numbers for higher duration values deviate from powers of 2 because 2^10
// = 1024, which is not exactly 1000. The length of each string *must* be 6 or
// smaller, otherwise a bit of code in LogBinTableRenderer.h must be updated.
static const char kBinLabel00[] PROGMEM = "<2us";
static const char kBinLabel01[] PROGMEM = "<4us";
static const char kBinLabel02[] PROGMEM = "<8us";
static const char kBinLabel03[] PROGMEM = "<16us";
static const char kBinLabel04[] PROGMEM = "<32us";
static const char kBinLabel05[] PROGMEM = "<64us";
static const char kBinLabel06[] PROGMEM = "<128us";
static const char kBinLabel07[] PROGMEM = "<256us";
static const char kBinLabel08[] PROGMEM = "<512us";
static const char kBinLabel09[] PROGMEM = "<1ms";
static const char kBinLabel10[] PROGMEM = "<2ms";
static const char kBinLabel11[] PROGMEM = "<4ms";
static const char kBinLabel12[] PROGMEM = "<8ms";
static const char kBinLabel13[] PROGMEM = "<16ms";
static const char kBinLabel14[] PROGMEM = "<33ms";
static const char kBinLabel15[] PROGMEM = "<66ms";
static const char kBinLabel16[] PROGMEM = "<131ms";
static const char kBinLabel17[] PROGMEM = "<262ms";
static const char kBinLabel18[] PROGMEM = "<524ms";
static const char kBinLabel19[] PROGMEM = "<1s";
static const char kBinLabel20[] PROGMEM = "<2s";
static const char kBinLabel21[] PROGMEM = "<4s";
static const char kBinLabel22[] PROGMEM = "<8s";
static const char kBinLabel23[] PROGMEM = "<17s";
static const char kBinLabel24[] PROGMEM = "<34s";
static const char kBinLabel25[] PROGMEM = "<67s";
static const char kBinLabel26[] PROGMEM = "<134s";
static const char kBinLabel27[] PROGMEM = "<268s";
static const char kBinLabel28[] PROGMEM = "<537s";
static const char kBinLabel29[] PROGMEM = "<1074s";
static const char kBinLabel30[] PROGMEM = "<2147s";
static const char kBinLabel31[] PROGMEM = "<4295s";

const char* const kBinLabels[kNumBinLabels] PROGMEM = {
  kBinLabel00,
  kBinLabel01,
  kBinLabel02,
  kBinLabel03,
  kBinLabel04,
  kBinLabel05,
  kBinLabel06,
  kBinLabel07,
  kBinLabel08,
  kBinLabel09,
  kBinLabel10,
  kBinLabel11,
  kBinLabel12,
  kBinLabel13,
  kBinLabel14,
  kBinLabel15,
  kBinLabel16,
  kBinLabel17,
  kBinLabel18,
  kBinLabel19,
  kBinLabel20,
  kBinLabel21,
  kBinLabel22,
  kBinLabel23,
  kBinLabel24,
  kBinLabel25,
  kBinLabel26,
  kBinLabel27,
  kBinLabel28,
  kBinLabel29,
  kBinLabel30,
  kBinLabel31,
};

}
}
