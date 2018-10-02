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

// Set this to 1 to send debugging from CoroutineScheduler::runCoroutine() to
// Serial. You must run Serial.begin(speed) in setup().
#define ACE_ROUTINE_DEBUG 0

#include "CoroutineScheduler.h"

#if ACE_ROUTINE_DEBUG == 1
  #include <Arduino.h> // Serial, Print
#endif

namespace ace_routine {

CoroutineScheduler* CoroutineScheduler::getScheduler() {
  static CoroutineScheduler singletonScheduler;
  return &singletonScheduler;
}

void CoroutineScheduler::setupScheduler() {
  mCurrent = Coroutine::getRoot();

  // Pre-scan to remove any coroutines whose suspend() was called before the
  // CoroutineScheduler::setup(). This makes unit tests easier to write because
  // they can just concentrate on only the active coroutines. And this is also
  // more intuitive because we don't need to wait a complete runCoroutine()
  // cycle to remove these from the queue.
  Coroutine** current = Coroutine::getRoot();
  while (*current != nullptr) {
    if ((*current)->isSuspended()) {
      *current = *((*current)->getNext());
    } else {
      current = (*current)->getNext();
    }
  }
}

void CoroutineScheduler::runCoroutine() {
  // If reached the end, start from the beginning again.
  if (*mCurrent == nullptr) {
    mCurrent = Coroutine::getRoot();
    // Return if the list is empty. Checking for a null getRoot() inside the
    // if-statement is deliberate, since it optimizes the common case where the
    // linked list is not empty.
    if (*mCurrent == nullptr) {
      return;
    }
  }

#if ACE_ROUTINE_DEBUG == 1
  Serial.print(F("Processing "));
  (*mCurrent)->printeName(Serial);
  Serial.println();
#endif

  // Handle the coroutine's dispatch back to the last known internal status.
  switch ((*mCurrent)->getStatus()) {
    case Coroutine::kStatusYielding:
      (*mCurrent)->runCoroutine();
      mCurrent = (*mCurrent)->getNext();
      break;
    case Coroutine::kStatusDelaying: {
      // Check isDelayExpired() here to optimize away an extra call into the
      // Coroutine::runCoroutine(). Everything would still work if we just
      // dispatched into the Coroutine::runCoroutine() because that method
      // checks isDelayExpired() as well.
      if ((*mCurrent)->isDelayExpired()) {
        (*mCurrent)->runCoroutine();
      }
      mCurrent = (*mCurrent)->getNext();
      break;
    }
    case Coroutine::kStatusEnding:
      // take the coroutine out of the list, and mark it terminated
      (*mCurrent)->setTerminated();
      *mCurrent = *((*mCurrent)->getNext());
      break;
    case Coroutine::kStatusSuspended:
      // take the coroutine out of the list
      *mCurrent = *((*mCurrent)->getNext());
      break;
    default:
      // Should never happen but skip to next coroutine to prevent infinite
      // loop.
      mCurrent = (*mCurrent)->getNext();
      break;
  }
}

void CoroutineScheduler::listCoroutines(Print& printer) {
  for (Coroutine** p = Coroutine::getRoot(); (*p) != nullptr;
      p = (*p)->getNext()) {
    printer.print(F("Coroutine "));
    (*p)->getName().printTo(printer);
    printer.print(F("; status: "));
    (*p)->statusPrintTo(printer);
    printer.println();
  }
}

}
