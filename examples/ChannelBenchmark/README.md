# Channel Benchmark

The `ChannelBenchmark` tries to determine the amount of time taken by the usage
of a `Channel. It uses 2 coroutines and 2 channels between them. The master
coroutines starts by writing a `long` message to the `slave`. The slaves takes
the integer, increments it by one, then sends it back to the `master`. The
master does the same thing. Each time the counter increments by one, there was
one `Channel::read()` and one `Channel::write` operation performed.

The times under the `diff` column is the approximate time taken by the
2 `Channel` operations, with the overhead of the incrementing the counter,
and the `CoroutineScheduler` context switching subtracted (the `base` column).

All times in microseconds

## Arduino Nano

```
------------+------+------+
    Channel | base | diff |
------------+------+------+
      42.83 |11.00 |31.82 |
------------+------+------+
```

## Teensy 3.2

```
------------+------+------+
    Channel | base | diff |
------------+------+------+
       6.24 | 1.31 | 4.93 |
------------+------+------+
```

## ESP8266

```
------------+------+------+
    Channel | base | diff |
------------+------+------+
      20.13 | 6.16 |13.97 |
------------+------+------+
```

## ESP32

```
------------+------+------+
    Channel | base | diff |
------------+------+------+
       5.66 | 1.69 | 3.96 |
------------+------+------+
```

Note: The ESP32 results seems to be sensitive to compiler optimization. The
addition of a single `Serial.println(counter)` at the end of the benchmark can
cause the numbers for the `doMasterSlaveChannel()` to increase by a factor of
3X. This effect was not seen in any other environments.
