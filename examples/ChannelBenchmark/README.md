# ChannelBenchmark

The `ChannelBenchmark` estimates the CPU overhead of writing and reading from
a Channel. The "Baseline" uses a Coroutine to increment a counter to a
pre-determined `NUM_COUNT`, and measures the microseconds per iteration of the
counter.

The "Channels" benchmark keeps the original Coroutine that counts, and adds a
writer Coroutine that writes to a Channel, and a reader Coroutine that reads
from the same Channel. We then measure the time take to increase the counter to
`NUM_COUNT` as before, and calculate the microseconds per iteration.

The times under the `diff` column below is the difference per iteration
which is an approximation of how much overhead the Channel write and read
operations took, per iteration.

All times in below are in microseconds.

**Version**: AceRoutine v1.5.0

**DO NOT EDIT**: This file was auto-generated using `make README.md`.

## Dependencies

This program depends on the following libraries:

* [AceRoutine](https://github.com/bxparks/AceRoutine)
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

## Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* Arduino AVR Boards 1.8.4
* `micros()` has a resolution of 4 microseconds

```

CPU:
+---------------+--------+-------------+--------+
| Functionality |  count | micros/iter |   diff |
+---------------+--------+-------------+--------+
| Baseline      |  10000 |      16.400 |  0.000 |
| Channels      |  10000 |      28.100 | 11.700 |
+---------------+--------+-------------+--------+

```

## SparkFun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* SparkFun AVR Boards 1.1.13
* `micros()` has a resolution of 4 microseconds

```

CPU:
+---------------+--------+-------------+--------+
| Functionality |  count | micros/iter |   diff |
+---------------+--------+-------------+--------+
| Baseline      |  10000 |      16.500 |  0.000 |
| Channels      |  10000 |      28.300 | 11.800 |
+---------------+--------+-------------+--------+

```

## STM32

* STM32 "Blue Pill", STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* STM32duino 2.2.0

```

CPU:
+---------------+--------+-------------+--------+
| Functionality |  count | micros/iter |   diff |
+---------------+--------+-------------+--------+
| Baseline      |  50000 |       3.040 |  0.000 |
| Channels      |  50000 |       4.460 |  1.420 |
+---------------+--------+-------------+--------+

```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* ESP8266 Boards 3.0.2

```

CPU:
+---------------+--------+-------------+--------+
| Functionality |  count | micros/iter |   diff |
+---------------+--------+-------------+--------+
| Baseline      |  10000 |       2.700 |  0.000 |
| Channels      |  10000 |       4.200 |  1.500 |
+---------------+--------+-------------+--------+

```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* ESP32 Boards 2.0.2

```

CPU:
+---------------+--------+-------------+--------+
| Functionality |  count | micros/iter |   diff |
+---------------+--------+-------------+--------+
| Baseline      | 100000 |       0.880 |  0.000 |
| Channels      | 100000 |       1.340 |  0.460 |
+---------------+--------+-------------+--------+

```

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* Teensyduino 1.56
* Compiler options: "Faster"

```

CPU:
+---------------+--------+-------------+--------+
| Functionality |  count | micros/iter |   diff |
+---------------+--------+-------------+--------+
| Baseline      | 100000 |       0.950 |  0.000 |
| Channels      | 100000 |       2.130 |  1.180 |
+---------------+--------+-------------+--------+

```

