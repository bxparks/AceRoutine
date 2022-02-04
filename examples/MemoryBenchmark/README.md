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

**Version**: AceRoutine v1.4.1

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
      180 bytes. Replacing with native `/1000` does not help much because native
      long division consumes about 130 bytes and is 3X slower on AVR processors.
    * Remove `COROUTINE_DELAY_MICRO()` functionality. Saves about 15-20 bytes
      of flash memory per coroutine on AVR. Saves 80-100 bytes plus 20-30 bytes
      of flash per coroutine on 32-bit processors.
    * Add `Blink Function` and `Blink Coroutine`, 2 implementations of the same
      asymmetric blink functionality where the HIGH level lasts for a different
      duration than the LOW level.
    * Remove `Coroutine::getName()`, `Coroutine::mName()`, and
      `Coroutine::setupCoroutine()`. The human-readable name is no longer
      retained. Saves 10-30 bytes of flash and 3 bytes of static memory per
      coroutine instance for AVR; 10-40 bytes and 8 bytes of static memory per
      instance on 32-bit processors.
    * Use 2 different `Coroutine` subclasses, instead of 2 instances of the same
      `Coroutine` subclass, for the "Scheduler, Two Coroutines" benchmark . This
      makes it more functionally equal to the "Two Coroutines" benchmark which
      automatically creates 2 different subclasses through the `COROUTINE()`
      macro. Increases flash memory by 80-130 bytes across the board.
* v1.3.1
    * Bring back `COROUTINE_DELAY_MICROS()` and `COROUTINE_DELAY_SECONDS()`
      using an implementation that increases flash and static memory *only* if
      they are used.
        * `COROUTINE_DELAY_MICROS()` actually consumes about 30 bytes fewer
          flash memory (on AVR) than `COROUTINE_DELAY()`, probably because there
          is additional overhead in the Arduino `millis()` function compared to
          `micros()`.
        * `COROUTINE_DELAY_SECONDS()` consumes about 100 bytes more flash memory
          (on AVR) compared to `COROUTINE_DELAY()` because of the division by
          1000 which must be done in software on 8-bit processors.
        * The `CoroutineScheduler` is forced to become simpler  which reduces
          flash usage by 10-20 bytes.
* v1.4
    * Upgrade STM32duino Core to 2.0.0.
        * Flash usage increases by 2.3kB across the board, but static RAM goes
          down by 250 bytes.
        * Very little change to AceRoutine code itself.
        * "Blink Function" and "Blink Coroutine" both increase flash usage by
          about 50 bytes, probably due to additional resource consumption of
          `digitalWrite()` in 2.0.0.
    * Upgrade SparkFun SAMD Core to 1.8.3.
        * No change in memory usage.
    * Add a dummy `FooClass` with virtual methods for Teensy 3.2 so that
      `FEATURE_BASELINE` becomes a useful comparison to the other benchmarks.
      Teensyduino seems to pull in `malloc()` and `free()` functions as soon as
      any class with virtual methods are uses, unlike AVR where they are pulled
      in only if the destructor is virtual.
    * Add benchmarks for calling `CoroutineScheduler::setupCoroutine()`.
      Increases flash memory by 50-60 bytes *per coroutine* (AVR) and 30-40
      bytes per coroutine (32-bit processors).

* v1.4.1
    * Upgrade tool chain
        * Arduino IDE from 1.8.13 to 1.8.19
        * Arduino CLI from 0.14.0 to 0.19.2
        * Arduino AVR Core from 1.8.3 to 1.8.4
        * STM32duino from 2.0.0 to 2.2.0
        * ESP8266 Core from 2.7.4 to 3.0.2
        * ESP32 Core from 1.0.6 to 2.0.2
        * Teensyduino from 1.54 to 1.56

## How to Generate

