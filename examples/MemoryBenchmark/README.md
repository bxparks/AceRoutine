# Memory Benchmarks

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

**Version**: AceRoutine v1.1

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
esp8266.txt
esp32.txt
teensy.txt
```

The `generate_table.awk` program reads one of `*.txt` files and prints out an
ASCII table that can be directly embedded into this README.md file. For example
the following command produces the table in the Nano section below:

```
$ ./generate_table.awk < nano.txt
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
| functionality                   |    flash/ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        |    606/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| One Coroutine                   |   1890/   54 |  1284/   43 |
| Two Coroutines                  |   2202/   85 |  1596/   74 |
| Scheduler, One Coroutine        |   2028/   64 |  1422/   53 |
| Scheduler, Two Coroutines       |   2286/   79 |  1680/   68 |
+--------------------------------------------------------------+
```

## Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13

```
+--------------------------------------------------------------+
| functionality                   |    flash/ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        |   3554/  151 |     0/    0 |
|---------------------------------+--------------+-------------|
| One Coroutine                   |   4794/  194 |  1240/   43 |
| Two Coroutines                  |   5090/  225 |  1536/   74 |
| Scheduler, One Coroutine        |   4918/  204 |  1364/   53 |
| Scheduler, Two Coroutines       |   5174/  219 |  1620/   68 |
+--------------------------------------------------------------+
```

## SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Arduino SAMD Core 1.8.6

```
+--------------------------------------------------------------+
| functionality                   |    flash/ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        |  11104/ 2368 |     0/    0 |
|---------------------------------+--------------+-------------|
| One Coroutine                   |  11740/ 2416 |   636/   48 |
| Two Coroutines                  |  11964/ 2444 |   860/   76 |
| Scheduler, One Coroutine        |  11820/ 2424 |   716/   56 |
| Scheduler, Two Coroutines       |  11964/ 2452 |   860/   84 |
+--------------------------------------------------------------+
```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
+--------------------------------------------------------------+
| functionality                   |    flash/ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        | 256924/26800 |     0/    0 |
|---------------------------------+--------------+-------------|
| One Coroutine                   | 257876/26832 |   952/   32 |
| Two Coroutines                  | 258152/26864 |  1228/   64 |
| Scheduler, One Coroutine        | 258020/26848 |  1096/   48 |
| Scheduler, Two Coroutines       | 258248/26880 |  1324/   80 |
+--------------------------------------------------------------+
```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.4

```
+--------------------------------------------------------------+
| functionality                   |    flash/ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        | 207021/14588 |     0/    0 |
|---------------------------------+--------------+-------------|
| One Coroutine                   | 207749/14620 |   728/   32 |
| Two Coroutines                  | 208065/14652 |  1044/   64 |
| Scheduler, One Coroutine        | 208829/14660 |  1808/   72 |
| Scheduler, Two Coroutines       | 209033/14684 |  2012/   96 |
+--------------------------------------------------------------+
```

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53.beta
* Compiler options: "Faster"

```
+--------------------------------------------------------------+
| functionality                   |    flash/ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        |  10832/ 4148 |     0/    0 |
|---------------------------------+--------------+-------------|
| One Coroutine                   |  12160/ 4188 |  1328/   40 |
| Two Coroutines                  |  12388/ 4216 |  1556/   68 |
| Scheduler, One Coroutine        |  12320/ 4196 |  1488/   48 |
| Scheduler, Two Coroutines       |  12452/ 4224 |  1620/   76 |
+--------------------------------------------------------------+
```
