# AutoBenchmark

## Arduino Nano

### Memory Size

```
sizeof(Routine): 13
sizeof(RoutineScheduler): 2
```
### CPU

```
------------+------+------+
 AceRoutine | base | diff |
------------+------+------+
       8.55 | 3.27 | 5.28 |
------------+------+------+
```

## Teensy 3.2

The benchmark numbers for Teensy are affected greatly by the compiler
optimization settings in the Arduino IDE dropdown menu (Tools > Optimize).

### Memory Size
```
sizeof(Routine): 24
sizeof(RoutineScheduler): 4
```

### CPU

```
----------------------+------------+------+------+
Optimization          | AceRoutine | base | diff |
----------------------+------------+------+------+
Faster                |       3.50 | 1.63 | 1.87 |
Faster+LTO            |       1.73 | 0.71 | 1.02 |
Fast                  |       3.44 | 1.92 | 1.52 |
Fast+LTO              |       1.31 | 0.55 | 0.76 |
Fastest               |       3.50 | 1.83 | 1.67 |
Fastest+pure-code     |       3.55 | 2.04 | 1.50 |
Fastest+LTO           |       1.60 | 0.72 | 0.88 |
Fastest+pure-code+LTO |       1.79 | 0.71 | 1.08 |
Smallest code         |       3.76 | 1.96 | 1.79 |
Smallest code + LTO   |       1.67 | 0.68 | 0.98 |
----------------------+------------+------+------+
```

## ESP8266

Memory Size: (same as Teensy 3.2)

```
------------+------+------+
AceRoutine  | base | diff |
------------+------+------+
      10.74 | 9.28 | 1.45 |
------------+------+------+
```

## ESP32

Memory Size: (same as Teensy 3.2)

```
------------+------+------+
 AceRoutine | base | diff |
------------+------+------+
       6.25 | 5.84 | 0.40 |
------------+------+------+
```