This requires the [AUniter](https://github.com/bxparks/AUniter) script
to execute the Arduino IDE programmatically.

The `Makefile` has rules for several microcontrollers:

```
$ make benchmarks
```
produces the following files:

```
attiny.txt
nano.txt
micro.txt
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

## Results

* Baseline: A program that does (almost) nothing
* One Delay Function: A single non-blocking delay function that waits 10 millis.
* Two Delay Functions: Two non-blocking delay functions.
* One Coroutine: One instance of `Coroutine` that waits 10 millis, executed
  directly through `runCoroutine()`.
* Two Coroutines: Two instances of `Coroutine` that wait 10 millis, executed
  directly through `runCoroutine()`.
* Scheduler, One Coroutine: One instance of `Coroutine` executed through the
  `CoroutineScheduler`.
* Scheduler, Two Coroutines: Two instances of `Coroutine` executed through the
  `CoroutineScheduler`.
* Blink Function: A function that blinks the LED asymmetrically, with HIGH
  lasting a different duration than LOW.
* Blink Coroutine: A `Coroutine` that blinks asymmetrically, exactly the same as
  the `blink()` function.

### ATtiny85

* 8MHz ATtiny85
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* SpenceKonde/ATTinyCore 1.5.2

```
+--------------------------------------------------------------------+
| functionality                         |  flash/  ram |       delta |
|---------------------------------------+--------------+-------------|
| Baseline                              |    400/   11 |     0/    0 |
|---------------------------------------+--------------+-------------|
| One Delay Function                    |    450/   13 |    50/    2 |
| Two Delay Functions                   |    508/   15 |   108/    4 |
|---------------------------------------+--------------+-------------|
| One Coroutine                         |    632/   32 |   232/   21 |
| Two Coroutines                        |    802/   51 |   402/   40 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |    600/   32 |   200/   21 |
| Two Coroutines (micros)               |    738/   51 |   338/   40 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |    728/   32 |   328/   21 |
| Two Coroutines (seconds)              |    926/   51 |   526/   40 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              |    760/   34 |   360/   23 |
| Scheduler, Two Coroutines             |    924/   53 |   524/   42 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |    728/   34 |   328/   23 |
| Scheduler, Two Coroutines (micros)    |    860/   53 |   460/   42 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |    856/   34 |   456/   23 |
| Scheduler, Two Coroutines (seconds)   |   1048/   53 |   648/   42 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |    812/   34 |   412/   23 |
| Scheduler, Two Coroutines (setup)     |   1072/   53 |   672/   42 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |    790/   34 |   390/   23 |
| Scheduler, Two Coroutines (man setup) |   1058/   53 |   658/   42 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |    546/   14 |   146/    3 |
| Blink Coroutine                       |    756/   32 |   356/   21 |
+--------------------------------------------------------------------+

```

### Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* Arduino AVR Boards 1.8.3

```
+--------------------------------------------------------------------+
| functionality                         |  flash/  ram |       delta |
|---------------------------------------+--------------+-------------|
| Baseline                              |    606/   11 |     0/    0 |
|---------------------------------------+--------------+-------------|
| One Delay Function                    |    654/   13 |    48/    2 |
| Two Delay Functions                   |    714/   15 |   108/    4 |
|---------------------------------------+--------------+-------------|
| One Coroutine                         |    844/   32 |   238/   21 |
| Two Coroutines                        |   1016/   51 |   410/   40 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |    816/   32 |   210/   21 |
| Two Coroutines (micros)               |    960/   51 |   354/   40 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |    944/   32 |   338/   21 |
| Two Coroutines (seconds)              |   1148/   51 |   542/   40 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              |    968/   34 |   362/   23 |
| Scheduler, Two Coroutines             |   1132/   53 |   526/   42 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |    940/   34 |   334/   23 |
| Scheduler, Two Coroutines (micros)    |   1076/   53 |   470/   42 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |   1068/   34 |   462/   23 |
| Scheduler, Two Coroutines (seconds)   |   1264/   53 |   658/   42 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |   1018/   34 |   412/   23 |
| Scheduler, Two Coroutines (setup)     |   1282/   53 |   676/   42 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |    996/   34 |   390/   23 |
| Scheduler, Two Coroutines (man setup) |   1268/   53 |   662/   42 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |    938/   14 |   332/    3 |
| Blink Coroutine                       |   1158/   32 |   552/   21 |
+--------------------------------------------------------------------+

```

### SparkFun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* SparkFun AVR Boards 1.1.13

```
+--------------------------------------------------------------------+
| functionality                         |  flash/  ram |       delta |
|---------------------------------------+--------------+-------------|
| Baseline                              |   3554/  151 |     0/    0 |
|---------------------------------------+--------------+-------------|
| One Delay Function                    |   3602/  153 |    48/    2 |
| Two Delay Functions                   |   3662/  155 |   108/    4 |
|---------------------------------------+--------------+-------------|
| One Coroutine                         |   3732/  172 |   178/   21 |
| Two Coroutines                        |   3904/  191 |   350/   40 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |   3704/  172 |   150/   21 |
| Two Coroutines (micros)               |   3848/  191 |   294/   40 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |   3832/  172 |   278/   21 |
| Two Coroutines (seconds)              |   4036/  191 |   482/   40 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              |   3856/  174 |   302/   23 |
| Scheduler, Two Coroutines             |   4020/  193 |   466/   42 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |   3828/  174 |   274/   23 |
| Scheduler, Two Coroutines (micros)    |   3964/  193 |   410/   42 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |   3956/  174 |   402/   23 |
| Scheduler, Two Coroutines (seconds)   |   4152/  193 |   598/   42 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |   3906/  174 |   352/   23 |
| Scheduler, Two Coroutines (setup)     |   4170/  193 |   616/   42 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |   3884/  174 |   330/   23 |
| Scheduler, Two Coroutines (man setup) |   4156/  193 |   602/   42 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |   3994/  154 |   440/    3 |
| Blink Coroutine                       |   4154/  172 |   600/   21 |
+--------------------------------------------------------------------+

```

### STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* STM32duino 2.0.0

```
+--------------------------------------------------------------------+
| functionality                         |  flash/  ram |       delta |
|---------------------------------------+--------------+-------------|
| Baseline                              |  21884/ 3540 |     0/    0 |
|---------------------------------------+--------------+-------------|
| One Delay Function                    |  21912/ 3544 |    28/    4 |
| Two Delay Functions                   |  21960/ 3544 |    76/    4 |
|---------------------------------------+--------------+-------------|
| One Coroutine                         |  22012/ 3564 |   128/   24 |
| Two Coroutines                        |  22156/ 3584 |   272/   44 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |  22076/ 3564 |   192/   24 |
| Two Coroutines (micros)               |  22220/ 3584 |   336/   44 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |  22028/ 3564 |   144/   24 |
| Two Coroutines (seconds)              |  22188/ 3584 |   304/   44 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              |  22084/ 3568 |   200/   28 |
| Scheduler, Two Coroutines             |  22188/ 3588 |   304/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |  22148/ 3568 |   264/   28 |
| Scheduler, Two Coroutines (micros)    |  22252/ 3588 |   368/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |  22100/ 3568 |   216/   28 |
| Scheduler, Two Coroutines (seconds)   |  22220/ 3588 |   336/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |  22108/ 3568 |   224/   28 |
| Scheduler, Two Coroutines (setup)     |  22240/ 3588 |   356/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |  22100/ 3568 |   216/   28 |
| Scheduler, Two Coroutines (man setup) |  22236/ 3588 |   352/   48 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |  22120/ 3540 |   236/    0 |
| Blink Coroutine                       |  22228/ 3560 |   344/   20 |
+--------------------------------------------------------------------+

```

### ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* ESP8266 Boards 2.7.4

```
+--------------------------------------------------------------------+
| functionality                         |  flash/  ram |       delta |
|---------------------------------------+--------------+-------------|
| Baseline                              | 260329/27916 |     0/    0 |
|---------------------------------------+--------------+-------------|
| One Delay Function                    | 260377/27916 |    48/    0 |
| Two Delay Functions                   | 260441/27916 |   112/    0 |
|---------------------------------------+--------------+-------------|
| One Coroutine                         | 260525/27944 |   196/   28 |
| Two Coroutines                        | 260669/27960 |   340/   44 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                | 260541/27944 |   212/   28 |
| Two Coroutines (micros)               | 260701/27960 |   372/   44 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               | 260541/27944 |   212/   28 |
| Two Coroutines (seconds)              | 260717/27960 |   388/   44 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              | 260573/27944 |   244/   28 |
| Scheduler, Two Coroutines             | 260701/27968 |   372/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     | 260589/27944 |   260/   28 |
| Scheduler, Two Coroutines (micros)    | 260733/27968 |   404/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    | 260589/27944 |   260/   28 |
| Scheduler, Two Coroutines (seconds)   | 260749/27968 |   420/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      | 260605/27944 |   276/   28 |
| Scheduler, Two Coroutines (setup)     | 260765/27968 |   436/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  | 260589/27944 |   260/   28 |
| Scheduler, Two Coroutines (man setup) | 260749/27968 |   420/   52 |
|---------------------------------------+--------------+-------------|
| Blink Function                        | 261001/27988 |   672/   72 |
| Blink Coroutine                       | 261133/28008 |   804/   92 |
+--------------------------------------------------------------------+

```

### ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* ESP32 Boards 1.0.6

```
+--------------------------------------------------------------------+
| functionality                         |  flash/  ram |       delta |
|---------------------------------------+--------------+-------------|
| Baseline                              | 204573/16060 |     0/    0 |
|---------------------------------------+--------------+-------------|
| One Delay Function                    | 204921/16084 |   348/   24 |
| Two Delay Functions                   | 204993/16084 |   420/   24 |
|---------------------------------------+--------------+-------------|
| One Coroutine                         | 205025/16108 |   452/   48 |
| Two Coroutines                        | 205189/16124 |   616/   64 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                | 205013/16108 |   440/   48 |
| Two Coroutines (micros)               | 205177/16124 |   604/   64 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               | 205041/16108 |   468/   48 |
| Two Coroutines (seconds)              | 205221/16124 |   648/   64 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              | 205089/16108 |   516/   48 |
| Scheduler, Two Coroutines             | 205229/16132 |   656/   72 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     | 205077/16108 |   504/   48 |
| Scheduler, Two Coroutines (micros)    | 205217/16132 |   644/   72 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    | 205105/16108 |   532/   48 |
| Scheduler, Two Coroutines (seconds)   | 205261/16132 |   688/   72 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      | 205117/16108 |   544/   48 |
| Scheduler, Two Coroutines (setup)     | 205289/16132 |   716/   72 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  | 205109/16108 |   536/   48 |
| Scheduler, Two Coroutines (man setup) | 205289/16132 |   716/   72 |
|---------------------------------------+--------------+-------------|
| Blink Function                        | 205273/16092 |   700/   32 |
| Blink Coroutine                       | 205377/16108 |   804/   48 |
+--------------------------------------------------------------------+

```

### Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* Teensyduino 1.53
* Compiler options: "Faster"

```
+--------------------------------------------------------------------+
| functionality                         |  flash/  ram |       delta |
|---------------------------------------+--------------+-------------|
| Baseline                              |  10232/ 4152 |     0/    0 |
|---------------------------------------+--------------+-------------|
| One Delay Function                    |  10264/ 4156 |    32/    4 |
| Two Delay Functions                   |  10292/ 4156 |    60/    4 |
|---------------------------------------+--------------+-------------|
| One Coroutine                         |  10380/ 4176 |   148/   24 |
| Two Coroutines                        |  10496/ 4196 |   264/   44 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |  10436/ 4176 |   204/   24 |
| Two Coroutines (micros)               |  10540/ 4196 |   308/   44 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |  10400/ 4176 |   168/   24 |
| Two Coroutines (seconds)              |  10536/ 4196 |   304/   44 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              |  10444/ 4180 |   212/   28 |
| Scheduler, Two Coroutines             |  10556/ 4200 |   324/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |  10500/ 4180 |   268/   28 |
| Scheduler, Two Coroutines (micros)    |  10600/ 4200 |   368/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |  10464/ 4180 |   232/   28 |
| Scheduler, Two Coroutines (seconds)   |  10596/ 4200 |   364/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |  10476/ 4180 |   244/   28 |
| Scheduler, Two Coroutines (setup)     |  10620/ 4200 |   388/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |  10464/ 4180 |   232/   28 |
| Scheduler, Two Coroutines (man setup) |  10608/ 4200 |   376/   48 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |  10688/ 4160 |   456/    8 |
| Blink Coroutine                       |  10812/ 4176 |   580/   24 |
+--------------------------------------------------------------------+

```

