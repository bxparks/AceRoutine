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

// Much of this copied from AUnit/src/aunit/FCString.h and
// AUnit/src/aunit/Compare.h.

#include <stdint.h> // uintptr_t
#include <string.h> // strcmp()
#include <Print.h> // Print
#include "Flash.h"
#include "FCString.h"

namespace ace_routine {

size_t FCString::printTo(Print& printer) const {
  if (mString.cstring == nullptr) {
    return printer.print((uintptr_t) this);
  }

  if (mStringType == kCStringType) {
    return printer.print(getCString());
  } else {
    return printer.print(getFString());
  }
}

namespace internal {

int compareString(const char* a, const char* b) {
  if (a == b) { return 0; }
  if (a == nullptr) { return -1; }
  if (b == nullptr) { return 1; }
  return strcmp(a, b);
}

int compareString(const char* a, const __FlashStringHelper* b) {
  if (a == (const char*) b) { return 0; }
  if (a == nullptr) { return -1; }
  if (b == nullptr) { return 1; }
  return strcmp_P(a, (const char*) b);
}

int compareString(const __FlashStringHelper* a, const char* b) {
  return -compareString(b, a);
}

int compareString(const __FlashStringHelper* a, const __FlashStringHelper* b) {
  if (a == b) { return 0; }
  if (a == nullptr) { return -1; }
  if (b == nullptr) { return 1; }
  const char* aa = reinterpret_cast<const char*>(a);
  const char* bb = reinterpret_cast<const char*>(b);

  while (true) {
    uint8_t ca = pgm_read_byte(aa);
    uint8_t cb = pgm_read_byte(bb);
    if (ca != cb) return (int) ca - (int) cb;
    if (ca == '\0') return 0;
    aa++;
    bb++;
  }
}

}

int FCString::compareTo(const FCString& that) const {
  if (getType() == FCString::kCStringType) {
      if (that.getType() == FCString::kCStringType) {
        return internal::compareString(getCString(), that.getCString());
      } else {
        return internal::compareString(getCString(), that.getFString());
      }
  } else {
      if (that.getType() == FCString::kCStringType) {
        return internal::compareString(getFString(), that.getCString());
      } else {
        return internal::compareString(getFString(), that.getFString());
      }
  }
}

}
