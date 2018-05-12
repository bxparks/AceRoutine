# AceRoutine

A low-memory footprint, fast-switching, cooperative multitasking library using
stackless routines on Arduino platforms.

Version: (2018-05-11)

## Summary

This library is an implementation of the
[ProtoThreads](http://dunkels.com/adam/pt) library for the
Arduino platform. It emulates a stackless subroutine that can suspend execution
using a `yield()` or `delay()` functionality to allow other routines to execute.
When the scheduler makes it way back to the original routine, the execution
continues right after the `yield()` or `delay()`.

After I had completed most of the library, I discovered that I had essentially
reimplemented the `<ProtoThread.h>` library in the
[Cosa framework](https://github.com/mikaelpatel/Cosa). The difference is that
AceRoutine is a self-contained library that works on any platform supporting the
Arduino API (AVR, Teensy, ESP8266, ESP32, etc).

I could not justify calling these "coroutines" because they cannot be called
aribtrarily, and they do not return any values. They are not really "threads"
either because they are stackless and cannot retain local variables persistently
across multiple invocations. I call them just "routines" in this library, you
can call them "ace routines" if you wish. ("ACE" used to mean something for some
of my other libraries, but it's just a generic marker that doesn't mean anything
right now.)

There are only 2 classes in this library:
* each routine is an instance of a subclass of the `Routine` class,
* the `RoutineScheduler` ojbect handles the scheduling

The library provides a number of macros to help create and manage life cycle
of these routines:
* `ROUTINE()`: defines an instance of `Routine` class or a user-provided
  custom subclass of `Routine`
* `ROUTINE_NAMED()`: defines an instance of `Routine` class that knows its
  human-readable name, needed for unit tests
* `ROUTINE_BEGIN()`: must occur at the start of a routine body
* `ROUTINE_END()`: must occur at the end of the routine body
* `ROUTINE_YIELD()`: yields execution back to the `RoutineScheduler`
* `ROUTINE_AWAIT(condition)`: yield until `condition` become `true`
* `ROUTINE_DELAY(millis)`: yields back execution for `millis`. The `millis`
  parameter is defined as a `uint16_t`.
* `ROUTINE_LOOP()`: convenience macro that loops forever

Here are some of the strong features of this library compared to
others (in my opinion of course):
* extremely low memory usage
    * each routine consumes only 13 bytes of static memory (on AVR) and
      24 bytes on 32-bit processors (ARM, ESP8266, ESP32)
    * the `RoutineScheduler` consumes only 2 (or 4) bytes no matter how
      many routines are active
* extremely fast context switching
    * ~5 microseconds on a 16 MHz ATmega328P
    * ~0.8 microseconds on Teensy 3.2 (depending on compiler settings)
    * ~0.4 microseconds on a ESP32
    * the `RoutineScheduler` simply walks along a singly linked list without
      much overhead
* uses "computed goto" feature of GCC to avoid the
  [Duff's Device](https://en.wikipedia.org/wiki/Duff%27s_device) hack,
  allowing `switch` statemens in the routines
* macros eliminate a lot of boilerplate code and makes the code easy to read
* easy to subclass the base `Routine` class and add additional variables and
  functions
    * macros can support these custom classes easily
* fully unit tested using [AUnit](https://github.com/bxparks/AUnit)

### HelloRoutine

This is the [HelloRoutine.ino](examples/HelloRoutine) sample sketch.

```
#include <AceRoutine.h>
using namespace ace_routine;

const int LED = LED_BUILTIN;
const int LED_ON = HIGH;
const int LED_OFF = LOW;

const int LED_DELAY = 200;

ROUTINE(blinkLed) {
  ROUTINE_LOOP() {
    digitalWrite(LED, LED_ON);
    ROUTINE_DELAY(LED_DELAY);
    digitalWrite(LED, LED_OFF);
    ROUTINE_DELAY(LED_DELAY);
  }
}

ROUTINE(printHello) {
  ROUTINE_BEGIN();

  Serial.print(F("Hello, "));
  ROUTINE_DELAY(1000);

  ROUTINE_END();
}

ROUTINE(printWorld) {
  ROUTINE_BEGIN();

  ROUTINE_AWAIT(printHello.isTerminated());
  Serial.println(F("World!"));

  ROUTINE_END();
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  RoutineScheduler::setup();
}

void loop() {
  RoutineScheduler::loop();
}
```

This prints "Hello, ", then waits one second, and then prints "World!".
At the same time, the LED blinks on and off.

## Installation

The latest stable release will eventually be available in the Arduino IDE
Library Manager. Search for "AceRoutine". Click install. It is not there
yet.

The development version can be installed by cloning the
[GitHub repository](https://github.com/bxparks/AceRoutine), checking out the
`develop` branch, then manually copying over the contents to the `./libraries`
directory used by the Arduino IDE. (The result is a directory named
`./libraries/AceRoutine`.) The `master` branch contains the stable release.

### Source Code

The source files are organized as follows:
* `src/AceRoutine.h` - main header file
* `src/ace_routine/` - implementation files
* `src/ace_routine/testing/` - internal testing files
* `tests/` - unit tests which depend on
  [AUnit](https://github.com/bxparks/AUnit)

### Docs

The [docs/](docs/) directory contains the
[Doxygen docs published on GitHub Pages](https://bxparks.github.io/AceRoutine/html).

### Examples

The following example sketches are provided:

* [HelloRoutine.ino](examples/HelloRoutine)
* [BlinkRoutine.ino](examples/BlinkRoutine): use routines to read a button
  and control how the LED blinks
* [BlinkCustomRoutine.ino](examples/BlinkCustomRoutine): same as BlinkRoutine
  but using a custom `Routine` class
* [CountAndBlink.ino](examples/CountAndBlink): count and blink at the same time
* [AutoBenchmark.ino](examples/AutoBenchmark):
  a program that performs CPU benchmarking

## Usage

### Include Header and Namespace

Only a single header file `AceRoutine.h` is required to use this library.
To prevent name clashes with other libraries that the calling code may use, all
classes are defined in the `ace_routine` namespace. To use the code without
prepending the `ace_routine::` prefix, use the `using` directive:

```
#include <AceRoutine.h>
using namespace ace_routine;
```

### Macros

The following macros are used:
* `ROUTINE()`: defines an instance of `Routine` class or a user-provided
  custom subclass of `Routine`
* `ROUTINE_NAMED()`: defines an instance of `Routine` class that knows its
  human-readable name, needed for unit tests
* `ROUTINE_BEGIN()`: must occur at the start of a routine body
* `ROUTINE_END()`: must occur at the end of the routine body
* `ROUTINE_YIELD()`: yields execution back to the `RoutineScheduler`
* `ROUTINE_AWAIT(condition)`: yield until `condition` become `true`
* `ROUTINE_DELAY(millis)`: yields back execution for `millis`. The `millis`
  parameter is defined as a `uint16_t`.
* `ROUTINE_LOOP()`: convenience macro that loops forever

### Defining Routines

The overall structure looks like this:
```
#include <AceRoutine.h>
using namespace ace_routine;

ROUTINE(oneShotRoutine) {
  ROUTINE_BEGIN();

  ...
  ROUTINE_YIELD();

  ROUTINE_AWAIT(condition);

  ...
  ROUTINE_DELAY(100);

  ...
  ROUTINE_END();
}

ROUTINE(loopingRoutine) {
  ROUTINE_LOOP() {
    ...
    ROUTINE_YIELD();
    ...
  }
}

void setup() {
  Serial.begin(115200); // optional
  while (!Serial); // Leonardo/Micro

  ...
  RoutineScheduler::setup();
  ...
}

void loop() {
  RoutineScheduler::loop();
}
```

### Running and Scheduling

Routines are registered with `RoutineScheduler` when `RoutineScheduler::setup()`
is called. Each call to `RoutineScheduler::loop()` executes one routine in the
list, in a simple round-robin scheduling algorithm. When a routine executes a
`ROUTINE_YIELD()` or a `ROUTINE_DELAY()`, the return context is saved to the
`Routine` instance, execution control returns to the `RoutineScheduler` which
resumes the execution of the next routine in the list.

The order of `Routine` instance list is undefined. This is because the
registration process relies the C++ static initialization (which occurs before
`setup()`), and the order of static initializiation is not defined by the
language spec.

### Stackless Routines

Each routine is stackless, or more accurately, the stack of the routine
is destroyed and recreated on every invocation of the routine. Therefore,
any local variable created on the stack in the routine will not preserve
its value after a `ROUTINE_YIELD()` or a `ROUTINE_DELAY()`.

The easiest way to get around ths problem is to use `static` variables inside
a `ROUTINE()`. Static variables are initialized once and preserve their value
through multiple calls to the function, which is exactly what is needed.

### Yield

`ROUTINE_YIELD()` returns control to the `RoutineSchedule` which is then able to
run another routines. Upon the next iteration, execution continues just after
`ROUTINE_YIELD()`. (Technically, the execution always begins at the top of the
function, but the `ROUTINE_BEGIN()` contains a dispatcher that gives the
illusion that the execution continues further down the function.)

### Delay

`ROUTINE_DELAY(millis)` delays the return of control until `millis` milliseconds
has elapsed. The argument is a `uint16_t`, a 16-bit unsigned integer, which
saves 4 bytes on each instance of `Routine`. However, the drawback is that the
largest value is 65535 milliseconds. To delay for longer, an explicit loop can
be used:
```
ROUTINE(waitThousandSeconds) {
  ROUTINE_BEGIN();
  static i = 0;
  for (i = 0; i < 1000; i++) {
    ROUTINE_DELAY(1000);
  }
  ...
  ROUTINE_END();
}
```
See **For Loop** section below for a description of the for-loop construct.

### Await

`ROUTINE_AWAIT(condition)` yields until the `condition` evaluates to `true`.
This is a convenience macro that is identical to:
```
while(!condition) ROUTINE_YIELD();
```

### One-Shot Routines

A one-shot routine starts the begining, finishes at the end, and permanently
terminates. The code looks like this:
```
ROUTINE(doStraightThrough) {
  ROUTINE_BEGIN();

  ...
  ROUTINE_YIELD();

  ...
  ROUTINE_DELAY(100);

  ...

  ROUTINE_END();
}

```

### Conditional If-Else

Conditional if-statements work as expected with the various macros:
```
ROUTINE(doStraightThrough) {
  ROUTINE_BEGIN();

  if (condition) {
    ...
    ROUTINE_YIELD();
  } else {
    ...
    ROUTINE_DELAY(100);
  }

  ...

  ROUTINE_END();
}
```

### Switch Statements

Unlike some implementations of stackless routines, AceRoutine routines are
compatible with `switch` statements:

```
ROUTINE(doThingsBasedOnSwitchConditions) {
  ROUTINE_BEGIN();
  ...

  switch (value) {
    case VAL_A:
      ...
      ROUTINE_YIELD();
      break;
    case VAL_B:
      ...
      ROUTINE_DELAY(100);
      break;
    default:
      ...
  }
  ...
  ROUTINE_END();
}
```

### For Loops

You cannot use a local variable in the `for-loop` because the variable counter
would be created on the stack, and get destroyed as soon as you `YIELD()` or
`DELAY()`. However, a reasonable solution is to use `static` variables. For
example:

```
ROUTINE(countToTen) {
  ROUTINE_BEGIN();
  static i = 0;
  for (i = 0; i < 10; i++) {
    ...
    ROUTINE_DELAY(100);
    ...
  }
  ROUTINE_END();
}
```

### While Loops

You can write a routine that loops while certain condition is valid like this,
just like you would normally, except that you call the `ROUTINE_YIELD()`
macro to cooperatively allow other routines to execute.

```
ROUTINE(loopWhileCondition) {
  ROUTINE_BEGIN();
  while (condition) {
    ...
    ROUTINE_YIELD();
    ...
  }
  ROUTINE_END();
}
```

### Forever Loops

In many cases, you just want to loop forever. You could use a `while (true)`
statement in the above code, like this:

```
ROUTINE(loopForever) {
  ROUTINE_BEGIN();
  while (true) {
    ...
    ROUTINE_YIELD();
  }
  ROUTINE_END();
}
```
but a forever-loop occurs so often that there is a convenience macro
named `ROUTINE_LOOP()` to make this easy:
```
ROUTINE(loopForever) {
  ROUTINE_LOOP() {
    ...
    ROUTINE_YIELD();
    ...
  }
}
```

Note that the terminating `ROUTINE_END()` is no longer required,
because the loop doesn't exit in this case. (Technically, it
isn't required with the `while (true)` version either, but I'm trying hard to
preserve the rule that a `BEGIN()` must always be matched by an `END()`).

You could actually exit the loop using `ROUTINE_END()` in the middle of the
loop:
```
ROUTINE(loopForever) {
  ROUTINE_LOOP() {
    if (condition) {
      ROUTINE_END();
    }
    ...
    ROUTINE_YIELD();
  }
}
```
I hadn't designed this syntax to work from the start, and was surprised to find
that it actually worked.

### Nested Routines

Routines **cannot** be nested. In other words, if you call another function
from within a routine, that nested function is just a regular C++ function,
not a routine. You should not (and cannot) use the various `ROUTINE_XXX()`
macros inside the nested function. The macros have been designed to trigger
compiler errors if you try:
```
void doSomething() {
  ...
  ROUTINE_YIELD(); // ***compiler error***
  ...
}

ROUTINE(cannotUseNestedRoutines) {
  ROUTINE_LOOP() {
    if (condition) {
      doSomething(); // doesn't work
    } else {
      ROUTINE_YIELD();
    }
  }
}
```

### Routine Instance

All routines are instances of the `Routine` class or one of its
subclasses. The name of the routine instance is the name provided
in the `ROUTINE()` macro. So for
```
ROUTINE(doSomething) {
  ROUTINE_BEGIN();
  ...
  ROUTINE_END();
}
```
there is a globally scoped instance of a subclass of `Routine` named
`doSomething`. The name of this subclass is `Routine_doSomething` but it is
unlikely that you will need know the exact name of this generated class.

### Routine State

A routine has 4 internal states:
* `kStatusYielding`: routine returned using `ROUTINE_YIELD()`
* `kStatusDelaying`: routine returned using `ROUTINE_DELAYING()`
* `kStatusEnding`: routine returned using `ROUTINE_END()`
* `kStatusTerminated`: routine has been removed from the scheduler queue and
  is permanently terminated

You can query these internal states using the following methods on the
`Routine` class:
* `Routine::isYielding()`
* `Routine::isDelaying()`
* `Routine::isEnding()`
* `Routine::isTerminated()`

To call one of these functions, use the `Routine` instance variable that
was created using the `ROUTINE()` macro:

```
ROUTINE(doSomething) {
  ROUTINE_BEGIN();
  ...
  ROUTINE_END();
}

ROUTINE(doSomethingElse) {
  ROUTINE_BEGIN();

  ...
  ROUTINE_AWAIT(doSomething.isTerminated());

  ...
  ROUTINE_END();
}
```

### Macros Can Be Used As Statements

Notice that the `ROUTINE_YIELD()` macro in the above example appears on the
same line as the `while`, without the optional `{ }` braces. These macros have
been carefully constructed to allow them to be used almost everywhere a valid
C/C++ statement is allowed.

### External Routines

A routine defined in another `.cpp` file will initialize and run just like any
other routine. However, if you want to reference an externally defined
routine, you must provide an `extern` declaration for that instance. The macro
that will do that for you is `EXTERN_ROUTINE()`.

If the routine in the external `.cpp` file was defined like this:
```
ROUTINE(doSomethingExternal) {
  ...
}
```
then the extern declaration is defined like this:
```
EXTERN_ROUTINE(doSomethingExternal);

ROUTINE(doSomethingExternal) {

  ...
  if (!doSomethingExternal.isTerminated()) ROUTINE_DELAY(1000);
  ...

}
```

### Communication Between Routines

The AceRoutine library does not provide any internal mechanism to
pass data between routines. Here are some options:

The easiest is to use **global variables** which are modified by multiple
routines.

You can use custom Routine classes and define a class static variables which can
be shared among routines which sublcass The same class.

You can define methods on the custom Routine class, and pass messages back and
forth between routines by method calls.

### Advanced Custom Routines

We have said that all routines are instances of the `Routine` class.
You can create custom subclasses of `Routine` and create routines which
are instances of the custom class. Use the 2-argument version of the
`ROUTINE` macro like this:
```
class CustomRoutine : public Routine {
  public:
    void enable(bool enable) { enabled = enable; }

  protected:
    bool enabled = 0;
};

ROUTINE(CustomRoutine, blinkSlow) {
  ROUTINE_LOOP() {
    ...
  }
}
...
```
The 2-argument version created an instace called `blinkSlow` which is an
instance of a subclass of `CustomRoutine`. Everything else remains the same.

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
      same technique as this `AceRoutine` library. I did not discover this
      until I had pretty much completed the library.

### Comparing AceRoutine to Other Libraries

This library falls in the "Threads or Coroutines" camp. The inspiration for this
library came from
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
  occurred to that in C++, we could make the context variables almost disappear
  by making "coroutine" an instance of a class and moving the context variables
  into the member variables.

I looked around to see if there already was a library that implemented these
ideas and I couldn't see one. However, after essentially finishing this library,
I discovered I had pretty much reimplemented the `<ProtoThread.h>` library in
the Cosa framework. It was eerie to see how similar the 2 implementations had
turned out.

## Resource Consumption

### Memory

All objects are statically allocated (i.e. not heap or stack).

* 8-bit processors (AVR Nano, UNO, etc):
    * sizeof(Routine): 13
    * sizeof(RoutineScheduler): 2
* 32-bit processors (e.g. Teensy ARM, ESP8266, ESP32)
    * sizeof(Routine): 24
    * sizeof(RoutineScheduler): 4

In other words, you can create 100 `Routine` instances and they would use only
1300 bytes of static RAM on an 8-bit AVR processor.

The `RoutineScheduler` consumes only 2 bytes of memory no matter how many
routines are created. That's because it depends on a singly-link list whose
pointers live on the `Routine` object, not in the `RoutineScheduler`.

### CPU

See [examples/AutoBenchmark](examples/AutoBenchmark).

## System Requirements

This library was developed and tested using:
* [Arduino IDE 1.8.5](https://www.arduino.cc/en/Main/Software)
* [Teensyduino 1.41](https://www.pjrc.com/teensy/td_download.html)
* [ESP8266 Arduino Core 2.4.1](https://arduino-esp8266.readthedocs.io/en/2.4.1/)
* [arduino-esp32](https://github.com/espressif/arduino-esp32)

I used MacOS 10.13.3 and Ubuntu 17.10 for most of my development.

The library is tested on the following hardware before each release:

* Arduino Nano clone (16 MHz ATmega328P)
* Arduino Pro Micro clone (16 MHz ATmega32U4)
* Teensy 3.2 (72 MHz ARM Cortex-M4)
* NodeMCU 1.0 clone (ESP-12E module, 80 MHz ESP8266)
* ESP32 dev board (ESP-WROOM-32 module, 240 MHz dual core Tensilica LX6)

I will occasionally test on the following hardware as a sanity check:

* Arduino UNO R3 clone (16 MHz ATmega328P)
* Arduino Pro Mini clone (16 MHz ATmega328P)
* Teensy LC (48 MHz ARM Cortex-M0+)
* ESP-01 (ESP-01 module, 80 MHz ESP8266)

## Changelog

See [CHANGELOG.md](CHANGELOG.md).

## License

[MIT License](https://opensource.org/licenses/MIT)

## Authors

Created by Brian T. Park (brian@xparks.net).
