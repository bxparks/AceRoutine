# AceRoutine

[![AUnit Tests](https://github.com/bxparks/AceRoutine/actions/workflows/aunit_tests.yml/badge.svg)](https://github.com/bxparks/AceRoutine/actions/workflows/aunit_tests.yml)

**NEW: Profiling in v1.5**: Version 1.5 adds the ability to profile the
execution time of `Coroutine::runCoroutine()` and render the histogram as a
table or a JSON object. See [Coroutine
Profiling](USER_GUIDE.md#CoroutineProfiling) for details.

A low-memory, fast-switching, cooperative multitasking library using
stackless coroutines on Arduino platforms.

This library is an implementation of the
[ProtoThreads](http://dunkels.com/adam/pt) library for the
Arduino platform. It emulates a stackless coroutine that can suspend execution
using a `yield()` or `delay()` functionality to allow other coroutines to
execute. When the scheduler makes its way back to the original coroutine, the
execution continues right after the `yield()` or `delay()`.

There are only 2 core classes in this library:

* `Coroutine` class provides the context variables for all coroutines
* `CoroutineScheduler` class handles the scheduling (optional)

The following classes are used for profiling:

* `CoroutineProfiler` interface
* `LogBinProfiler` provides an implementation that tracks the execution time
  in 32 logarithmic bins from 1us to 4295s.
* `LogBinTableRenderer` prints the histogram as a table
* `LogBinJsonRenderer` prints the histogram as a JSON object

The following is an experimental feature whose API and functionality may change
considerably in the future:

* `Channel` class allows coroutines to send messages to each other

The library provides a number of macros to help create coroutines and manage
their life cycle:

* `COROUTINE()`: defines an instance of the `Coroutine` class or an
  instance of a user-defined subclass of `Coroutine`
* `COROUTINE_BEGIN()`: must occur at the start of a coroutine body
* `COROUTINE_END()`: must occur at the end of the coroutine body
* `COROUTINE_YIELD()`: yields execution back to the caller, often
  `CoroutineScheduler` but not necessarily
* `COROUTINE_AWAIT(condition)`: yield until `condition` becomes `true`
* `COROUTINE_DELAY(millis)`: yields back execution for `millis`. The `millis`
  parameter is defined as a `uint16_t`.
* `COROUTINE_DELAY_MICROS(micros)`: yields back execution for `micros`. The
  `micros` parameter is defined as a `uint16_t`.
* `COROUTINE_DELAY_SECONDS(seconds)`: yields back execution for
  `seconds`. The `seconds` parameter is defined as a `uint16_t`.
* `COROUTINE_LOOP()`: convenience macro that loops forever
* `COROUTINE_CHANNEL_WRITE(channel, value)`: writes a value to a `Channel`
* `COROUTINE_CHANNEL_READ(channel, value)`: reads a value from a `Channel`

Here are some of the compelling features of this library compared to
others (in my opinion of course):

* low memory usage
    * 8-bit (e.g. AVR) processors:
        * the first `Coroutine` consumes about 230 bytes of flash
        * each additional `Coroutine` consumes 170 bytes of flash
        * each `Coroutine` consumes 16 bytes of static RAM
        * `CoroutineScheduler` consumes only about 40 bytes of flash and
          2 bytes of RAM independent of the number of coroutines
    * 32-bit (e.g. STM32, ESP8266, ESP32) processors
        * the first `Coroutine` consumes between 120-450 bytes of flash
        * each additional `Coroutine` consumes about 130-160 bytes of flash,
        * each `Coroutine` consumes 28 bytes of static RAM
        * `CoroutineScheduler` consumes only about 40-60 bytes of flash
          and 4 bytes of static RAM independent of the number of coroutines
* extremely fast context switching
    * Direct Scheduling (call `Coroutine::runCoroutine()` directly)
        * ~1.0 microseconds on a 16 MHz ATmega328P
        * ~0.4 microseconds on a 48 MHz SAMD21
        * ~0.3 microseconds on a 72 MHz STM32
        * ~0.3 microseconds on a 80 MHz ESP8266
        * ~0.1 microseconds on a 240 MHz ESP32
        * ~0.17 microseconds on 96 MHz Teensy 3.2 (depending on compiler
          settings)
    * Coroutine Scheduling (use `CoroutineScheduler::loop()`):
        * ~5.2 microseconds on a 16 MHz ATmega328P
        * ~1.3 microseconds on a 48 MHz SAMD21
        * ~0.9 microseconds on a 72 MHz STM32
        * ~0.8 microseconds on a 80 MHz ESP8266
        * ~0.3 microseconds on a 240 MHz ESP32
        * ~0.4 microseconds on 96 MHz Teensy 3.2 (depending on compiler
          settings)
* uses the
  [computed goto](https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html)
  feature of the GCC compiler (also supported by Clang) to avoid the
  [Duff's Device](https://en.wikipedia.org/wiki/Duff%27s_device) hack
    * allows `switch` statements in the coroutines
* C/C++ macros eliminate boilerplate code and make the code easy to read
* the base `Coroutine` class is easy to subclass to add additional variables and
  functions
* fully unit tested using [AUnit](https://github.com/bxparks/AUnit)

Some limitations are:

* A `Coroutine` cannot return any values.
* A `Coroutine` is stackless and therefore cannot preserve local stack variables
  across multiple calls. Often the class member variables or function static
  variables are reasonable substitutes.
* Coroutines are designed to be statically allocated, not dynamically created
  and destroyed on the heap. Dynamic memory allocation on an 8-bit
  microcontroller with 2kB of RAM would cause too much heap fragmentation. And
  the virtual destructor pulls in `malloc()` and `free()` which increases flash
  memory by 600 bytes on AVR processors.
* A `Channel` is an experimental feature and has limited features. It is
  currently an unbuffered, synchronized channel. It can be used by only one
  reader and one writer.

After I had completed most of this library, I discovered that I had essentially
reimplemented the `<ProtoThread.h>` library in the
[Cosa framework](https://github.com/mikaelpatel/Cosa). The difference is that
AceRoutine is a self-contained library that works on any platform supporting the
Arduino API (AVR, Teensy, ESP8266, ESP32, etc), and it provides a handful of
additional macros that can reduce boilerplate code.

**Version**: 1.5.1 (2022-09-20)

**Changelog**: [CHANGELOG.md](CHANGELOG.md)

## Table of Contents

* [Hello Coroutines](#HelloCoroutines)
    * [Hello Coroutine](#HelloCoroutine)
    * [Hello Scheduler](#HelloScheduler)
    * [Hello Manual Coroutine](#HelloManualCoroutine)
    * [Hello Coroutine with Profiler](#HelloCoroutineWithProfiler)
    * [Hello Scheduler with Profiler](#HelloSchedulerWithProfiler)
* [Installation](#Installation)
    * [Source Code](#SourceCode)
* [Documentation](#Documentation)
    * [Examples](#Examples)
* [Comparisons](#Comparisons)
* [Resource Consumption](#ResourceConsumption)
    * [Static Memory](#StaticMemory)
    * [Flash Memory](#FlashMemory)
    * [CPU](#CPU)
* [System Requirements](#SystemRequirements)
    * [Hardware](#Hardware)
    * [Tool Chain](#ToolChain)
    * [Operating System](#OperatingSystem)
* [License](#LicenseSystemRequirements)
* [Feedback and Support](#FeedbackAndSupport)
* [Authors](#Authors)

<a name="HelloCoroutines"></a>
## Hello Coroutines

<a name="HelloCoroutine"></a>
### HelloCoroutine

This is the [HelloCoroutine.ino](examples/HelloCoroutine) sample sketch which
uses the `COROUTINE()` macro to automatically handle a number of boilerplate
code, and some internal bookkeeping operations. Using the `COROUTINE()` macro
works well for relatively small and simple coroutines.

```C++
#include <AceRoutine.h>
using namespace ace_routine;

const int LED = LED_BUILTIN;
const int LED_ON = HIGH;
const int LED_OFF = LOW;

COROUTINE(blinkLed) {
  COROUTINE_LOOP() {
    digitalWrite(LED, LED_ON);
    COROUTINE_DELAY(100);
    digitalWrite(LED, LED_OFF);
    COROUTINE_DELAY(500);
  }
}

COROUTINE(printHelloWorld) {
  COROUTINE_LOOP() {
    Serial.print(F("Hello, "));
    Serial.flush();
    COROUTINE_DELAY(1000);
    Serial.println(F("World"));
    COROUTINE_DELAY(4000);
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro
  pinMode(LED, OUTPUT);
}

void loop() {
  blinkLed.runCoroutine();
  printHelloWorld.runCoroutine();
}
```

The `printHelloWorld` coroutine prints "Hello, ", waits 1 second, then prints
"World", then waits 4 more seconds, then repeats from the start. At the same
time, the `blinkLed` coroutine blinks the builtin LED on and off, on for 100 ms
and off for 500 ms.

<a name="HelloScheduler"></a>
### HelloScheduler

The [HelloScheduler.ino](examples/HelloScheduler) sketch implements the same
thing using the `CoroutineScheduler`:

```C++
#include <AceRoutine.h>
using namespace ace_routine;

... // same as above

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro
  pinMode(LED, OUTPUT);

  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
```

The `CoroutineScheduler` can automatically manage all coroutines defined by the
`COROUTINE()` macro, which eliminates the need to itemize your coroutines in the
`loop()` method manually. Unfortunately, this convenience is not free (see
[MemoryBenchmark](examples/MemoryBenchmark)):

* The `CoroutineScheduler` singleton instance increases the flash memory by
  about 110 bytes.
* The `CoroutineScheduler::loop()` method calls the `Coroutine::runCoroutine()`
  method through the `virtual` dispatch instead of directly, which is slower and
  takes more flash memory.
* Each `Coroutine` instance consumes an additional ~70 bytes of flash
  when using the `CoroutineScheduler`.

On 8-bit processors with limited memory, the additional resource consumption can
be important. On 32-bit processors with far more memory, these additional
resources are often inconsequential. Therefore the `CoroutineScheduler` is
recommended mostly on 32-bit processors.

<a name="HelloManualCoroutine"></a>
### HelloManualCoroutine

The [HelloManualCoroutine.ino](examples/HelloManualCoroutine) program shows what
the code looks like without the convenience of the `COROUTINE()` macro. For more
complex programs, with more than a few coroutines, especially if the coroutines
need to communicate with each other, this coding structure can be more powerful.

```C++
#include <Arduino.h>
#include <AceRoutine.h>
using namespace ace_routine;

const int LED = LED_BUILTIN;
const int LED_ON = HIGH;
const int LED_OFF = LOW;

class BlinkLedCoroutine: public Coroutine {
  public:
    int runCoroutine() override {
      COROUTINE_LOOP() {
        digitalWrite(LED, LED_ON);
        COROUTINE_DELAY(100);
        digitalWrite(LED, LED_OFF);
        COROUTINE_DELAY(500);
      }
    }
};

class PrintHelloWorldCoroutine: public Coroutine {
  public:
    int runCoroutine() override {
      COROUTINE_LOOP() {
        Serial.print(F("Hello, "));
        Serial.flush();
        COROUTINE_DELAY(1000);
        Serial.println(F("World"));
        COROUTINE_DELAY(4000);
      }
    }
};

BlinkLedCoroutine blinkLed;
PrintHelloWorldCoroutine printHelloWorld;

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro
  pinMode(LED, OUTPUT);
}

void loop() {
  blinkLed.runCoroutine();
  printHelloWorld.runCoroutine();
}
```

### HelloCoroutineWithProfiler

Version 1.5 added support for profiling the execution time of
`Coroutine::runCoroutine()` through the `CoroutineProfiler` interface. Currently
only a single implementation (`LogBinProfiler`) is provided.

The [HelloCoroutineWithProfiler.ino](examples/HelloCoroutineWithProfiler)
program shows how to setup the profilers and extract the profiling information
using the `Coroutine::runCoroutineWithProfiler()` instead of the usual
`Coroutine::runCoroutine()`:

```C++
#include <AceRoutine.h>
using namespace ace_routine;

const int PIN = 2;
const int LED = LED_BUILTIN;
const int LED_ON = HIGH;
const int LED_OFF = LOW;

COROUTINE(blinkLed) {
  COROUTINE_LOOP() {
    digitalWrite(LED, LED_ON);
    COROUTINE_DELAY(100);
    digitalWrite(LED, LED_OFF);
    COROUTINE_DELAY(500);
  }
}

COROUTINE(printHelloWorld) {
  COROUTINE_LOOP() {
    Serial.print(F("Hello, "));
    Serial.flush();
    COROUTINE_DELAY(1000);
    Serial.println(F("World"));
    COROUTINE_DELAY(4000);
  }
}

COROUTINE(printProfiling) {
  COROUTINE_LOOP() {
    LogBinTableRenderer::printTo(
        Serial, 3 /*startBin*/, 14 /*endBin*/, false /*clear*/);
    LogBinJsonRenderer::printTo(
        Serial, 3 /*startBin*/, 14 /*endBin*/);

    COROUTINE_DELAY(5000);
  }
}

LogBinProfiler profiler1;
LogBinProfiler profiler2;
LogBinProfiler profiler3;

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  pinMode(LED, OUTPUT);
  pinMode(PIN, INPUT);

  // Coroutine names can be either C-string or F-string.
  blinkLed.setName("blinkLed");
  readPin.setName(F("readPin"));

  // Manually attach the profilers to the coroutines.
  blinkLed.setProfiler(&profiler1);
  readPin.setProfiler(&profiler2);
  printProfiling.setProfiler(&profiler3);
}

void loop() {
  blinkLed.runCoroutineWithProfiling();
  printHelloWorld.runCoroutineWithProfiling();
  printProfiling.runCoroutineWithProfiler();
}
```

Every 5 seconds, the `printProfiling` coroutine will print the profiling
information in 2 formats on the `Serial` port:

* a formatted table through the `LogBinTableRenderer`
* a JSON object using the `LogBinJsonRenderer`

```
name         <16us <32us <64us<128us<256us<512us  <1ms  <2ms  <4ms  <8ms    >>
0x1DB        16921 52650     0     0     0     0     0     0     0     0     1
readPin      65535  1189     0     0     0     0     0     0     0     0     0
blinkLed     65535   830     0     0     0     0     0     0     0     0     0
{
"0x1DB":[16921,52650,0,0,0,0,0,0,0,0,1],
"readPin":[65535,1189,0,0,0,0,0,0,0,0,0],
"blinkLed":[65535,830,0,0,0,0,0,0,0,0,0]
}
```

<a name="HelloSchedulerWithProfiler"></a>
### HelloSchedulerWithProfiler

The [HelloSchedulerWithProfiler.ino](examples/HelloSchedulerWithProfiler) sketch
implements the same thing as `HelloCoroutineWithProfiler` using 2 techniques to
handle more than a handful of coroutines:

* use `LogBinProfiler::createProfilers()` to automatically create the profilers
  on the heap and assign them to all coroutines
* use `CoroutineScheduler::loopWithProfiler()` method instead of the
  `CoroutineScheduler::loop()` method.

```C++
#include <AceRoutine.h>
using namespace ace_routine;

const int PIN = 2;
const int LED = LED_BUILTIN;
const int LED_ON = HIGH;
const int LED_OFF = LOW;

COROUTINE(blinkLed) {
  COROUTINE_LOOP() {
    digitalWrite(LED, LED_ON);
    COROUTINE_DELAY(100);
    digitalWrite(LED, LED_OFF);
    COROUTINE_DELAY(500);
  }
}

COROUTINE(printHelloWorld) {
  COROUTINE_LOOP() {
    Serial.print(F("Hello, "));
    Serial.flush();
    COROUTINE_DELAY(1000);
    Serial.println(F("World"));
    COROUTINE_DELAY(4000);
  }
}

COROUTINE(printProfiling) {
  COROUTINE_LOOP() {
    LogBinTableRenderer::printTo(
        Serial, 3 /*startBin*/, 14 /*endBin*/, false /*clear*/);
    LogBinJsonRenderer::printTo(
        Serial, 3 /*startBin*/, 14 /*endBin*/);

    COROUTINE_DELAY(5000);
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  pinMode(LED, OUTPUT);
  pinMode(PIN, INPUT);

  // Coroutine names can be either C-string or F-string.
  blinkLed.setName("blinkLed");
  readPin.setName(F("readPin"));

  // Create profilers on the heap and attach them to all coroutines.
  LogBinProfiler::createProfilers();

  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loopWithProfiler();
}
```

The `printProfiling` coroutine will print the same information as before every 5
seconds:

```
name         <16us <32us <64us<128us<256us<512us  <1ms  <2ms  <4ms  <8ms    >>
0x1DB        16921 52650     0     0     0     0     0     0     0     0     1
readPin      65535  1189     0     0     0     0     0     0     0     0     0
blinkLed     65535   830     0     0     0     0     0     0     0     0     0
{
"0x1DB":[16921,52650,0,0,0,0,0,0,0,0,1],
"readPin":[65535,1189,0,0,0,0,0,0,0,0,0],
"blinkLed":[65535,830,0,0,0,0,0,0,0,0,0]
}
```

<a name="Installation"></a>
## Installation

The latest stable release is available in the Arduino IDE Library Manager. Two
libraries need to be installed as of v1.5.0:

* Search for "AceRoutine". Click Install.
* It should automatically install (or prompt you to install) the "AceCommon"
  library.

The development version can be installed by cloning the following git repos:

* AceRoutine (https://github.com/bxparks/AceRoutine)
* AceCommon (https://github.com/bxparks/AceCommon)

You can copy these directories to the `./libraries` directory used by the
Arduino IDE. (You should see 2 directories,  named `./libraries/AceRoutine` and
`./libraries/AceCommon`). Or you can create symlinks from `/.libraries` to these
directories.

The `develop` branch contains the latest working version.
The `master` branch contains the stable release.

<a name="SourceCode"></a>
### Source Code

The source files are organized as follows:

* `src/AceRoutine.h` - main header file
* `src/ace_routine/` - implementation files
* `src/ace_routine/testing/` - internal testing files
* `tests/` - unit tests which depend on
  [AUnit](https://github.com/bxparks/AUnit)
* `examples/` - example programs

<a name="Documentation"></a>
## Documentation

* [README.md](README.md) - this file
* [Doxygen docs](https://bxparks.github.io/AceRoutine/html) published on GitHub
  Pages
* [USER_GUIDE.md](USER_GUIDE.md)

<a name="Examples"></a>
### Examples

The following programs are provided under the `examples` directory:

* Beginner Examples
    * [HelloCoroutine.ino](examples/HelloCoroutine)
    * [HelloScheduler.ino](examples/HelloScheduler): same as `HelloCoroutine`
      except using the `CoroutineScheduler` instead of manually running the
      coroutines
    * [HelloManualCoroutine.ino](examples/HelloManualCoroutine): same as
      `HelloCoroutine` except the `Coroutine` subclasses and instances are
      created and registered manually
    * [HelloCoroutineWithProfiler.ino](examples/HelloCoroutineWithProfiler)
    * [HelloSchedulerWithProfiler.ino](examples/HelloSchedulerWithProfiler):
      same as `HelloCoroutineWithProfiler` using `CoroutineScheduler`
* Intermediate Examples
    * [BlinkSlowFastRoutine.ino](examples/BlinkSlowFastRoutine): use coroutines
      to read a button and control how the LED blinks
    * [BlinkSlowFastManualRoutine.ino](examples/BlinkSlowFastManualRoutine):
      same as BlinkSlowFastRoutine but using manual `Coroutine` subclasses
    * [CountAndBlink.ino](examples/CountAndBlink): count and blink at the same
      time
    * [Delay.ino](examples/Delay): validate the `COROUTINE_DELAY()` macro
* Advanced Examples
    * [SoundManager](examples/SoundManager): Use a sound manager coroutine to
      control the sounds made by a sound generator coroutine, using the
      `reset()` function to interrupt the sound generator.
* Channels (experimental)
    * [Pipe.ino](examples/Pipe): uses a `Channel` to allow a Writer to send
      messages to a Reader through a "pipe" (unfinished)
    * [Task.ino](examples/Pipe): uses a `Channel` to allow a Writer to send
      messages to a Reade (unfinished)
    * a working example of Channels can be found in the
      [CommandLineInterface](https://github.com/bxparks/AceUtils/blob/develop/src/cli/README.md)
      package in the AceUtils library (https://github.com/bxparks/AceUtils).
* Benchmarks
    * Internal programs to extract various CPU and memory benchmarks.
    * [AutoBenchmark.ino](examples/AutoBenchmark): performs CPU benchmarking
    * [MemoryBenchmark.ino](examples/MemoryBenchmark): determines the flash and
      static memory consumptions of certain AceRoutine features
    * [ChannelBenchmark.ino](examples/ChannelBenchmark): determines the amount
      of CPU overhead of a `Channel` by using 2 coroutines to ping-pong an
      integer across 2 channels

<a name="Comparisons"></a>
## Comparisons to Other Multitasking Libraries

There are several interesting and useful multithreading libraries for Arduino.
I'll divide the libraries in to 2 camps:
* tasks
* threads or coroutines

### Task Managers

Task managers run a set of tasks. They do not provide a way to resume
execution after `yield()` or `delay()`.

* [JMScheduler](https://github.com/jmparatte/jm_Scheduler)
* [TaskScheduler](https://github.com/arkhipenko/TaskScheduler)
* [ArduinoThread](https://github.com/ivanseidel/ArduinoThread)

### Threads or Coroutines

In order of increasing complexity, here are some libraries that provide
broader abstraction of threads or coroutines:

* [Littlebits coroutines](https://github.com/renaudbedard/littlebits-arduino/tree/master/Libraries/Coroutines)
    * Implemented using Duff's Device which means that nested
      `switch` statements don't work.
    * The scheduler has a fixed queue size.
    * The context structure is exposed.
* [Arduino-Scheduler](https://github.com/mikaelpatel/Arduino-Scheduler)
    * Overrides the system's `yield()` for a seamless experience.
    * Uses `setjmp()` and `longjmp()`.
    * Provides an independent stack to each coroutine whose size is configurable
      at runtime (defaults to 128 for AVR, 1024 for 32-bit processors).
    * ESP8266 or ESP32 not supported (or at least I did not see it).
* [Cosa framework](https://github.com/mikaelpatel/Cosa)
    * A full-featured, alternative development environment using the Arduino
      IDE, but not compatible with the Arduino API or libraries.
    * Installs as a separate "core" using the Board Manager.
    * Includes various ways of multi-tasking (Events, ProtoThreads, Threads,
      Coroutines).
    * The `<ProtoThread.h>` library in the Cosa framework uses basically the
      same technique as this `AceRoutine` library.

### Comparing AceRoutine to Other Libraries

The AceRoutine library falls in the "Threads or Coroutines" camp. The
inspiration for this library came from
[ProtoThreads](http://dunkels.com/adam/pt) and
[Coroutines in C](https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html)
where an incredibly brilliant and ugly technique called
[Duff's Device](https://en.wikipedia.org/wiki/Duff%27s_device)
is used to perform labeled `goto` statements inside the "coroutines" to resume
execution from the point of the last `yield()` or `delay()`. It occurred to me
that I could make the code a lot cleaner and easier to use in a number of ways:

* Instead of using *Duff's Device*, I could use the GCC language extension
  called the
  [computed goto](https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html).
  I would lose ANSI C compatbility, but all of the Arduino platforms
  (AVR, Teensy, ESP8266, ESP32) use the GCC compiler and the Arduino
  software already relies on GCC-specific features (e.g. flash strings using
  `PROGMEM` attribute). In return, `switch` statements would work
  inside the coroutines, which wasn't possible using the Duff's Device.
* Each "coroutine" needs to keep some small number of context variables.
  In the C language, this needs to be passed around using a `struct`. It
  occurred to me that in C++, we could make the context variables almost
  disappear by making "coroutine" an instance of a class and moving the context
  variables into the member variables.
* I could use C-processor macros similar to the ones used in
  [AUnit](https://github.com/bxparks/AUnit) to hide much of the boilerplate code
  and complexity from the user

I looked around to see if there already was a library that implemented these
ideas and I couldn't find one. However, after writing most of this library, I
discovered that my implementation was very close to the `<ProtoThread.h>` module
in the Cosa framework. It was eerie to see how similar the 2 implementations had
turned out at the lower level. I think the AceRoutine library has a couple of
advantages:
* it provides additional macros (i.e. `COROUTINE()` and `EXTERN_COROUTINE()`) to
  eliminate boilerplate code, and
* it is a standalone Arduino library that does not depend on a larger
  framework.

<a name="ResourceConsumption"></a>
## Resource Consumption

<a name="StaticMemory"></a>
### Static Memory

All objects are statically allocated (i.e. not heap or stack).

On 8-bit processors (AVR Nano, Uno, etc):

```
sizeof(Coroutine): 16
sizeof(CoroutineScheduler): 2
sizeof(Channel<int>): 5
sizeof(LogBinProfiler): 66
sizeof(LogBinTableRenderer): 1
sizeof(LogBinJsonRenderer): 1
```

On 32-bit processors (e.g. Teensy ARM, ESP8266, ESP32):

```
sizeof(Coroutine): 28
sizeof(CoroutineScheduler): 4
sizeof(Channel<int>): 12
sizeof(LogBinProfiler): 68
sizeof(LogBinTableRenderer): 1
sizeof(LogBinJsonRenderer): 1
```

The `CoroutineScheduler` consumes only 2 bytes (8-bit processors) or 4 bytes
(32-bit processors) of static memory no matter how many coroutines are created.
That's because it depends on a singly-linked list whose pointers live on the
`Coroutine` object, not in the `CoroutineScheduler`. But using the
`CoroutineScheduler::loop()` instead of calling `Coroutine::runCoroutine()`
directly increases flash memory usage by 70-100 bytes.

The `Channel` object requires 2 copies of the parameterized `<T>` type so its
size is equal to `1 + 2 * sizeof(T)`, rounded to the nearest memory alignment
boundary (i.e. a total of 12 bytes for a 32-bit processor).

<a name="FlashMemory"></a>
### Flash Memory

The [examples/MemoryBenchmark](examples/MemoryBenchmark) program gathers
flash and memory consumption numbers for various boards (AVR, ESP8266, ESP32,
etc) for a handful of AceRoutine features. Here are some highlights:

**Arduino Nano (8-bits)**

```
+--------------------------------------------------------------------+
| functionality                         |  flash/  ram |       delta |
|---------------------------------------+--------------+-------------|
| Baseline                              |   1616/  186 |     0/    0 |
|---------------------------------------+--------------+-------------|
| One Delay Function                    |   1664/  188 |    48/    2 |
| Two Delay Functions                   |   1726/  190 |   110/    4 |
|---------------------------------------+--------------+-------------|
| One Coroutine (millis)                |   1804/  212 |   188/   26 |
| Two Coroutines (millis)               |   1998/  236 |   382/   50 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                |   1776/  212 |   160/   26 |
| Two Coroutines (micros)               |   1942/  236 |   326/   50 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               |   1904/  212 |   288/   26 |
| Two Coroutines (seconds)              |   2130/  236 |   514/   50 |
|---------------------------------------+--------------+-------------|
| One Coroutine, Profiler               |   1874/  212 |   258/   26 |
| Two Coroutines, Profiler              |   2132/  236 |   516/   50 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (millis)     |   1928/  214 |   312/   28 |
| Scheduler, Two Coroutines (millis)    |   2114/  238 |   498/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     |   1900/  214 |   284/   28 |
| Scheduler, Two Coroutines (micros)    |   2058/  238 |   442/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    |   2028/  214 |   412/   28 |
| Scheduler, Two Coroutines (seconds)   |   2246/  238 |   630/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      |   1978/  214 |   362/   28 |
| Scheduler, Two Coroutines (setup)     |   2264/  238 |   648/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  |   1956/  214 |   340/   28 |
| Scheduler, Two Coroutines (man setup) |   2250/  238 |   634/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine, Profiler    |   1992/  214 |   376/   28 |
| Scheduler, Two Coroutines, Profiler   |   2178/  238 |   562/   52 |
|---------------------------------------+--------------+-------------|
| Scheduler, LogBinProfiler             |   2112/  286 |   496/  100 |
| Scheduler, LogBinTableRenderer        |   3514/  304 |  1898/  118 |
| Scheduler, LogBinJsonRenderer         |   3034/  308 |  1418/  122 |
|---------------------------------------+--------------+-------------|
| Blink Function                        |   1948/  189 |   332/    3 |
| Blink Coroutine                       |   2118/  212 |   502/   26 |
+--------------------------------------------------------------------+
```

**ESP8266 (32-bits)**

```
+--------------------------------------------------------------------+
| functionality                         |  flash/  ram |       delta |
|---------------------------------------+--------------+-------------|
| Baseline                              | 264981/27984 |     0/    0 |
|---------------------------------------+--------------+-------------|
| One Delay Function                    | 265045/27992 |    64/    8 |
| Two Delay Functions                   | 265109/27992 |   128/    8 |
|---------------------------------------+--------------+-------------|
| One Coroutine (millis)                | 265177/28028 |   196/   44 |
| Two Coroutines (millis)               | 265337/28060 |   356/   76 |
|---------------------------------------+--------------+-------------|
| One Coroutine (micros)                | 265209/28028 |   228/   44 |
| Two Coroutines (micros)               | 265369/28060 |   388/   76 |
|---------------------------------------+--------------+-------------|
| One Coroutine (seconds)               | 265209/28028 |   228/   44 |
| Two Coroutines (seconds)              | 265385/28060 |   404/   76 |
|---------------------------------------+--------------+-------------|
| One Coroutine, Profiler               | 265257/28028 |   276/   44 |
| Two Coroutines, Profiler              | 265433/28060 |   452/   76 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (millis)     | 265241/28036 |   260/   52 |
| Scheduler, Two Coroutines (millis)    | 265385/28060 |   404/   76 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (micros)     | 265257/28036 |   276/   52 |
| Scheduler, Two Coroutines (micros)    | 265401/28060 |   420/   76 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (seconds)    | 265257/28036 |   276/   52 |
| Scheduler, Two Coroutines (seconds)   | 265417/28060 |   436/   76 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (setup)      | 265273/28036 |   292/   52 |
| Scheduler, Two Coroutines (setup)     | 265433/28060 |   452/   76 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine (man setup)  | 265257/28036 |   276/   52 |
| Scheduler, Two Coroutines (man setup) | 265433/28060 |   452/   76 |
|---------------------------------------+--------------+-------------|
| Scheduler, One Coroutine, Profiler    | 265321/28036 |   340/   52 |
| Scheduler, Two Coroutines, Profiler   | 265449/28060 |   468/   76 |
|---------------------------------------+--------------+-------------|
| Scheduler, LogBinProfiler             | 265465/28100 |   484/  116 |
| Scheduler, LogBinTableRenderer        | 267381/28100 |  2400/  116 |
| Scheduler, LogBinJsonRenderer         | 266789/28104 |  1808/  120 |
|---------------------------------------+--------------+-------------|
| Blink Function                        | 265669/28064 |   688/   80 |
| Blink Coroutine                       | 265801/28100 |   820/  116 |
+--------------------------------------------------------------------+
```

Comparing `Blink Function` and `Blink Coroutine` is probably the most
fair comparison, because they implement the exact same functionality. The code
is given in
[Comparison To NonBlocking Function](USER_GUIDE.md#ComparisonToNonBlockingFunction).
The `Blink Function` implements the asymmetric blink (HIGH and LOW having
different durations) functionality using a simple, non-blocking function with an
internal `prevMillis` static variable. The `Blink Coroutine` implements the
same logic using a `Coroutine`. The `Coroutine` version is far more readable and
maintainable, with only about 220 additional bytes of flash on AVR, and 130
bytes on an ESP8266. In many situations, the increase in flash memory size may
be worth paying to get easier code maintenance.

<a name="CPU"></a>
### CPU

See [examples/AutoBenchmark](examples/AutoBenchmark). Here are 2 samples:

Arduino Nano:

```
+---------------------------------+--------+-------------+--------+
| Functionality                   |  iters | micros/iter |   diff |
|---------------------------------+--------+-------------+--------|
| EmptyLoop                       |  10000 |       1.700 |  0.000 |
|---------------------------------+--------+-------------+--------|
| DirectScheduling                |  10000 |       2.900 |  1.200 |
| DirectSchedulingWithProfiler    |  10000 |       5.700 |  4.000 |
|---------------------------------+--------+-------------+--------|
| CoroutineScheduling             |  10000 |       7.100 |  5.400 |
| CoroutineSchedulingWithProfiler |  10000 |       9.300 |  7.600 |
+---------------------------------+--------+-------------+--------+
```

ESP8266:

```
+---------------------------------+--------+-------------+--------+
| Functionality                   |  iters | micros/iter |   diff |
|---------------------------------+--------+-------------+--------|
| EmptyLoop                       |  10000 |       0.200 |  0.000 |
|---------------------------------+--------+-------------+--------|
| DirectScheduling                |  10000 |       0.500 |  0.300 |
| DirectSchedulingWithProfiler    |  10000 |       0.800 |  0.600 |
|---------------------------------+--------+-------------+--------|
| CoroutineScheduling             |  10000 |       0.900 |  0.700 |
| CoroutineSchedulingWithProfiler |  10000 |       1.100 |  0.900 |
+---------------------------------+--------+-------------+--------+
```

<a name="SystemRequirements"></a>
## System Requirements

<a name="Hardware"></a>
### Hardware

**Tier 1: Fully Supported**

These boards are tested on each release:

* Arduino Nano (16 MHz ATmega328P)
* SparkFun Pro Micro (16 MHz ATmega32U4)
* STM32 Blue Pill (STM32F103C8, 72 MHz ARM Cortex-M3)
* NodeMCU 1.0 (ESP-12E module, 80 MHz ESP8266)
* WeMos D1 Mini (ESP-12E module, 80 MHz ESP8266)
* ESP32 dev board (ESP-WROOM-32 module, 240 MHz dual core Tensilica LX6)
* Teensy 3.2 (96 MHz ARM Cortex-M4)

**Tier 2: Should work**

These boards should work, but they are not tested frequently by me, or I don't
own the specific hardware so they were tested by a community member:

* ATtiny85 (8 MHz ATtiny85)
* Arduino Pro Mini (16 MHz ATmega328P)
* Mini Mega 2560 (Arduino Mega 2560 compatible, 16 MHz ATmega2560)
* Teensy LC (48 MHz ARM Cortex-M0+)
* [Adafruit nRF52 Boards](https://github.com/adafruit/Adafruit_nRF52_Arduino)
    * [Circuit Playground Bluefruit](https://www.adafruit.com/product/4333)
      tested by a community member

**Tier 3: May work, but not supported**

* SAMD21 M0 Mini (48 MHz ARM Cortex-M0+)
    * Arduino-branded SAMD21 boards use the ArduinoCore-API, so are explicitly
      blacklisted. See below.
    * Other 3rd party SAMD21 boards *may* work using the SparkFun SAMD core.
    * However, as of SparkFun SAMD Core v1.8.6 and Arduino IDE 1.8.19, I can no
      longer upload binaries to these 3rd party boards due to errors.
    * Therefore, third party SAMD21 boards are now in this new Tier 3 category.
    * This library may work on these boards, but I can no longer support them.

**Tier Blacklisted**

The following boards are *not* supported and are explicitly blacklisted to allow
the compiler to print useful error messages instead of hundreds of lines of
compiler errors:

* Any platform using the ArduinoCore-API
  (https://github.com/arduino/ArduinoCore-api). For example:
    * Nano Every
    * MKRZero
    * Raspberry Pi Pico RP2040

<a name="ToolChain"></a>
### Tool Chain

This library was developed and tested using:
* [Arduino IDE 1.8.19](https://www.arduino.cc/en/Main/Software)
* [Arduino CLI 0.19.2](https://arduino.github.io/arduino-cli)
* [Arduino AVR Boards 1.8.4](https://github.com/arduino/ArduinoCore-avr)
* [Arduino SAMD Boards 1.8.9](https://github.com/arduino/ArduinoCore-samd)
* [SparkFun AVR Boards 1.1.13](https://github.com/sparkfun/Arduino_Boards)
* [SparkFun SAMD Boards 1.8.6](https://github.com/sparkfun/Arduino_Boards)
* [STM32duino 2.2.0](https://github.com/stm32duino/Arduino_Core_STM32)
* [ESP8266 Arduino 3.0.2](https://github.com/esp8266/Arduino)
* [ESP32 Arduino 2.0.2](https://github.com/espressif/arduino-esp32)
* [Teensyduino 1.56](https://www.pjrc.com/teensy/td_download.html)
* [Adafruit nRF52 1.3.0](https://github.com/adafruit/Adafruit_nRF52_Arduino)

This library is *not* compatible with:

* Any platform using the
  [ArduinoCore-API](https://github.com/arduino/ArduinoCore-api), for example:
    * [Arduino megaAVR](https://github.com/arduino/ArduinoCore-megaavr/)
    * [MegaCoreX](https://github.com/MCUdude/MegaCoreX)
    * [Arduino SAMD Boards >=1.8.10](https://github.com/arduino/ArduinoCore-samd)

It should work with [PlatformIO](https://platformio.org/) but I have
not tested it.

The library works on Linux or MacOS (using both g++ and clang++ compilers) using
the [EpoxyDuino](https://github.com/bxparks/EpoxyDuino) emulation layer.

<a name="OperatingSystem"></a>
### Operating System

I use Ubuntu 20.04 for the vast majority of my development. I expect that the
library will work fine under MacOS and Windows, but I have not explicitly tested
them.

<a name="License"></a>
## License

[MIT License](https://opensource.org/licenses/MIT)

<a name="FeedbackAndSupport"></a>
## Feedback and Support

If you have any questions, comments, or feature requests for this library,
please use the [GitHub
Discussions](https://github.com/bxparks/AceRoutine/discussions) for this
project. If you have bug reports, please file a ticket in [GitHub
Issues](https://github.com/bxparks/AceRoutine/issues). Feature requests should
go into Discussions first because they often have alternative solutions which
are useful to remain visible, instead of disappearing from the default view of
the Issue tracker after the ticket is closed.

Please refrain from emailing me directly unless the content is sensitive. The
problem with email is that I cannot reference the email conversation when other
people ask similar questions later.

<a name="Authors"></a>
## Authors

Created by Brian T. Park (brian@xparks.net).
