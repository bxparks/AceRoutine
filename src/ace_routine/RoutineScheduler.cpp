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

// Set this to 1 to send debugging from RoutineScheduler::runRoutine() to
// Serial. You must run Serial.begin(speed) in setup().
#define ACE_ROUTINE_DEBUG 0

#include "RoutineScheduler.h"

#if ACE_ROUTINE_DEBUG == 1
  #include <Arduino.h> // Serial, Print
#endif

namespace ace_routine {

RoutineScheduler* RoutineScheduler::getScheduler() {
  static RoutineScheduler singletonScheduler;
  return &singletonScheduler;
}

void RoutineScheduler::setupScheduler() {
  mCurrent = Routine::getRoot();

  // Pre-scan to remove any routines whose suspend() was called before the
  // RoutineScheduler::setup(). This makes unit tests easier to write because
  // they can just concentrate on only the active routines. And this is also
  // more intuitive because we don't need to wait a complete run() cycle to
  // remove these from the queue.
  Routine** current = Routine::getRoot();
  while (*current != nullptr) {
    if ((*current)->isSuspended()) {
      *current = *((*current)->getNext());
    } else {
      current = (*current)->getNext();
    }
  }
}

void RoutineScheduler::runRoutine() {
  // If reached the end, start from the beginning again.
  if (*mCurrent == nullptr) {
    mCurrent = Routine::getRoot();
    // Return if the list is empty
    if (*mCurrent == nullptr) {
      return;
    }
  }

#if ACE_ROUTINE_DEBUG == 1
  Serial.print(F("Processing "));
  (*mCurrent)->printeName(&Serial);
  Serial.println();
#endif

  // Handle the routine's dispatch back to the last known internal status.
  switch ((*mCurrent)->getStatus()) {
    case Routine::kStatusYielding:
      (*mCurrent)->run();
      mCurrent = (*mCurrent)->getNext();
      break;
    case Routine::kStatusDelaying: {
      uint16_t elapsedMillis =
          (*mCurrent)->millis() - (*mCurrent)->getDelayStart();
      if (elapsedMillis >= (*mCurrent)->getDelay()) {
        (*mCurrent)->run();
      }
      mCurrent = (*mCurrent)->getNext();
      break;
    }
    case Routine::kStatusEnding:
      // take the routine out of the list, and mark it terminated
      (*mCurrent)->setTerminated();
      *mCurrent = *((*mCurrent)->getNext());
      break;
    case Routine::kStatusSuspended:
      // take the routine out of the list
      *mCurrent = *((*mCurrent)->getNext());
      break;
  }
}

void RoutineScheduler::listRoutines(Print* printer) {
  for (Routine** p = Routine::getRoot(); (*p) != nullptr;
      p = (*p)->getNext()) {
    printer->print(F("Routine "));
    (*p)->printName(printer);
    printer->print(F("; status: "));
    printer->println((*p)->getStatus());
  }
}

}
