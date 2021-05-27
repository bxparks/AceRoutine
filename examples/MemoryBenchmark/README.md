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
| One Coroutine                   |   1104/   34 |   498/   23 |
| Two Coroutines                  |   1310/   55 |   704/   44 |
| Scheduler, One Coroutine        |   1192/   36 |   586/   25 |
| Scheduler, Two Coroutines       |   1356/   51 |   750/   40 |
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
| One Coroutine                   |   3992/  174 |   438/   23 |
| Two Coroutines                  |   4198/  195 |   644/   44 |
| Scheduler, One Coroutine        |   4080/  176 |   526/   25 |
| Scheduler, Two Coroutines       |   4248/  191 |   694/   40 |
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
| One Coroutine                   |  10416/    0 |   344/    0 |
| Two Coroutines                  |  10592/    0 |   520/    0 |
| Scheduler, One Coroutine        |  10464/    0 |   392/    0 |
| Scheduler, Two Coroutines       |  10552/    0 |   480/    0 |
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
| One Coroutine                   |  19380/ 3820 |   240/   32 |
| Two Coroutines                  |  19540/ 3848 |   400/   60 |
| Scheduler, One Coroutine        |  19456/ 3824 |   316/   36 |
| Scheduler, Two Coroutines       |  19532/ 3852 |   392/   64 |
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
| One Coroutine                   | 257268/26828 |   344/   28 |
| Two Coroutines                  | 257496/26860 |   572/   60 |
| Scheduler, One Coroutine        | 257316/26836 |   392/   36 |
| Scheduler, Two Coroutines       | 257432/26860 |   508/   60 |
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
| One Coroutine                   | 198498/13148 |   588/   56 |
| Two Coroutines                  | 198738/13180 |   828/   88 |
| Scheduler, One Coroutine        | 198550/13156 |   640/   64 |
| Scheduler, Two Coroutines       | 198674/13180 |   764/   88 |
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
| One Coroutine                   |  11108/ 4180 |  3480/ 1132 |
| Two Coroutines                  |  11336/ 4208 |  3708/ 1160 |
| Scheduler, One Coroutine        |  11176/ 4184 |  3548/ 1136 |
| Scheduler, Two Coroutines       |  11352/ 4212 |  3724/ 1164 |
+--------------------------------------------------------------+

```

