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
 * @file Flash.h
 *
 * Various macros to smooth over the differences among the various platforms
 * with regards to their support for flash strings and the various macros used
 * to create and access them.
 *
 * Copied from AUnit/src/aunit/Flash.h, see that file for more info.
 *
 * In summary, we support flash strings for AVR because those MCUs have very
 * small static RAM (1-2kB), but we disable flash strings for Teensy, ESP8266,
 * and ESP32 because those implementations tend to be buggy or tricky, and
 * they have far more static RAM.
 */

#ifndef ACE_ROUTINE_FLASH_H
#define ACE_ROUTINE_FLASH_H

class __FlashStringHelper;

/**
 * The FPSTR() macro is defined on ESP8266, not defined on Teensy and AVR, and
 * broken on ESP32. We define our own version to make this work on all 4
 * platforms. We might be able to use just FPSTR() if
 * https://github.com/espressif/arduino-esp32/issues/1371 is fixed.
 */
#define ACE_ROUTINE_FPSTR(pstr_pointer) \
    (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))

#if defined(__AVR__) || defined(__arm__)
  #include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
  #include <pgmspace.h>
#else
  #error Unsupported platform
#endif

#if defined(__AVR__)
  #define ACE_ROUTINE_F(x) F(x)
#elif defined(ESP8266) || defined(ESP32) || defined(__arm__)
  #define ACE_ROUTINE_F(x) ACE_ROUTINE_FPSTR(x)
#else
  #error Unsupported platform
#endif

#endif
