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

**Version**: AceRoutine v1.2.4

**DO NOT EDIT**: This file was auto-generated using `make README.md`.

## Dependencies

This program depends on the following libraries:

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

Version 1.2.3 adds benchmarks for STM32.

## Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(Coroutine): 15
sizeof(CoroutineScheduler): 2
sizeof(Channel<int>): 5

CPU:
+------------+------+------+
| AceRoutine | base | diff |
|------------+------+------|
|       8.43 | 3.14 | 5.28 |
+------------+------+------+

```

## Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(Coroutine): 15
sizeof(CoroutineScheduler): 2
sizeof(Channel<int>): 5

CPU:
+------------+------+------+
| AceRoutine | base | diff |
|------------+------+------|
|       8.47 | 3.16 | 5.31 |
+------------+------+------+

```

## SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Sparkfun SAMD Core 1.8.1

```
Sizes of Objects:
sizeof(Coroutine): 28
sizeof(CoroutineScheduler): 4
sizeof(Channel<int>): 12

CPU:
+------------+------+------+
| AceRoutine | base | diff |
|------------+------+------|
|       2.90 | 0.44 | 2.46 |
+------------+------+------+

```

## STM32

* STM32 "Blue Pill", STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 1.9.0

```
Sizes of Objects:
sizeof(Coroutine): 28
sizeof(CoroutineScheduler): 4
sizeof(Channel<int>): 12

CPU:
+------------+------+------+
| AceRoutine | base | diff |
|------------+------+------|
|       2.40 | 0.64 | 1.76 |
+------------+------+------+

```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
Sizes of Objects:
sizeof(Coroutine): 28
sizeof(CoroutineScheduler): 4
sizeof(Channel<int>): 12

CPU:
+------------+------+------+
| AceRoutine | base | diff |
|------------+------+------|
|       5.99 | 4.52 | 1.47 |
+------------+------+------+

```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.4

```
Sizes of Objects:
sizeof(Coroutine): 28
sizeof(CoroutineScheduler): 4
sizeof(Channel<int>): 12

CPU:
+------------+------+------+
| AceRoutine | base | diff |
|------------+------+------|
|       1.61 | 1.20 | 0.41 |
+------------+------+------+

```

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53
* Compiler options: "Faster"

```
Sizes of Objects:
sizeof(Coroutine): 28
sizeof(CoroutineScheduler): 4
sizeof(Channel<int>): 12

CPU:
+------------+------+------+
| AceRoutine | base | diff |
|------------+------+------|
|       1.17 | 0.16 | 1.01 |
+------------+------+------+

```

