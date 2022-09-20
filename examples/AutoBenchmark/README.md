# AutoBenchmark

The `AutoBenchmark` tries to measure the amount of overhead caused by the
context switching between coroutines. It uses 2 alternating coroutines to
increment a global counter for X number of seconds. Each time the counter is
incremented, there is a `Coroutine` context switch. The amount of microseconds
it takes to increment the counter by 1 is given in the `AceRoutine` column.

It then uses does the same thing using just a simple while-loop, which provides
a baseline. This is represented by the `base` column.

The difference between the 2 benchmarks (represented by the `diff` column below)
is the overhead caused by the `Coroutine` context switch.

All times in below are in microseconds.

**DO NOT EDIT**: This file was auto-generated using `make README.md`.

**Version**: AceRoutine v1.5.1

## Dependencies

This program depends on the following libraries:

* [AceRoutine](https://github.com/bxparks/AceRoutine)
* [AceCommon](https://github.com/bxparks/AceCommon)

## How to Generate

This requires the [AUniter](https://github.com/bxparks/AUniter) script
to execute the Arduino IDE programmatically.

The `Makefile` has rules to generate the `*.txt` results file for several
microcontrollers that I usually support, but the `$ make benchmarks` command
does not work very well because the USB port of the microcontroller is a
dynamically changing parameter. I created a semi-automated way of collect the
`*.txt` files:

1. Connect the microcontroller to the serial port. I usually do this through a
USB hub with individually controlled switch.
2. Type `$ auniter ports` to determine its `/dev/ttyXXX` port number (e.g.
`/dev/ttyUSB0` or `/dev/ttyACM0`).
3. If the port is `USB0` or `ACM0`, type `$ make nano.txt`, etc.
4. Switch off the old microontroller.
5. Go to Step 1 and repeat for each microcontroller.

The `generate_table.awk` program reads one of `*.txt` files and prints out an
ASCII table that can be directly embedded into this README.md file. For example
the following command produces the table in the Nano section below:

```
$ ./generate_table.awk < nano.txt
```

Fortunately, we no longer need to run `generate_table.awk` for each `*.txt`
file. The process has been automated using the `generate_readme.py` script which
will be invoked by the following command:
```
$ make README.md
```

## CPU Time Changes

* v1.2.3
    * Add benchmarks for STM32.
* v1.3
    * Replace floating point calculation in AutoBenchmark with fixed point
      calculations in micros and nanos, with the final printing done in
      micros to 3 decimal places (without using floating point ops).
    * Replace looping over a fixed elapsed time, with looping over fixed number
      of iterations for better accuracy.
        * If the elapsed time is kept constant, then the number of iteration
          of `doBaseline()` is different than the number of iterations
          of doAceRoutine()`. When we subtract, the overhead of the loop
          (e.g. `millis() - start`) are NOT canceld out correctly.
        * New numbers for `CoroutineScheduler`:
            * Nano: 5.28 -> 5.200 micros
            * Micro: 5.31 -> 5.000 micros
            * SAMD: 2.46 -> 1.933 micros
            * STM32: 1.76 -> 1.367 micros
            * ESP8266: 1.67 -> 1.100 micros
            * ESP32: 0.41 -> 0.300 micros
            * Teensy 3.2: 1.01 -> 0.500 micros
    * Add benchmark numbers for "DirectScheduling".
        * Calls `Coroutine::runCoroutine()` directly, instead of using the
          `CoroutineScheduler` to avoid the virtual dispatch.
        * Avoids overhead of cycling through the linked list.
        * Context switching using `DirectScheduling` is 3-9X faster compared to
          using `CoroutineScheduler` class.
    * Replace virtual clock ticking methods (`Coroutine::coroutineMillis()`,
      `Coroutine::coroutineMicros()`, `Coroutine::coroutineSeconds()`) with
      static calls to `ClockInterface` template class.
        * 10-40% performance improvement of `CoroutineScheduler::loop()`.
    * Remove `COROUTINE_DELAY_SECONDS()` and `COROUTINE_DELAY_MICROS()` which
      eliminates the `mDelayType` discriminator, saving 1 byte on AVR.
    * Remove `Coroutine::mName` (type `ace_common::FCString`) which saves
      3 bytes on AVR, and 8 bytes on 32-bit processors.
* v1.3.1
    * Bring back `COROUTINE_DELAY_MICROS()` and `COROUTINE_DELAY_SECONDS()`,
      using an alternate implementation that increases flash and static memory
      consumption *only* if they are used.
    * `CoroutineScheduler::runCoroutine()` now always calls
      `Coroutine::runCoroutine()` when in Delaying state, without trying to
      optimize the test for `isDelayXxxExpired()`.
        * Makes `CoroutineScheduler` slightly smaller in flash size.
        * Makes `CoroutineScheduler` slightly slower on AVR processors (e.g. 5.2
          -> 5.5 micros on AVR) , but is actually *faster* on 32-bit processors
          (e.g. 1.100 -> 0.600 micros on ESP8266).
* v1.4
    * Upgrade STM32duino Core from 1.9.0 to 2.0.0.
    * Upgrade SparkFun SAMD Core from 1.8.1 to 1.8.3.
    * No changes observed.

* v1.4.1
    * Upgrade tool chain
        * Arduino IDE from 1.8.13 to 1.8.19
        * Arduino CLI from 0.14.0 to 0.19.2
        * Arduino AVR Core from 1.8.3 to 1.8.4
        * STM32duino from 2.0.0 to 2.2.0
        * ESP8266 Core from 2.7.4 to 3.0.2
        * ESP32 Core from 1.0.6 to 2.0.2
        * Teensyduino from 1.54 to 1.56

* v1.5.0
    * Add `CoroutineProfiler` to `CoroutineScheduler`.
        * `CoroutineScheduler::runCoroutine()` becomes slightly slower:
        * 2.2-3 microseconds (AVR)
        * 0.4 microseconds (STM32)
        * 0.2-0.3 microseconds (ESP8266)
        * 0.1 microseconds (ESP32)
        * 0.03-0.17 microseconds (Teensy 3.2)

* v1.5.1
    * Upgrade tool chain
        * Arduino CLI from 0.19.2 to 0.27.1
        * Arduino AVR Core from 1.8.4 to 1.8.5
        * STM32duino from 2.2.0 to 2.3.0
        * ESP32 Core from 2.0.2 to 2.0.5
        * Teensyduino from 1.56 to 1.57

## Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.19, Arduino CLI 0.27.1
* Arduino AVR Boards 1.8.5
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(Coroutine): 16
sizeof(CoroutineScheduler): 2
sizeof(Channel<int>): 5
sizeof(LogBinProfiler): 66
sizeof(LogBinTableRenderer): 1
sizeof(LogBinJsonRenderer): 1

CPU:
+---------------------------------+--------+-------------+--------+
| Functionality                   |  iters | micros/iter |   diff |
|---------------------------------+--------+-------------+--------|
| EmptyLoop                       |  10000 |       1.900 |  0.000 |
|---------------------------------+--------+-------------+--------|
| DirectScheduling                |  10000 |       2.800 |  0.900 |
| DirectSchedulingWithProfiler    |  10000 |       5.800 |  3.900 |
|---------------------------------+--------+-------------+--------|
| CoroutineScheduling             |  10000 |       7.000 |  5.100 |
| CoroutineSchedulingWithProfiler |  10000 |       9.300 |  7.400 |
+---------------------------------+--------+-------------+--------+

```

## SparkFun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.19, Arduino CLI 0.27.1
* SparkFun AVR Boards 1.1.13
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(Coroutine): 16
sizeof(CoroutineScheduler): 2
sizeof(Channel<int>): 5
sizeof(LogBinProfiler): 66
sizeof(LogBinTableRenderer): 1
sizeof(LogBinJsonRenderer): 1

CPU:
+---------------------------------+--------+-------------+--------+
| Functionality                   |  iters | micros/iter |   diff |
|---------------------------------+--------+-------------+--------|
| EmptyLoop                       |  10000 |       1.700 |  0.000 |
|---------------------------------+--------+-------------+--------|
| DirectScheduling                |  10000 |       2.900 |  1.200 |
| DirectSchedulingWithProfiler    |  10000 |       5.700 |  4.000 |
|---------------------------------+--------+-------------+--------|
| CoroutineScheduling             |  10000 |       7.100 |  5.400 |
| CoroutineSchedulingWithProfiler |  10000 |       9.300 |  7.600 |
+---------------------------------+--------+-------------+--------+

```

## STM32

* STM32 "Blue Pill", STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.19, Arduino CLI 0.27.1
* STM32duino 2.3.0

```
Sizes of Objects:
sizeof(Coroutine): 28
sizeof(CoroutineScheduler): 4
sizeof(Channel<int>): 12
sizeof(LogBinProfiler): 68
sizeof(LogBinTableRenderer): 1
sizeof(LogBinJsonRenderer): 1

CPU:
+---------------------------------+--------+-------------+--------+
| Functionality                   |  iters | micros/iter |   diff |
|---------------------------------+--------+-------------+--------|
| EmptyLoop                       |  30000 |       0.166 |  0.000 |
|---------------------------------+--------+-------------+--------|
| DirectScheduling                |  30000 |       0.533 |  0.367 |
| DirectSchedulingWithProfiler    |  30000 |       0.933 |  0.767 |
|---------------------------------+--------+-------------+--------|
| CoroutineScheduling             |  30000 |       1.100 |  0.934 |
| CoroutineSchedulingWithProfiler |  30000 |       1.466 |  1.300 |
+---------------------------------+--------+-------------+--------+

```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.19, Arduino CLI 0.27.1
* ESP8266 Boards 3.0.2

```
Sizes of Objects:
sizeof(Coroutine): 28
sizeof(CoroutineScheduler): 4
sizeof(Channel<int>): 12
sizeof(LogBinProfiler): 68
sizeof(LogBinTableRenderer): 1
sizeof(LogBinJsonRenderer): 1

CPU:
+---------------------------------+--------+-------------+--------+
| Functionality                   |  iters | micros/iter |   diff |
|---------------------------------+--------+-------------+--------|
| EmptyLoop                       |  10000 |       0.200 |  0.000 |
|---------------------------------+--------+-------------+--------|
| DirectScheduling                |  10000 |       0.500 |  0.300 |
| DirectSchedulingWithProfiler    |  10000 |       0.800 |  0.600 |
|---------------------------------+--------+-------------+--------|
| CoroutineScheduling             |  10000 |       0.900 |  0.700 |
| CoroutineSchedulingWithProfiler |  10000 |       1.100 |  0.900 |
+---------------------------------+--------+-------------+--------+

```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.19, Arduino CLI 0.27.1
* ESP32 Boards 2.0.5

```
Sizes of Objects:
sizeof(Coroutine): 28
sizeof(CoroutineScheduler): 4
sizeof(Channel<int>): 12
sizeof(LogBinProfiler): 68
sizeof(LogBinTableRenderer): 1
sizeof(LogBinJsonRenderer): 1

CPU:
+---------------------------------+--------+-------------+--------+
| Functionality                   |  iters | micros/iter |   diff |
|---------------------------------+--------+-------------+--------|
| EmptyLoop                       |  30000 |       0.033 |  0.000 |
|---------------------------------+--------+-------------+--------|
| DirectScheduling                |  30000 |       0.100 |  0.067 |
| DirectSchedulingWithProfiler    |  30000 |       0.233 |  0.200 |
|---------------------------------+--------+-------------+--------|
| CoroutineScheduling             |  30000 |       0.333 |  0.300 |
| CoroutineSchedulingWithProfiler |  30000 |       0.433 |  0.400 |
+---------------------------------+--------+-------------+--------+

```

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.19, Arduino CLI 0.27.1
* Teensyduino 1.57
* Compiler options: "Faster"

```
Sizes of Objects:
sizeof(Coroutine): 28
sizeof(CoroutineScheduler): 4
sizeof(Channel<int>): 12
sizeof(LogBinProfiler): 68
sizeof(LogBinTableRenderer): 1
sizeof(LogBinJsonRenderer): 1

CPU:
+---------------------------------+--------+-------------+--------+
| Functionality                   |  iters | micros/iter |   diff |
|---------------------------------+--------+-------------+--------|
| EmptyLoop                       |  30000 |       0.066 |  0.000 |
|---------------------------------+--------+-------------+--------|
| DirectScheduling                |  30000 |       0.233 |  0.167 |
| DirectSchedulingWithProfiler    |  30000 |       0.266 |  0.200 |
|---------------------------------+--------+-------------+--------|
| CoroutineScheduling             |  30000 |       0.500 |  0.434 |
| CoroutineSchedulingWithProfiler |  30000 |       0.666 |  0.600 |
+---------------------------------+--------+-------------+--------+

```

