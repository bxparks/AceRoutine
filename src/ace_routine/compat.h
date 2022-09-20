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

/**
 * @file compat.h
 *
 * Various macros to smooth over the differences among the various platforms
 * with regards to their support for flash strings and the various macros used
 * to create and access them.
 *
 * Copied from AUnit/src/aunit/Flash.h and
 * AceTime/src/ace_time/common/compat.h.
 *
 * We support flash strings (F() macro) for AVR because those MCUs have very
 * small static RAM (1-2kB). Prior to v1.0, we disabled F() for ESP8266 because
 * those implementations were buggy. But it seems that recent ESP8266 cores
 * (v2.5 and higher) seems to have fixed the problems with F(), so I have
 * reactivated it. The F() is automatically a no-op for Teensy and ESP32.
 *
 * The FPSTR() macro is a useful macro that was originally created on the
 * ESP8266. But it was incorrectly implemented on the ESP32, until
 * https://github.com/espressif/arduino-esp32/issues/1371 is fixed (hopefully
 * by v1.0.3).
 */

#ifndef ACE_ROUTINE_FLASH_H
#define ACE_ROUTINE_FLASH_H

class __FlashStringHelper;

#if defined(ARDUINO_ARCH_AVR)
  #include <avr/pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))

#elif defined(ARDUINO_ARCH_SAMD)
  #include <avr/pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))

#elif defined(ARDUINO_ARCH_STM32)
  #include <avr/pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))

#elif defined(TEENSYDUINO)
  #include <avr/pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))

#elif defined(ESP8266)
  #include <pgmspace.h>

#elif defined(ESP32)
  #include <pgmspace.h>

#elif defined(EPOXY_DUINO)
  #include <pgmspace.h>

#elif defined(ARDUINO_NRF52_ADAFRUIT)
  #include <avr/pgmspace.h>
  #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))

#else
  #warning Untested platform, AceRoutine may still work...

  #include <avr/pgmspace.h>
  #ifndef FPSTR
    /**
     * A macro that converts a `const char*` that already points to a PROGMEM
     * string to a `const __FlashStringHelper*` which can be passed to
     * overloaded functions and methods.
     */
    #define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))
  #endif

#endif

#endif
