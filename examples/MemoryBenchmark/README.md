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

**Version**: AceRoutine v1.5

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

* v1.5
    * Add support for human-readable names to coroutine.
        * Increases flash usage by 6-10 bytes per coroutine, even if name isn't
          used.
        * Increases static ram by 3 bytes (AVR) and 4 bytes (32-bits) per
          coroutine.

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
| One Coroutine                         |    638/   35 |   238/   24 |
| Two Coroutines                        |    822/   57 |   422/   46 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |    606/   35 |   206/   24 |
| Two Coroutines (micros)               |    758/   57 |   358/   46 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |    734/   35 |   334/   24 |
| Two Coroutines (seconds)              |    946/   57 |   546/   46 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              |    766/   37 |   366/   26 |
| Scheduler, Two Coroutines             |    944/   59 |   544/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |    734/   37 |   334/   26 |
| Scheduler, Two Coroutines (micros)    |    880/   59 |   480/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |    862/   37 |   462/   26 |
| Scheduler, Two Coroutines (seconds)   |   1068/   59 |   668/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |    818/   37 |   418/   26 |
| Scheduler, Two Coroutines (setup)     |   1092/   59 |   692/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |    796/   37 |   396/   26 |
| Scheduler, Two Coroutines (man setup) |   1078/   59 |   678/   48 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |    546/   14 |   146/    3 |
| Blink Coroutine                       |    762/   35 |   362/   24 |
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
| One Coroutine                         |    850/   35 |   244/   24 |
| Two Coroutines                        |   1036/   57 |   430/   46 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |    822/   35 |   216/   24 |
| Two Coroutines (micros)               |    980/   57 |   374/   46 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |    950/   35 |   344/   24 |
| Two Coroutines (seconds)              |   1168/   57 |   562/   46 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              |    974/   37 |   368/   26 |
| Scheduler, Two Coroutines             |   1152/   59 |   546/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |    946/   37 |   340/   26 |
| Scheduler, Two Coroutines (micros)    |   1096/   59 |   490/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |   1074/   37 |   468/   26 |
| Scheduler, Two Coroutines (seconds)   |   1284/   59 |   678/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |   1024/   37 |   418/   26 |
| Scheduler, Two Coroutines (setup)     |   1302/   59 |   696/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |   1002/   37 |   396/   26 |
| Scheduler, Two Coroutines (man setup) |   1288/   59 |   682/   48 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |    938/   14 |   332/    3 |
| Blink Coroutine                       |   1164/   35 |   558/   24 |
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
| One Coroutine                         |   3738/  175 |   184/   24 |
| Two Coroutines                        |   3924/  197 |   370/   46 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |   3710/  175 |   156/   24 |
| Two Coroutines (micros)               |   3868/  197 |   314/   46 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |   3838/  175 |   284/   24 |
| Two Coroutines (seconds)              |   4056/  197 |   502/   46 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              |   3862/  177 |   308/   26 |
| Scheduler, Two Coroutines             |   4040/  199 |   486/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |   3834/  177 |   280/   26 |
| Scheduler, Two Coroutines (micros)    |   3984/  199 |   430/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |   3962/  177 |   408/   26 |
| Scheduler, Two Coroutines (seconds)   |   4172/  199 |   618/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |   3912/  177 |   358/   26 |
| Scheduler, Two Coroutines (setup)     |   4190/  199 |   636/   48 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |   3890/  177 |   336/   26 |
| Scheduler, Two Coroutines (man setup) |   4176/  199 |   622/   48 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |   3994/  154 |   440/    3 |
| Blink Coroutine                       |   4160/  175 |   606/   24 |
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
| One Coroutine                         |  22016/ 3568 |   132/   28 |
| Two Coroutines                        |  22164/ 3592 |   280/   52 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |  22080/ 3568 |   196/   28 |
| Two Coroutines (micros)               |  22228/ 3592 |   344/   52 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |  22032/ 3568 |   148/   28 |
| Two Coroutines (seconds)              |  22196/ 3592 |   312/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              |  22088/ 3572 |   204/   32 |
| Scheduler, Two Coroutines             |  22192/ 3596 |   308/   56 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |  22152/ 3572 |   268/   32 |
| Scheduler, Two Coroutines (micros)    |  22256/ 3596 |   372/   56 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |  22104/ 3572 |   220/   32 |
| Scheduler, Two Coroutines (seconds)   |  22224/ 3596 |   340/   56 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |  22112/ 3572 |   228/   32 |
| Scheduler, Two Coroutines (setup)     |  22244/ 3596 |   360/   56 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |  22104/ 3572 |   220/   32 |
| Scheduler, Two Coroutines (man setup) |  22240/ 3596 |   356/   56 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |  22120/ 3540 |   236/    0 |
| Blink Coroutine                       |  22232/ 3564 |   348/   24 |
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
| Two Coroutines                        | 260669/27968 |   340/   52 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                | 260541/27944 |   212/   28 |
| Two Coroutines (micros)               | 260701/27968 |   372/   52 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               | 260541/27944 |   212/   28 |
| Two Coroutines (seconds)              | 260717/27968 |   388/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              | 260573/27952 |   244/   36 |
| Scheduler, Two Coroutines             | 260717/27976 |   388/   60 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     | 260589/27952 |   260/   36 |
| Scheduler, Two Coroutines (micros)    | 260733/27976 |   404/   60 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    | 260589/27952 |   260/   36 |
| Scheduler, Two Coroutines (seconds)   | 260749/27976 |   420/   60 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      | 260605/27952 |   276/   36 |
| Scheduler, Two Coroutines (setup)     | 260781/27976 |   452/   60 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  | 260589/27952 |   260/   36 |
| Scheduler, Two Coroutines (man setup) | 260765/27976 |   436/   60 |
|---------------------------------------+--------------+-------------|
| Blink Function                        | 261001/27988 |   672/   72 |
| Blink Coroutine                       | 261133/28016 |   804/  100 |
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
| One Delay Function                    | 204713/16068 |   140/    8 |
| Two Delay Functions                   | 204785/16068 |   212/    8 |
|---------------------------------------+--------------+-------------|
| One Coroutine                         | 204821/16092 |   248/   32 |
| Two Coroutines                        | 204989/16116 |   416/   56 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                | 204809/16092 |   236/   32 |
| Two Coroutines (micros)               | 204977/16116 |   404/   56 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               | 204837/16092 |   264/   32 |
| Two Coroutines (seconds)              | 205021/16116 |   448/   56 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              | 204885/16100 |   312/   40 |
| Scheduler, Two Coroutines             | 205025/16124 |   452/   64 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     | 204873/16100 |   300/   40 |
| Scheduler, Two Coroutines (micros)    | 205013/16124 |   440/   64 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    | 204901/16100 |   328/   40 |
| Scheduler, Two Coroutines (seconds)   | 205057/16124 |   484/   64 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      | 204913/16100 |   340/   40 |
| Scheduler, Two Coroutines (setup)     | 205085/16124 |   512/   64 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  | 204905/16100 |   332/   40 |
| Scheduler, Two Coroutines (man setup) | 205085/16124 |   512/   64 |
|---------------------------------------+--------------+-------------|
| Blink Function                        | 205065/16076 |   492/   16 |
| Blink Coroutine                       | 205173/16100 |   600/   40 |
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
| One Coroutine                         |  10384/ 4180 |   152/   28 |
| Two Coroutines                        |  10504/ 4204 |   272/   52 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |  10440/ 4180 |   208/   28 |
| Two Coroutines (micros)               |  10548/ 4204 |   316/   52 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |  10404/ 4180 |   172/   28 |
| Two Coroutines (seconds)              |  10544/ 4204 |   312/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              |  10448/ 4184 |   216/   32 |
| Scheduler, Two Coroutines             |  10564/ 4208 |   332/   56 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |  10504/ 4184 |   272/   32 |
| Scheduler, Two Coroutines (micros)    |  10608/ 4208 |   376/   56 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |  10468/ 4184 |   236/   32 |
| Scheduler, Two Coroutines (seconds)   |  10604/ 4208 |   372/   56 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |  10480/ 4184 |   248/   32 |
| Scheduler, Two Coroutines (setup)     |  10628/ 4208 |   396/   56 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |  10468/ 4184 |   236/   32 |
| Scheduler, Two Coroutines (man setup) |  10616/ 4208 |   384/   56 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |  10688/ 4160 |   456/    8 |
| Blink Coroutine                       |  10816/ 4180 |   584/   28 |
+--------------------------------------------------------------------+

```

