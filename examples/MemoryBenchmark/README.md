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

**Version**: AceRoutine v1.3

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
* Arduino IDE 1.8.13
* SpenceKonde/ATTinyCore 1.5.2

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        |    400/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| One Delay Function              |    450/   13 |    50/    2 |
| Two Delay Functions             |    508/   15 |   108/    4 |
|---------------------------------+--------------+-------------|
| One Coroutine                   |    628/   30 |   228/   19 |
| Two Coroutines                  |    796/   47 |   396/   36 |
|---------------------------------+--------------+-------------|
| Scheduler, One Coroutine        |    734/   32 |   334/   21 |
| Scheduler, Two Coroutines       |    976/   49 |   576/   38 |
|---------------------------------+--------------+-------------|
| Blink Function                  |    546/   14 |   146/    3 |
| Blink Coroutine                 |    752/   30 |   352/   19 |
+--------------------------------------------------------------+

```

### Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        |    606/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| One Delay Function              |    654/   13 |    48/    2 |
| Two Delay Functions             |    714/   15 |   108/    4 |
|---------------------------------+--------------+-------------|
| One Coroutine                   |    840/   30 |   234/   19 |
| Two Coroutines                  |   1010/   47 |   404/   36 |
|---------------------------------+--------------+-------------|
| Scheduler, One Coroutine        |    946/   32 |   340/   21 |
| Scheduler, Two Coroutines       |   1188/   49 |   582/   38 |
|---------------------------------+--------------+-------------|
| Blink Function                  |    938/   14 |   332/    3 |
| Blink Coroutine                 |   1154/   30 |   548/   19 |
+--------------------------------------------------------------+

```

### SparkFun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        |   3554/  151 |     0/    0 |
|---------------------------------+--------------+-------------|
| One Delay Function              |   3602/  153 |    48/    2 |
| Two Delay Functions             |   3662/  155 |   108/    4 |
|---------------------------------+--------------+-------------|
| One Coroutine                   |   3728/  170 |   174/   19 |
| Two Coroutines                  |   3898/  187 |   344/   36 |
|---------------------------------+--------------+-------------|
| Scheduler, One Coroutine        |   3834/  172 |   280/   21 |
| Scheduler, Two Coroutines       |   4076/  189 |   522/   38 |
|---------------------------------+--------------+-------------|
| Blink Function                  |   3994/  154 |   440/    3 |
| Blink Coroutine                 |   4150/  170 |   596/   19 |
+--------------------------------------------------------------+

```

### SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* SparkFun SAMD Core 1.8.1

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        |  10072/    0 |     0/    0 |
|---------------------------------+--------------+-------------|
| One Delay Function              |  10112/    0 |    40/    0 |
| Two Delay Functions             |  10152/    0 |    80/    0 |
|---------------------------------+--------------+-------------|
| One Coroutine                   |  10224/    0 |   152/    0 |
| Two Coroutines                  |  10360/    0 |   288/    0 |
|---------------------------------+--------------+-------------|
| Scheduler, One Coroutine        |  10264/    0 |   192/    0 |
| Scheduler, Two Coroutines       |  10416/    0 |   344/    0 |
|---------------------------------+--------------+-------------|
| Blink Function                  |  10160/    0 |    88/    0 |
| Blink Coroutine                 |  10272/    0 |   200/    0 |
+--------------------------------------------------------------+

```

### STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 1.9.0

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        |  19140/ 3788 |     0/    0 |
|---------------------------------+--------------+-------------|
| One Delay Function              |  19164/ 3792 |    24/    4 |
| Two Delay Functions             |  19212/ 3792 |    72/    4 |
|---------------------------------+--------------+-------------|
| One Coroutine                   |  19256/ 3812 |   116/   24 |
| Two Coroutines                  |  19396/ 3832 |   256/   44 |
|---------------------------------+--------------+-------------|
| Scheduler, One Coroutine        |  19316/ 3816 |   176/   28 |
| Scheduler, Two Coroutines       |  19448/ 3836 |   308/   48 |
|---------------------------------+--------------+-------------|
| Blink Function                  |  19280/ 3788 |   140/    0 |
| Blink Coroutine                 |  19384/ 3808 |   244/   20 |
+--------------------------------------------------------------+

```

### ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        | 256924/26800 |     0/    0 |
|---------------------------------+--------------+-------------|
| One Delay Function              | 256988/26808 |    64/    8 |
| Two Delay Functions             | 257052/26808 |   128/    8 |
|---------------------------------+--------------+-------------|
| One Coroutine                   | 257104/26820 |   180/   20 |
| Two Coroutines                  | 257264/26844 |   340/   44 |
|---------------------------------+--------------+-------------|
| Scheduler, One Coroutine        | 257152/26828 |   228/   28 |
| Scheduler, Two Coroutines       | 257344/26844 |   420/   44 |
|---------------------------------+--------------+-------------|
| Blink Function                  | 257424/26816 |   500/   16 |
| Blink Coroutine                 | 257556/26836 |   632/   36 |
+--------------------------------------------------------------+

```

### ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.6

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        | 197910/13092 |     0/    0 |
|---------------------------------+--------------+-------------|
| One Delay Function              | 198258/13124 |   348/   32 |
| Two Delay Functions             | 198330/13124 |   420/   32 |
|---------------------------------+--------------+-------------|
| One Coroutine                   | 198358/13140 |   448/   48 |
| Two Coroutines                  | 198526/13164 |   616/   72 |
|---------------------------------+--------------+-------------|
| Scheduler, One Coroutine        | 198414/13148 |   504/   56 |
| Scheduler, Two Coroutines       | 198626/13164 |   716/   72 |
|---------------------------------+--------------+-------------|
| Blink Function                  | 198602/13120 |   692/   28 |
| Blink Coroutine                 | 198710/13136 |   800/   44 |
+--------------------------------------------------------------+

```

### Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53
* Compiler options: "Faster"

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Baseline                        |   7628/ 3048 |     0/    0 |
|---------------------------------+--------------+-------------|
| One Delay Function              |  10836/ 4152 |  3208/ 1104 |
| Two Delay Functions             |  10868/ 4152 |  3240/ 1104 |
|---------------------------------+--------------+-------------|
| One Coroutine                   |  10932/ 4172 |  3304/ 1124 |
| Two Coroutines                  |  11056/ 4192 |  3428/ 1144 |
|---------------------------------+--------------+-------------|
| Scheduler, One Coroutine        |  10980/ 4176 |  3352/ 1128 |
| Scheduler, Two Coroutines       |  11128/ 4196 |  3500/ 1148 |
|---------------------------------+--------------+-------------|
| Blink Function                  |  11244/ 4148 |  3616/ 1100 |
| Blink Coroutine                 |  11364/ 4168 |  3736/ 1120 |
+--------------------------------------------------------------+

```

