# AceRoutine

[![AUnit Tests](https://github.com/bxparks/AceRoutine/actions/workflows/aunit_tests.yml/badge.svg)](https://github.com/bxparks/AceRoutine/actions/workflows/aunit_tests.yml)

**New**: [GitHub Discussions](https://github.com/bxparks/AceRoutine/discussions)
for this project is now active! Let's use that for general support questions,
and reserve the [GitHub Issues](https://github.com/bxparks/AceRoutine/issues)
section for bugs and feature requests.

A low-memory, fast-switching, cooperative multitasking library using
stackless coroutines on Arduino platforms.

This library is an implementation of the
[ProtoThreads](http://dunkels.com/adam/pt) library for the
Arduino platform. It emulates a stackless coroutine that can suspend execution
using a `yield()` or `delay()` functionality to allow other coroutines to
execute. When the scheduler makes it way back to the original coroutine, the
execution continues right after the `yield()` or `delay()`.

There are only 3 classes in this library:
* `Coroutine` class provides the context variables for all coroutines,
* `CoroutineScheduler` class optionally handles the scheduling,
* `Channel` class allows coroutines to send messages to each other. This is
  an experimental feature whose API and feature may change considerably
  in the future.

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
* `COROUTINE_LOOP()`: convenience macro that loops forever
* `COROUTINE_CHANNEL_WRITE(channel, value)`: writes a value to a `Channel`
* `COROUTINE_CHANNEL_READ(channel, value)`: reads a value from a `Channel`

Here are some of the compelling features of this library compared to
others (in my opinion of course):
* low memory usage
    * each coroutine consumes only 15 bytes of RAM on 8-bit processors (AVR) and
      28 bytes on 32-bit processors (ARM, ESP8266, ESP32)
    * the `CoroutineScheduler` consumes only 2 bytes (8-bit) or 4 bytes (32-bit)
      no matter how many coroutines are active
* extremely fast context switching
    * ~5.3 microseconds on a 16 MHz ATmega328P
    * ~2.5 microseconds on a 48 MHz SAMD21
    * ~1.8 microseconds on a 72 MHz STM32
    * ~1.5 microseconds on a 80 MHz ESP8266
    * ~0.4 microseconds on a 240 MHz ESP32
    * ~1.0 microseconds on 96 MHz Teensy 3.2 (depending on compiler settings)
* uses the [computed goto](https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html)
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
* Coroutines are currently designed to be statically allocated, not dynamically
  created and destroyed. This is mostly because dynamic memory allocation
  on an 8-bit microcontroller with 2kB of RAM should probably be avoided.
  Dynamically created coroutines may be added in the future for 32-bit
  microcontrollers which have far more memory.
* A `Channel` is an experimental feature and has limited features. It is
  currently an unbuffered, synchronized channel. It can be used by only one
  reader and one writer.

After I had completed most of this library, I discovered that I had essentially
reimplemented the `<ProtoThread.h>` library in the
[Cosa framework](https://github.com/mikaelpatel/Cosa). The difference is that
AceRoutine is a self-contained library that works on any platform supporting the
Arduino API (AVR, Teensy, ESP8266, ESP32, etc), and it provides a handful of
additional macros that can reduce boilerplate code.

**Version**: 1.3 (2021-05-27)

**Changelog**: [CHANGELOG.md](CHANGELOG.md)

**Important Change in v1.1**: This library now depends on the AceCommon library
(https://github.com/bxparks/AceCommon). See the *Installation* section below.

## Table of Contents

* [Hello Coroutines](#HelloCoroutines)
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
* [License](#LicenseSystemRequirements)
* [Feedback and Support](#FeedbackAndSupport)
* [Authors](#Authors)

<a name="HelloCoroutines"></a>
## Hello Coroutines

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
`COROUTINE()` macro, which eliminates the need to itemize your coroutines in
the `loop()` method manually.

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

  blinkLed.setupCoroutine(F("blinkLed"));
  printHelloWorld.setupCoroutine(F("printHelloWorld"));
  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
```

<a name="Installation"></a>
## Installation

The latest stable release is available in the Arduino IDE Library Manager. Two
libraries need to be installed since v1.1:

* Search for "AceRoutine". Click Install.
* Search for "AceCommon". Click Install.

The development version can be installed by cloning the 2 git repos:

* AceRoutine (https://github.com/bxparks/AceRoutine)
* AceCommon  (https://github.com/bxparks/AceCommon)

You can copy these directories to the `./libraries` directory used by the
Arduino IDE. (The result is a directory named `./libraries/AceRoutine` and
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
* Intermediate Examples
    * [BlinkSlowFastRoutine.ino](examples/BlinkSlowFastRoutine): use coroutines
      to read a button and control how the LED blinks
    * [BlinkSlowFastManualRoutine.ino](examples/BlinkSlowFastManualRoutine):
      same as BlinkSlowFastRoutine but using manual `Coroutine` subclasses
    * [CountAndBlink.ino](examples/CountAndBlink): count and blink at the same
      time
    * [Delay.ino](examples/Delay): validate the delay macros
      `COROUTINE_DELAY()` and `COROUTINE_DELAY_MICROS()`
* Advanced Examples
    * [SoundManager](examples/SoundManager): Use a sound manager coroutine to
      control the sounds made by a sound generator coroutine, using the
      `reset()` function to interrupt the sound generator.
* Channels
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

* 8-bit processors (AVR Nano, UNO, etc):
    * `sizeof(Coroutine)`: 15
    * `sizeof(CoroutineScheduler)`: 2
    * `sizeof(Channel<int>)`: 5
* 32-bit processors (e.g. Teensy ARM, ESP8266, ESP32)
    * `sizeof(Coroutine)`: 28
    * `sizeof(CoroutineScheduler)`: 4
    * `sizeof(Channel<int>)`: 12

In other words, you can create 100 `Coroutine` instances and they would use only
1400 bytes of static RAM on an 8-bit AVR processor.

The `CoroutineScheduler` consumes only 2 bytes of memory no matter how many
coroutines are created. That's because it depends on a singly-linked list whose
pointers live on the `Coroutine` object, not in the `CoroutineScheduler`. But
the code for the class increases flash memory usage by about 150 bytes.

The `Channel` object requires 2 copies of the parameterized `<T>` type so its
size is equal to `1 + 2 * sizeof(T)`, rounded to the nearest memory alignment
boundary (i.e. a total of 12 bytes for a 32-bit processor).

<a name="FlashMemory"></a>
### Flash Memory

The [examples/MemoryBenchmark](examples/MemoryBenchmark) program gathers
flash and memory consumption numbers for various boards (AVR, ESP8266, ESP32,
etc) for a handful of AceRoutine features. Here are some highlights:

* AVR (e.g. Nano)
    * 1 Coroutine: 500 bytes
    * 2 Coroutines: 700 bytes
    * `CoroutineScheduler()` + 1 Coroutine: 600 bytes
    * `CoroutineScheduler()` + 2 Coroutines: 750 bytes
* ESP8266
    * 1 Coroutine: 350 bytes
    * 2 Coroutines: 600 bytes
    * `CoroutineScheduler()` + 1 Coroutine: 640 bytes
    * `CoroutineScheduler()` + 2 Coroutines: 760 bytes

<a name="CPU"></a>
### CPU

See [examples/AutoBenchmark](examples/AutoBenchmark). In summary, the overhead
of AceRoutine context switching is about 6 micros on an 8-bit AVR, to as low as
0.33 micros on a 32-bit ESP32.

<a name="SystemRequirements"></a>
## System Requirements

### Hardware

The library has been extensively tested on the following boards:

* Arduino Nano clone (16 MHz ATmega328P)
* SparkFun Pro Micro clone (16 MHz ATmega32U4)
* SAMD21 M0 Mini (48 MHz ARM Cortex-M0+)
* STM32 Blue Pill (STM32F103C8, 72 MHz ARM Cortex-M3)
* NodeMCU 1.0 (ESP-12E module, 80 MHz ESP8266)
* WeMos D1 Mini (ESP-12E module, 80 MHz ESP8266)
* ESP32 dev board (ESP-WROOM-32 module, 240 MHz dual core Tensilica LX6)
* Teensy 3.2 (96 MHz ARM Cortex-M4)

I will occasionally test on the following hardware as a sanity check:

* Arduino Pro Mini clone (16 MHz ATmega328P)
* Mini Mega 2560 (Arduino Mega 2560 compatible, 16 MHz ATmega2560)
* Teensy LC (48 MHz ARM Cortex-M0+)

### Tool Chain

This library was developed and tested using:
* [Arduino IDE 1.8.13](https://www.arduino.cc/en/Main/Software)
* [Arduino CLI 0.14.0](https://arduino.github.io/arduino-cli)
* [Arduino AVR Boards 1.8.3](https://github.com/arduino/ArduinoCore-avr)
* [Arduino SAMD Boards 1.8.9](https://github.com/arduino/ArduinoCore-samd)
* [SparkFun AVR Boards 1.1.13](https://github.com/sparkfun/Arduino_Boards)
* [SparkFun SAMD Boards 1.8.1](https://github.com/sparkfun/Arduino_Boards)
* [STM32duino 1.9.0](https://github.com/stm32duino/Arduino_Core_STM32)
* [ESP8266 Arduino 2.7.4](https://github.com/esp8266/Arduino)
* [ESP32 Arduino 1.0.4](https://github.com/espressif/arduino-esp32)
* [Teensydino 1.53](https://www.pjrc.com/teensy/td_download.html)

It should work with [PlatformIO](https://platformio.org/) but I have
not tested it.

The library works on Linux or MacOS (using both g++ and clang++ compilers) using
the [EpoxyDuino](https://github.com/bxparks/EpoxyDuino) emulation layer.

### Operating System

I use Ubuntu 18.04 and 20.04 for most of my development and sometimes do sanity
checks on MacOS 10.14.5.

<a name="License"></a>
## License

[MIT License](https://opensource.org/licenses/MIT)

<a name="FeedbackAndSupport"></a>
## Feedback and Support

If you find this library useful, consider starring this project on GitHub. The
stars will let me prioritize the more popular libraries over the less popular
ones.

If you have any questions, comments and other support questions about how to
use this library, please use the
[GitHub Discussions](https://github.com/bxparks/AceRoutine/discussions)
for this project. If you have bug reports or feature requests, please file a
ticket in [GitHub Issues](https://github.com/bxparks/AceRoutine/issues).
I'd love to hear about how this software and its documentation can be improved.
I can't promise that I will incorporate everything, but I will give your ideas
serious consideration.

Please refrain from emailing me directly unless the content is sensitive. The
problem with email is that I cannot reference the email conversation when other
people ask similar questions later.

<a name="Authors"></a>
## Authors

Created by Brian T. Park (brian@xparks.net).
