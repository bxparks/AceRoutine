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
#include <Arduino.h> // millis()
#include "Routine.h"

namespace ace_routine {

// Use a static variable inside a function to solve the static initialization
// ordering problem.
Routine** Routine::getRoot() {
  static Routine* root;
  return &root;
}

void Routine::insertSorted() {
  Routine** p = getRoot();

  // O(N^2) insertion, good enough for small (O(100)?) number of routines.
  while (*p != nullptr) {
    if (getName().compareTo((*p)->getName()) <= 0) break;
    p = &(*p)->mNext;
  }

  mNext = *p;
  *p = this;
}

void Routine::resume() {
  if (mStatus != kStatusSuspended) return;
  mStatus = kStatusYielding;

  // insert at the head of the linked list
  Routine** p = getRoot();
  mNext = *p;
  *p = this;
}

unsigned long Routine::millis() const {
  return ::millis();
}

}
