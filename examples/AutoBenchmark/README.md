# AutoBenchmark

All times in microseconds.

## Arduino Nano

### Memory Size

```
sizeof(Routine): 14
sizeof(RoutineScheduler): 2
```
### CPU

```
------------+------+------+
 AceRoutine | base | diff |
------------+------+------+
       9.71 | 3.26 | 6.44 |
------------+------+------+
```

## Teensy 3.2

The benchmark numbers for Teensy are affected greatly by the compiler
optimization settings in the Arduino IDE dropdown menu (Tools > Optimize).
CPU Speed set to 96 MHz (overclock).

### Memory Size
```
sizeof(Routine): 28
sizeof(RoutineScheduler): 4
```

### CPU

```
----------------------+------------+------+------+
Optimization          | AceRoutine | base | diff |
----------------------+------------+------+------+
Faster                |       3.65 | 1.67 | 1.98 |
Faster+LTO            |       2.10 | 0.72 | 1.38 |
Fast                  |       3.55 | 1.88 | 1.67 |
Fast+LTO              |       1.69 | 0.54 | 1.14 |
Fastest               |       3.59 | 1.83 | 1.75 |
Fastest+pure-code     |       3.61 | 2.00 | 1.60 |
Fastest+LTO           |       1.81 | 0.73 | 1.08 |
Fastest+pure-code+LTO |       1.90 | 0.72 | 1.18 |
Smallest code         |       3.88 | 1.88 | 2.00 |
Smallest code + LTO   |       1.81 | 0.68 | 1.12 |
----------------------+------------+------+------+
```

## ESP8266

Memory Size: (same as Teensy 3.2)

```
------------+------+------+
AceRoutine  | base | diff |
------------+------+------+
      10.83 | 9.11 | 1.71 |
------------+------+------+
```

## ESP32

Memory Size: (same as Teensy 3.2)

```
------------+------+------+
 AceRoutine | base | diff |
------------+------+------+
       6.59 | 6.13 | 0.46 |
------------+------+------+
```
