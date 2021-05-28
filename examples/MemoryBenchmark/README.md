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
    * Add benchmark for 'One Delay Function' and 'Two Delay Functions` which use
      functions with a non-blocking if-statement to implement the functionality
      of a `COROUTINE()` that loops every 10 milliseconds.
    * Remove `COROUTINE_DELAY_SECONDS()` functionality. Saves about 200 bytes on
      AVR processors, mostly from the removal of `udiv1000()` which takes almost
      180 bytes. Replacing with native `/1000` does not help that much because
      that consumes about 130 bytes and is 3X slower on AVR processors.

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
| One Delay Function              |    654/   13 |    48/    2 |
| Two Delay Functions             |    714/   15 |   108/    4 |
|---------------------------------+--------------+-------------|
| One Coroutine                   |    872/   34 |   266/   23 |
| Two Coroutines                  |   1070/   55 |   464/   44 |
|---------------------------------+--------------+-------------|
| Scheduler, One Coroutine        |    990/   36 |   384/   25 |
| Scheduler, Two Coroutines       |   1116/   51 |   510/   40 |
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
| One Delay Function              |   3602/  153 |    48/    2 |
| Two Delay Functions             |   3662/  155 |   108/    4 |
|---------------------------------+--------------+-------------|
| One Coroutine                   |   3760/  174 |   206/   23 |
| Two Coroutines                  |   3958/  195 |   404/   44 |
|---------------------------------+--------------+-------------|
| Scheduler, One Coroutine        |   3878/  176 |   324/   25 |
| Scheduler, Two Coroutines       |   4008/  191 |   454/   40 |
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
| One Delay Function              |  10112/    0 |    40/    0 |
| Two Delay Functions             |  10152/    0 |    80/    0 |
|---------------------------------+--------------+-------------|
| One Coroutine                   |  10368/    0 |   296/    0 |
| Two Coroutines                  |  10544/    0 |   472/    0 |
|---------------------------------+--------------+-------------|
| Scheduler, One Coroutine        |  10416/    0 |   344/    0 |
| Scheduler, Two Coroutines       |  10504/    0 |   432/    0 |
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
| One Delay Function              |  19164/ 3792 |    24/    4 |
| Two Delay Functions             |  19212/ 3792 |    72/    4 |
|---------------------------------+--------------+-------------|
| One Coroutine                   |  19348/ 3820 |   208/   32 |
| Two Coroutines                  |  19512/ 3848 |   372/   60 |
|---------------------------------+--------------+-------------|
| Scheduler, One Coroutine        |  19428/ 3824 |   288/   36 |
| Scheduler, Two Coroutines       |  19504/ 3852 |   364/   64 |
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
| One Delay Function              | 256988/26808 |    64/    8 |
| Two Delay Functions             | 257052/26808 |   128/    8 |
|---------------------------------+--------------+-------------|
| One Coroutine                   | 257188/26828 |   264/   28 |
| Two Coroutines                  | 257416/26860 |   492/   60 |
|---------------------------------+--------------+-------------|
| Scheduler, One Coroutine        | 257220/26836 |   296/   36 |
| Scheduler, Two Coroutines       | 257352/26860 |   428/   60 |
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
| One Delay Function              | 198258/13124 |   348/   32 |
| Two Delay Functions             | 198330/13124 |   420/   32 |
|---------------------------------+--------------+-------------|
| One Coroutine                   | 198426/13148 |   516/   56 |
| Two Coroutines                  | 198682/13180 |   772/   88 |
|---------------------------------+--------------+-------------|
| Scheduler, One Coroutine        | 198470/13156 |   560/   64 |
| Scheduler, Two Coroutines       | 198618/13180 |   708/   88 |
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
| One Delay Function              |  10836/ 4152 |  3208/ 1104 |
| Two Delay Functions             |  10868/ 4152 |  3240/ 1104 |
|---------------------------------+--------------+-------------|
| One Coroutine                   |  11032/ 4180 |  3404/ 1132 |
| Two Coroutines                  |  11184/ 4208 |  3556/ 1160 |
|---------------------------------+--------------+-------------|
| Scheduler, One Coroutine        |  11100/ 4184 |  3472/ 1136 |
| Scheduler, Two Coroutines       |  11200/ 4212 |  3572/ 1164 |
+--------------------------------------------------------------+

```

