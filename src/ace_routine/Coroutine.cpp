/*
MIT License

Copyright (c) 2018 Brian T. Park

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

#include <stdint.h> // uintptr_t
#include <Arduino.h> // millis(), micros()
#include <AceCommon.h> // udiv1000()
#include "Coroutine.h"
#include "compat.h" // FPSTR()

namespace ace_routine {

// Use a static variable inside a function to solve the static initialization
// ordering problem.
Coroutine** Coroutine::getRoot() {
  static Coroutine* root;
  return &root;
}

void Coroutine::insertSorted() {
  Coroutine** p = getRoot();

  // O(N^2) insertion, good enough for small (O(100)?) number of coroutines.
  while (*p != nullptr) {
    if (getName().compareTo((*p)->getName()) <= 0) break;
    p = &(*p)->mNext;
  }

  mNext = *p;
  *p = this;
}

void Coroutine::insertAtRoot() {
  Coroutine** root = getRoot();
  mNext = *root;
  *root = this;
}

void Coroutine::resume() {
  if (mStatus != kStatusSuspended) return;

  // We lost the original state of the coroutine when suspend() was called but
  // the coroutine will automatically go back into the original state when
  // Coroutine::runCoroutine() is called because COROUTINE_YIELD(),
  // COROUTINE_DELAY() and COROUTINE_AWAIT() are written to restore their
  // status.
  mStatus = kStatusYielding;
}

unsigned long Coroutine::coroutineMillis() const {
  return ::millis();
}

unsigned long Coroutine::coroutineMicros() const {
  return ::micros();
}

unsigned long Coroutine::coroutineSeconds() const {
  unsigned long m = ::millis();
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD) || defined(ESP8266)
  // No hardware division so the udiv1000() approximation is faster
  return ace_common::udiv1000(m);
#else
  return m / 1000;
#endif
}

// Create the sStatusStrings lookup table to translate Status integer to a
// human-readable string. When it is used, it increases flash memory by 86
// bytes, and static RAM by 14 bytes. It is currently only used by
// CoroutineScheduler::list() but I think it's worth it to make debugging
// easier.

static const char kStatusSuspendedString[] PROGMEM = "Suspended";
static const char kStatusYieldingString[] PROGMEM = "Yielding";
static const char kStatusDelayingString[] PROGMEM = "Delaying";
static const char kStatusRunningString[] PROGMEM = "Running";
static const char kStatusEndingString[] PROGMEM = "Ending";
static const char kStatusTerminatedString[] PROGMEM = "Terminated";

const __FlashStringHelper* const Coroutine::sStatusStrings[] = {
  FPSTR(kStatusSuspendedString),
  FPSTR(kStatusYieldingString),
  FPSTR(kStatusDelayingString),
  FPSTR(kStatusRunningString),
  FPSTR(kStatusEndingString),
  FPSTR(kStatusTerminatedString),
};

}
