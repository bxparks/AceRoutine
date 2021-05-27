#!/usr/bin/python3
#
# Python script that regenerates the README.md from the embedded template. Uses
# ./generate_table.awk to regenerate the ASCII tables from the various *.txt
# files.

from subprocess import check_output

nano_results = check_output(
    "./generate_table.awk < nano.txt", shell=True, text=True)
micro_results = check_output(
    "./generate_table.awk < micro.txt", shell=True, text=True)
samd_results = check_output(
    "./generate_table.awk < samd.txt", shell=True, text=True)
stm32_results = check_output(
    "./generate_table.awk < stm32.txt", shell=True, text=True)
esp8266_results = check_output(
    "./generate_table.awk < esp8266.txt", shell=True, text=True)
esp32_results = check_output(
    "./generate_table.awk < esp32.txt", shell=True, text=True)
teensy32_results = check_output(
    "./generate_table.awk < teensy32.txt", shell=True, text=True)

print(f"""\
# Memory Benchmark

The `MemoryBenchmark.ino` program compiles example code snippets using the
AceRoutine library. The `FEATURE` macro flag controls which feature is compiled.
The `collect.sh` edits this `FEATURE` flag programmatically, then runs the
Arduino IDE compiler on the program, and extracts the flash and static memory
usage into a text file (e.g. `nano.txt`).

The numbers shown below should be considered to be rough estimates. It is often
difficult to separate out the code size of the library from the overhead imposed
by the runtime environment of the processor. For example, it often seems like
the ESP8266 allocates flash memory in blocks of a certain quantity, so the
calculated flash size can jump around in unexpected ways.

**NOTE**: This file was auto-generated using `make README.md`. DO NOT EDIT.

**Version**: AceRoutine v1.3

**Changes**:

* v1.1 to v1.2 saw a significant *decrease* of flash usage by about 200-400
  bytes for various scenarios. This was due to several factors including:
    * `Coroutine::setupCoroutine()` no longer sorts the coroutines in the linked
      list by name
    * `Coroutine::suspend()` and `resume()` no longer modify the linked list
    * `CoroutineScheduler::runCoroutine()` simplified due to immutable
      linked list
    * `CoroutineScheduler::setupScheduler()` simplified due to immutable linked
      list
* v1.3
    * Remove virtual destructor on `Coroutine` class. Reduces flash memory
      consumption by 500-600 bytes on AVR processors, 350 bytes on SAMD21, and
      50-150 bytes on other 32-bit processors. The static memory is also reduced
      by 14 bytes on AVR processors.
    * Replace clock ticking virtual methods (`coroutineMicros()`,
      `coroutineMillis()`, and `coroutineSeconds()`) with static functions that
      delegate to `ClockInterface` which is a template parameter. Saves only
      0-40 bytes of flash on on AVR processors, but 100-1500 bytes of flash on
      32-bit processors.

## How to Generate

This requires the [AUniter](https://github.com/bxparks/AUniter) script
to execute the Arduino IDE programmatically.

The `Makefile` has rules for several microcontrollers:

```
$ make benchmarks
```
produces the following files:

```
nano.txt
micro.txt
samd.txt
stm32.txt
esp8266.txt
esp32.txt
teensy32.txt
```

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

## Functionality

* Baseline: A program that does (almost) nothing
* Coroutine (bare): A single `COROUTINE()` macro that does nothing.
* Coroutine (LOOP,DELAY): A `COROUTINE()` macro that uses `COROUTINE_LOOP()`
  and `COROUTINE_DELAY()` which are expected to used in the common case.
* CoroutineScheduler (bare): A single `Coroutine` instance with a
* `CoroutineScheduler`.
* CoroutineScheduler (LOOP,DELAY): A single `Coroutine` instance that
  uses `COROUTINE_LOOP()` and `COROUTINE_DELAY()` managed by a
  `CoroutineScheduler`.

## Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3

```
{nano_results}
```

## Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13

```
{micro_results}
```

## SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Sparkfun SAMD Core 1.8.1

```
{samd_results}
```

## STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 1.9.0

```
{stm32_results}
```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
{esp8266_results}
```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.4

```
{esp32_results}
```

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53
* Compiler options: "Faster"

```
{teensy32_results}
```
""")
