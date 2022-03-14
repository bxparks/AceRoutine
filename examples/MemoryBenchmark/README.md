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
    * Add a `CoroutineLogBinProfiler` to 2 coroutines.
        * Use a 3rd coroutine to print the frequency count to the `Serial`
          output every 10 seconds.
        * Provide `CoroutineLogBinProfiler::printBinsTo()` which prints
          a table of the frequency count over all coroutines. This represents a
          poor-man's version of the log-log graph of the frequency count.
        * Increases the flash size of the `CoroutineScheduler` by 100-140 bytes
          for both 8-bit and 32-bit processors, even if `CoroutineProfiler` is
          not used. This is a one-time hit.

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
| One Coroutine                         |    642/   37 |   242/   26 |
| Two Coroutines                        |    834/   61 |   434/   50 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |    610/   37 |   210/   26 |
| Two Coroutines (micros)               |    770/   61 |   370/   50 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |    738/   37 |   338/   26 |
| Two Coroutines (seconds)              |    958/   61 |   558/   50 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              |    866/   39 |   466/   28 |
| Scheduler, Two Coroutines             |   1052/   63 |   652/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |    834/   39 |   434/   28 |
| Scheduler, Two Coroutines (micros)    |    988/   63 |   588/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |    962/   39 |   562/   28 |
| Scheduler, Two Coroutines (seconds)   |   1176/   63 |   776/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |    920/   39 |   520/   28 |
| Scheduler, Two Coroutines (setup)     |   1202/   63 |   802/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |    896/   39 |   496/   28 |
| Scheduler, Two Coroutines (man setup) |   1186/   63 |   786/   52 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |    546/   14 |   146/    3 |
| Blink Coroutine                       |    766/   37 |   366/   26 |
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
| One Coroutine                         |    854/   37 |   248/   26 |
| Two Coroutines                        |   1048/   61 |   442/   50 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |    826/   37 |   220/   26 |
| Two Coroutines (micros)               |    992/   61 |   386/   50 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |    954/   37 |   348/   26 |
| Two Coroutines (seconds)              |   1180/   61 |   574/   50 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              |   1078/   39 |   472/   28 |
| Scheduler, Two Coroutines             |   1264/   63 |   658/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |   1050/   39 |   444/   28 |
| Scheduler, Two Coroutines (micros)    |   1208/   63 |   602/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |   1178/   39 |   572/   28 |
| Scheduler, Two Coroutines (seconds)   |   1396/   63 |   790/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |   1128/   39 |   522/   28 |
| Scheduler, Two Coroutines (setup)     |   1414/   63 |   808/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |   1106/   39 |   500/   28 |
| Scheduler, Two Coroutines (man setup) |   1400/   63 |   794/   52 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |    938/   14 |   332/    3 |
| Blink Coroutine                       |   1168/   37 |   562/   26 |
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
| One Coroutine                         |   3742/  177 |   188/   26 |
| Two Coroutines                        |   3936/  201 |   382/   50 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |   3714/  177 |   160/   26 |
| Two Coroutines (micros)               |   3880/  201 |   326/   50 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |   3842/  177 |   288/   26 |
| Two Coroutines (seconds)              |   4068/  201 |   514/   50 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              |   3966/  179 |   412/   28 |
| Scheduler, Two Coroutines             |   4152/  203 |   598/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |   3938/  179 |   384/   28 |
| Scheduler, Two Coroutines (micros)    |   4096/  203 |   542/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |   4066/  179 |   512/   28 |
| Scheduler, Two Coroutines (seconds)   |   4284/  203 |   730/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |   4016/  179 |   462/   28 |
| Scheduler, Two Coroutines (setup)     |   4302/  203 |   748/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |   3994/  179 |   440/   28 |
| Scheduler, Two Coroutines (man setup) |   4288/  203 |   734/   52 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |   3994/  154 |   440/    3 |
| Blink Coroutine                       |   4164/  177 |   610/   26 |
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
| One Coroutine                         |  22016/ 3572 |   132/   32 |
| Two Coroutines                        |  22164/ 3600 |   280/   60 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |  22080/ 3572 |   196/   32 |
| Two Coroutines (micros)               |  22228/ 3600 |   344/   60 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |  22032/ 3572 |   148/   32 |
| Two Coroutines (seconds)              |  22196/ 3600 |   312/   60 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              |  22208/ 3576 |   324/   36 |
| Scheduler, Two Coroutines             |  22312/ 3604 |   428/   64 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |  22208/ 3576 |   324/   36 |
| Scheduler, Two Coroutines (micros)    |  22312/ 3604 |   428/   64 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |  22224/ 3576 |   340/   36 |
| Scheduler, Two Coroutines (seconds)   |  22344/ 3604 |   460/   64 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |  22232/ 3576 |   348/   36 |
| Scheduler, Two Coroutines (setup)     |  22364/ 3604 |   480/   64 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |  22224/ 3576 |   340/   36 |
| Scheduler, Two Coroutines (man setup) |  22360/ 3604 |   476/   64 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |  22120/ 3540 |   236/    0 |
| Blink Coroutine                       |  22232/ 3568 |   348/   28 |
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
| One Coroutine                         | 260525/27952 |   196/   36 |
| Two Coroutines                        | 260685/27976 |   356/   60 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                | 260541/27952 |   212/   36 |
| Two Coroutines (micros)               | 260701/27976 |   372/   60 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               | 260541/27952 |   212/   36 |
| Two Coroutines (seconds)              | 260717/27976 |   388/   60 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              | 260669/27952 |   340/   36 |
| Scheduler, Two Coroutines             | 260813/27984 |   484/   68 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     | 260669/27952 |   340/   36 |
| Scheduler, Two Coroutines (micros)    | 260813/27984 |   484/   68 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    | 260701/27952 |   372/   36 |
| Scheduler, Two Coroutines (seconds)   | 260861/27984 |   532/   68 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      | 260701/27952 |   372/   36 |
| Scheduler, Two Coroutines (setup)     | 260877/27984 |   548/   68 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  | 260685/27952 |   356/   36 |
| Scheduler, Two Coroutines (man setup) | 260861/27984 |   532/   68 |
|---------------------------------------+--------------+-------------|
| Blink Function                        | 261001/27988 |   672/   72 |
| Blink Coroutine                       | 261149/28016 |   820/  100 |
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
| One Delay Function                    | 205129/16092 |   556/   32 |
| Two Delay Functions                   | 205201/16092 |   628/   32 |
|---------------------------------------+--------------+-------------|
| One Coroutine                         | 205241/16124 |   668/   64 |
| Two Coroutines                        | 205413/16148 |   840/   88 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                | 205229/16124 |   656/   64 |
| Two Coroutines (micros)               | 205401/16148 |   828/   88 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               | 205257/16124 |   684/   64 |
| Two Coroutines (seconds)              | 205445/16148 |   872/   88 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              | 205397/16124 |   824/   64 |
| Scheduler, Two Coroutines             | 205537/16156 |   964/   96 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     | 205373/16124 |   800/   64 |
| Scheduler, Two Coroutines (micros)    | 205513/16156 |   940/   96 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    | 205413/16124 |   840/   64 |
| Scheduler, Two Coroutines (seconds)   | 205569/16156 |   996/   96 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      | 205425/16124 |   852/   64 |
| Scheduler, Two Coroutines (setup)     | 205597/16156 |  1024/   96 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  | 205417/16124 |   844/   64 |
| Scheduler, Two Coroutines (man setup) | 205597/16156 |  1024/   96 |
|---------------------------------------+--------------+-------------|
| Blink Function                        | 205481/16100 |   908/   40 |
| Blink Coroutine                       | 205593/16124 |  1020/   64 |
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
| One Coroutine                         |  10388/ 4184 |   156/   32 |
| Two Coroutines                        |  10508/ 4212 |   276/   60 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |  10444/ 4184 |   212/   32 |
| Two Coroutines (micros)               |  10552/ 4212 |   320/   60 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |  10408/ 4184 |   176/   32 |
| Two Coroutines (seconds)              |  10548/ 4212 |   316/   60 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine              |  10568/ 4188 |   336/   36 |
| Scheduler, Two Coroutines             |  10684/ 4216 |   452/   64 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |  10556/ 4188 |   324/   36 |
| Scheduler, Two Coroutines (micros)    |  10660/ 4216 |   428/   64 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |  10588/ 4188 |   356/   36 |
| Scheduler, Two Coroutines (seconds)   |  10724/ 4216 |   492/   64 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |  10600/ 4188 |   368/   36 |
| Scheduler, Two Coroutines (setup)     |  10748/ 4216 |   516/   64 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |  10588/ 4188 |   356/   36 |
| Scheduler, Two Coroutines (man setup) |  10736/ 4216 |   504/   64 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |  10688/ 4160 |   456/    8 |
| Blink Coroutine                       |  10820/ 4184 |   588/   32 |
+--------------------------------------------------------------------+

```

