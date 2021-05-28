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
    * Add benchmark for 'Manual Delay Loop' which uses a simple function
      to implement the functionality of a `COROUTINE()` that loops every 10
      milliseconds.
        * Illustrates clearly that AceRoutine should rarely be used on 8-bit
          processors because it consumes 450 extra bytes for the 1st coroutine,
          and another 300 bytes for each additional coroutine.
        * On 32-bit processors with large amount of flash memory, the flash
          consumption overhead is not as limiting.

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
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        |    606/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| Manual Delay Loop               |    654/   13 |    48/    2 |
| One Coroutine                   |   1060/   34 |   454/   23 |
| One Coroutine, Delay Seconds    |   1050/   34 |   444/   23 |
| Two Coroutines                  |   1258/   55 |   652/   44 |
| Two Coroutine, Delay Seconds    |   1234/   55 |   628/   44 |
| Scheduler, One Coroutine        |   1146/   36 |   540/   25 |
| Scheduler, Two Coroutines       |   1304/   51 |   698/   40 |
+--------------------------------------------------------------+

```

## Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        |   3554/  151 |     0/    0 |
|---------------------------------+--------------+-------------|
| Manual Delay Loop               |   3602/  153 |    48/    2 |
| One Coroutine                   |   3948/  174 |   394/   23 |
| One Coroutine, Delay Seconds    |   3938/  174 |   384/   23 |
| Two Coroutines                  |   4146/  195 |   592/   44 |
| Two Coroutine, Delay Seconds    |   4122/  195 |   568/   44 |
| Scheduler, One Coroutine        |   4034/  176 |   480/   25 |
| Scheduler, Two Coroutines       |   4196/  191 |   642/   40 |
+--------------------------------------------------------------+

```

## SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Sparkfun SAMD Core 1.8.1

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        |  10072/    0 |     0/    0 |
|---------------------------------+--------------+-------------|
| Manual Delay Loop               |  10112/    0 |    40/    0 |
| One Coroutine                   |  10392/    0 |   320/    0 |
| One Coroutine, Delay Seconds    |  10400/    0 |   328/    0 |
| Two Coroutines                  |  10576/    0 |   504/    0 |
| Two Coroutine, Delay Seconds    |  10576/    0 |   504/    0 |
| Scheduler, One Coroutine        |  10440/    0 |   368/    0 |
| Scheduler, Two Coroutines       |  10536/    0 |   464/    0 |
+--------------------------------------------------------------+

```

## STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 1.9.0

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        |  19140/ 3788 |     0/    0 |
|---------------------------------+--------------+-------------|
| Manual Delay Loop               |  19164/ 3792 |    24/    4 |
| One Coroutine                   |  19364/ 3820 |   224/   32 |
| One Coroutine, Delay Seconds    |  19372/ 3820 |   232/   32 |
| Two Coroutines                  |  19532/ 3848 |   392/   60 |
| Two Coroutine, Delay Seconds    |  19540/ 3848 |   400/   60 |
| Scheduler, One Coroutine        |  19444/ 3824 |   304/   36 |
| Scheduler, Two Coroutines       |  19524/ 3852 |   384/   64 |
+--------------------------------------------------------------+

```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        | 256924/26800 |     0/    0 |
|---------------------------------+--------------+-------------|
| Manual Delay Loop               | 256988/26808 |    64/    8 |
| One Coroutine                   | 257220/26828 |   296/   28 |
| One Coroutine, Delay Seconds    | 257236/26828 |   312/   28 |
| Two Coroutines                  | 257448/26860 |   524/   60 |
| Two Coroutine, Delay Seconds    | 257464/26860 |   540/   60 |
| Scheduler, One Coroutine        | 257252/26836 |   328/   36 |
| Scheduler, Two Coroutines       | 257384/26860 |   460/   60 |
+--------------------------------------------------------------+

```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.4

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        | 197910/13092 |     0/    0 |
|---------------------------------+--------------+-------------|
| Manual Delay Loop               | 198258/13124 |   348/   32 |
| One Coroutine                   | 198446/13148 |   536/   56 |
| One Coroutine, Delay Seconds    | 198454/13148 |   544/   56 |
| Two Coroutines                  | 198698/13180 |   788/   88 |
| Two Coroutine, Delay Seconds    | 198714/13180 |   804/   88 |
| Scheduler, One Coroutine        | 198494/13156 |   584/   64 |
| Scheduler, Two Coroutines       | 198634/13180 |   724/   88 |
+--------------------------------------------------------------+

```

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53
* Compiler options: "Faster"

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        |   7628/ 3048 |     0/    0 |
|---------------------------------+--------------+-------------|
| Manual Delay Loop               |  10836/ 4152 |  3208/ 1104 |
| One Coroutine                   |  11060/ 4180 |  3432/ 1132 |
| One Coroutine, Delay Seconds    |  11068/ 4180 |  3440/ 1132 |
| Two Coroutines                  |  11240/ 4208 |  3612/ 1160 |
| Two Coroutine, Delay Seconds    |  11256/ 4208 |  3628/ 1160 |
| Scheduler, One Coroutine        |  11132/ 4184 |  3504/ 1136 |
| Scheduler, Two Coroutines       |  11260/ 4212 |  3632/ 1164 |
+--------------------------------------------------------------+

```

