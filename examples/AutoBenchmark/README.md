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

All times in microseconds.

## Arduino Nano

* AceRoutine 0.2.2
* Arduino IDE 1.8.9
* AVR Boards 1.6.23

**Memory**

```
sizeof(Coroutine): 14
sizeof(CoroutineScheduler): 2
sizeof(Channel<int>): 5
```

**CPU**

```
------------+------+------+
 AceRoutine | base | diff |
------------+------+------+
       9.58 | 3.27 | 6.31 |
------------+------+------+
```

## SAMD21 M0 Mini (Arduino MKR Zero Compatible)

* AceRoutine 0.2.2
* Arduino IDE 1.8.9
* SparkFun SAMD Boards 1.6.2

**Memory**
```
sizeof(Coroutine): 28
sizeof(CoroutineScheduler): 4
sizeof(Channel<int>): 12
```

**CPU**
```
------------+------+------+
 AceRoutine | base | diff |
------------+------+------+
       3.34 | 0.48 | 2.85 |
------------+------+------+
```

## ESP8266

* AceRoutine 0.2.2
* Arduino IDE 1.8.9
* ESP8266 Boards 2.5.2

**Memory**

```
sizeof(Coroutine): 28
sizeof(CoroutineScheduler): 4
sizeof(Channel<int>): 12
```

**CPU**

```
------------+------+------+
 AceRoutine | base | diff |
------------+------+------+
       6.00 | 4.52 | 1.48 |
------------+------+------+
```

## ESP32

* AceRoutine 0.2.2
* Arduino IDE 1.8.9
* ESP32 Boards 1.0.2

**Memory**

```
sizeof(Coroutine): 28
sizeof(CoroutineScheduler): 4
sizeof(Channel<int>): 12
```

**CPU**

```
------------+------+------+
 AceRoutine | base | diff |
------------+------+------+
       1.60 | 1.19 | 0.40 |
------------+------+------+
```

## Teensy 3.2

Prior to TeesyDuino 1.46 (or so), the benchmark numbers for Teensy were affected
greatly by the compiler optimization settings in the Arduino IDE dropdown menu
(Tools > Optimize). However, when the benchmark was run with version 1.46, the
run times did not vary as much with respect to the optimization settings. The
CPU Speed was set to 96 MHz (overclock).

* AceRoutine 0.2.2
* Arduino IDE: 1.8.9
* Teensyduino 1.46

**Memory**

```
sizeof(Coroutine): 28
sizeof(CoroutineScheduler): 4
sizeof(Channel<int>): 12
```

**CPU**

```
----------------------+------------+------+------+
Optimization          | AceRoutine | base | diff |
----------------------+------------+------+------+
Faster                |       1.29 | 0.15 | 1.13 |
Faster+LTO            |       0.85 | 0.16 | 0.68 |
Fast                  |       1.27 | 0.15 | 1.11 |
Fast+LTO              |       0.83 | 0.15 | 0.67 |
Fastest               |       1.27 | 0.15 | 1.11 |
Fastest+pure-code     |       1.27 | 0.15 | 1.11 |
Fastest+LTO           |       0.83 | 0.15 | 0.67 |
Fastest+pure-code+LTO |       0.91 | 0.15 | 0.76 |
Smallest code         |       1.35 | 0.16 | 1.19 |
Smallest code + LTO   |       1.17 | 0.16 | 1.00 |
----------------------+------------+------+------+
```
