# AceRoutine

A low-memory, fast-switching, cooperative multitasking library using
stackless coroutines on Arduino platforms.

Version: 0.2 (2018-10-02)

This library is currently in "beta" status. I'm releasing it through the Arduino
Library Manager to solicit feedback from interested users. Send me an email or
create a GitHub ticket.

[![AUniter Jenkins Badge](https://us-central1-xparks2018.cloudfunctions.net/badge?project=AceRoutine)](https://github.com/bxparks/AUniter)

## Summary

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
  an early experimental feature whose API and feature may change considerably
  in the future.

The library provides a number of macros to help create coroutines and manage
their life cycle:
* `COROUTINE()`: defines an instance of the `Coroutine` class or an
  instance of a user-defined subclass of `Coroutine`
* `COROUTINE_BEGIN()`: must occur at the start of a coroutine body
* `COROUTINE_END()`: must occur at the end of the coroutine body
* `COROUTINE_YIELD()`: yields execution back to the caller, often
  `CoroutineScheduler` but not necessarily
* `COROUTINE_AWAIT(condition)`: yield until `condition` become `true`
* `COROUTINE_DELAY(millis)`: yields back execution for `millis`. The `millis`
  parameter is defined as a `uint16_t`.
* `COROUTINE_DELAY_SECONDS(loopCounter, seconds)`: yields back execution for
  `seconds`. The `seconds` parameter is defined as a `uint16_t`.
* `COROUTINE_LOOP()`: convenience macro that loops forever
* `COROUTINE_CHANNEL_WRITE(channel, value)`: writes a value to a `Channel`
* `COROUTINE_CHANNEL_READ(channel, value)`: reads a value from a `Channel`

Here are some of the compelling features of this library compared to
others (in my opinion of course):
* low memory usage
    * each coroutine consumes only 14 bytes of RAM on 8-bit processors (AVR) and
      28 bytes on 32-bit processors (ARM, ESP8266, ESP32)
    * the `CoroutineScheduler` consumes only 2 bytes (8-bit) or 4 bytes (32-bit)
      no matter how many coroutines are active
* extremely fast context switching
    * ~6 microseconds on a 16 MHz ATmega328P
    * 1.1-2.0 microseconds on Teensy 3.2 (depending on compiler settings)
    * ~1.7 microseconds on a ESP8266
    * ~0.5 microseconds on a ESP32
* uses "computed goto" feature of GCC to avoid the
  [Duff's Device](https://en.wikipedia.org/wiki/Duff%27s_device) hack
    * allows `switch` statemens in the coroutines
* C/C++ macros eliminate boilerplate code and make the code easy to read
* the base `Coroutine` class is easy to subclass to add additional variables and
  functions
* fully unit tested using [AUnit](https://github.com/bxparks/AUnit)

Some limitations are:
* A `Coroutine` cannot return any values.
* A `Coroutine` is stackless cannot preserve local stack varaibles
  across multiple calls. Often the class member variables or function static
  variables are reasonable substitutes.
* A `Channel` is an experimental feature and has limited features. It is
  currently an unbuffered, synchrronized channel. It can be used by only one
  reader and one writer.

After I had completed most of this library, I discovered that I had essentially
reimplemented the `<ProtoThread.h>` library in the
[Cosa framework](https://github.com/mikaelpatel/Cosa). The difference is that
AceRoutine is a self-contained library that works on any platform supporting the
Arduino API (AVR, Teensy, ESP8266, ESP32, etc), and it provides a handful of
additional macros that can reduce boilerplate code.

### HelloCoroutine

This is the [HelloCoroutine.ino](examples/HelloCoroutine) sample sketch.

```C++
#include <AceRoutine.h>
using namespace ace_routine;

const int LED = LED_BUILTIN;
const int LED_ON = HIGH;
const int LED_OFF = LOW;

const int LED_ON_DELAY = 100;
const int LED_OFF_DELAY = 500;

COROUTINE(blinkLed) {
  COROUTINE_LOOP() {
    digitalWrite(LED, LED_ON);
    COROUTINE_DELAY(LED_ON_DELAY);
    digitalWrite(LED, LED_OFF);
    COROUTINE_DELAY(LED_OFF_DELAY);
  }
}

COROUTINE(printHello) {
  COROUTINE_BEGIN();

  Serial.print(F("Hello, "));
  COROUTINE_DELAY(1000);

  COROUTINE_END();
}

COROUTINE(printWorld) {
  COROUTINE_BEGIN();

  COROUTINE_AWAIT(printHello.isDone());
  Serial.println(F("World!"));

  COROUTINE_END();
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro
}

void loop() {
  blinkLed.runCoroutine();
  printHello.runCoroutine();
  printWorld.runCoroutine();
}
```

This prints "Hello, ", then waits one second, and then prints "World!".
At the same time, the LED blinks on and off.

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

  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
```

The `CoroutineScheduler` can automatically manage all coroutines defined by the
`COROUTINE()` macro, which eliminates the need to itemize your coroutines in
the `loop()` method manually.

## Installation

The latest stable release is available in the Arduino IDE Library Manager.
Search for "AceRoutine". Click Install.

The development version can be installed by cloning the
[GitHub repository](https://github.com/bxparks/AceRoutine), checking out the
`develop` branch, then manually copying over the contents to the `./libraries`
directory used by the Arduino IDE. (The result is a directory named
`./libraries/AceRoutine`.) The `master` branch contains the stable release.

### Source Code

The source files are organized as follows:
* `src/AceRoutine.h` - main header file
* `src/ace_routine/` - implementation files
* `src/ace_routine/cli` - command line interface library
* `src/ace_routine/testing/` - internal testing files
* `tests/` - unit tests which depend on
  [AUnit](https://github.com/bxparks/AUnit)
* `examples/` - example programs

### Docs

The [docs/](docs/) directory contains the
[Doxygen docs published on GitHub Pages](https://bxparks.github.io/AceRoutine/html).

### Examples

The following example sketches are provided:

* [AutoBenchmark.ino](examples/AutoBenchmark):
  a program that performs CPU benchmarking
* [HelloCoroutine.ino](examples/HelloCoroutine)
* [HelloScheduler.ino](examples/HelloScheduler): same as `HelloCoroutine`
  except using the `CoroutineScheduler` instead of manually running the
  coroutines
* [BlinkSlowFastRoutine.ino](examples/BlinkSlowFastRoutine): use coroutines
  to read a button and control how the LED blinks
* [BlinkSlowFastCustomRoutine.ino](examples/BlinkSlowFastCustomRoutine): same
  as BlinkSlowFastRoutine but using a custom `Coroutine` class
* [CountAndBlink.ino](examples/CountAndBlink): count and blink at the same time
* [CommandLineShell.ino](examples/CommandLineShell): uses the
  `src/ace_routine/cli` classes to implement a command line interface that
  accepts a number of commands on the serial port. In other words, it is a
  primitive "shell". The shell is non-blocking and uses coroutines so that other
  coroutines continue to run while the board waits for commands to be typed on
  the serial port.
* [Pipe.ino](examples/Pipe): uses a `Channel` to allow a Writer to send
  messages to a Reader
* [ChannelBenchmark.ino](examples/ChannelBenchmark): determines the amount of
  CPU overhead of a `Channel` by using 2 coroutines to ping-pong an integer
  across 2 channels

## Usage

### Include Header and Namespace

Only a single header file `AceRoutine.h` is required to use this library.
To prevent name clashes with other libraries that the calling code may use, all
classes are defined in the `ace_routine` namespace. To use the code without
prepending the `ace_routine::` prefix, use the `using` directive:

```C++
#include <AceRoutine.h>
using namespace ace_routine;
```

### Macros

The following macros are available to hide a lot of boilerplate code:

* `COROUTINE()`: defines an instance of `Coroutine` class or a user-provided
  custom subclass of `Coroutine`
* `COROUTINE_BEGIN()`: must occur at the start of a coroutine body
* `COROUTINE_END()`: must occur at the end of the coroutine body
* `COROUTINE_YIELD()`: yields execution back to the `CoroutineScheduler`
* `COROUTINE_AWAIT(condition)`: yield until `condition` become `true`
* `COROUTINE_DELAY(millis)`: yields back execution for `millis`. The maximum
  allowable delay is 32767 milliseconds.
* `COROUTINE_DELAY_SECONDS(loopCounter, seconds)`: yields back execution for
  `seconds`. The maximum allowable delay is the maximum value of the integer
  type of `loopCounter` which can be of any integer type.
* `COROUTINE_LOOP()`: convenience macro that loops forever, replaces
  `COROUTINE_BEGIN()` and `COROUTINE_END()`
* `COROUTINE_CHANNEL_WRITE()`: writes a message to a `Channel`
* `COROUTINE_CHANNEL_READ()`: reads a message from a `Channel`

### Overall Structure

The overall structure looks like this:
```C++
#include <AceRoutine.h>
using namespace ace_routine;

COROUTINE(oneShotRoutine) {
  COROUTINE_BEGIN();
  ...
  COROUTINE_YIELD();
  ...
  COROUTINE_AWAIT(condition);
  ...
  COROUTINE_DELAY(100);
  ...
  COROUTINE_END();
}

COROUTINE(loopingRoutine) {
  COROUTINE_LOOP() {
    ...
    COROUTINE_YIELD();
    ...
  }
}

void setup() {
  // Set up Serial port if needed by app, not needed by AceRoutine
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  ...
  CoroutineScheduler::setup();
  ...
}

void loop() {
  CoroutineScheduler::loop();
}
```

### Coroutine Instance

All coroutines are instances of the `Coroutine` class or one of its
subclasses. The name of the coroutine instance is the name provided
in the `COROUTINE()` macro. For example, in the following example:
```C++
COROUTINE(doSomething) {
  COROUTINE_BEGIN();
  ...
  COROUTINE_END();
}
```
there is a globally-scoped object named `doSomething` which is an instance of a
subclass of `Coroutine`. The name of this subclass is autogenerated to be
`Coroutine_doSomething` but it is unlikely that you will need know the exact
name of this generated class.

### Coroutine Body

The code immediately following the `COROUTINE()` macro becomes the body of the
`Coroutine::runCoroutine()` virtual method. Within this `runCoroutine()` method,
various helper macros (e.g. `COROUTINE_BEGIN()`, `COROUTINE_YIELD()`,
`COROUTINE_DELAY()`, etc) can be used. These helper macros are described below.

### Begin and End Markers

Within the `COROUTINE()` macro, the beginning of the coroutine code must start
with the `COROUTINE_BEGIN()` macro and the end of the coroutine code must end
with the `COROUTINE_END()` macro. They initialize various bookkeeping variables
in the `Coroutine` class that enable coroutines to be implemented. All other
`COROUTINE_xxx()` macros must appear between these BEGIN and END macros.

The `COROUTINE_LOOP()` macro is a special case that replaces the
`COROUTINE_BEGIN()` and `COROUTINE_END()` macros. See the **Forever Loops**
section below.

### Yield

`COROUTINE_YIELD()` returns control to the `CoroutineScheduler` which is then
able to run another coroutines. Upon the next iteration, execution continues
just after `COROUTINE_YIELD()`. (Technically, the execution always begins at the
top of the function, but the `COROUTINE_BEGIN()` contains a dispatcher that
gives the illusion that the execution continues further down the function.)

### Await

`COROUTINE_AWAIT(condition)` yields until the `condition` evaluates to `true`.
This is a convenience macro that is identical to:
```C++
while (!condition) COROUTINE_YIELD();
```

### Delay

The `COROUTINE_DELAY(millis)` macro delays the return of control until `millis`
milliseconds have elapsed. The `millis` argument is a `uint16_t`, a 16-bit
unsigned integer, which reduces the size of each coroutine instance by 4 bytes.
However, the actual maximum delay is limited to 32767 milliseconds to avoid
overflow situations if the other coroutines in the system take too much time for
their work before returning control to the waiting coroutine. With this limit,
the other coroutines have as much as 32767 milliseconds to complete their work,
which should be more than enough time for any conceivable situation. In
practice, coroutines should complete their work within several milliseconds and
yield control to the other coroutines as soon as possible.

To delay for longer, an explicit loop can be used. For example, to delay
for 1000 seconds, we can do this:
```C++
COROUTINE(waitThousandSeconds) {
  COROUTINE_BEGIN();
  static uint16_t i;
  for (i = 0; i < 1000; i++) {
    COROUTINE_DELAY(1000);
  }
  ...
  COROUTINE_END();
}
```
See **For Loop** section below for a description of the for-loop construct.

This for-loop construct happens often enough that it seemed worthwhile to
provide the `COROUTINE_DELAY_SECONDS(loopCounter, seconds)` convenience macro.
It replaces the above for-loop, like this:
```C++
COROUTINE(waitThousandSeconds) {
  COROUTINE_BEGIN();
  static uint16_t loopCounter;
  COROUTINE_DELAY_SECONDS(loopCounter, 1000);
  ...
  COROUTINE_END();
}
```
The `static uint16_t loopCounter` variable is still required for the
`COROUTINE_DELAY_SECONDS()` macro because it needs a loop counter that must
preserve its value across multiple invocation of the coroutine. The
`loopCounter` may be any integer type, and the maximum number of seconds is the
maximum value of that particular integer type. For example, if the `loopCounter`
was a `uint8_t`, the maximum delay would be 255 seconds. If the `loopCounter`
was a `uint32_t`, the maximum delay would be about 4 billion seconds.

The `loopCounter` may also be a member variable of the `Coroutine` subclass,
when you use custom or manual coroutines (see the sections on *Custom
Coroutines* or *Manual Coroutines* below for details). In other words, you can
define a coroutine with a `mDelayCounter` member variable, and use the
`COROUTINE_DELAY_SECONDS()` macro like this:
```C++
class MyCoroutine: public Coroutine {
  public:
    virtual int runCoroutine() override {
      ...
      COROUTINE_DELAY_SECONDS(mDelayCounter, 1000);
      ...
    }

  private:
    uint16_t mDelayCounter;
};
```

### Stackless Coroutines

Each coroutine is stackless. More accurately, the stack of the coroutine
is destroyed and recreated on every invocation of the coroutine. Therefore,
any local variable created on the stack in the coroutine will not preserve
its value after a `COROUTINE_YIELD()` or a `COROUTINE_DELAY()`.

The easiest way to get around ths problem is to use `static` variables inside
a `COROUTINE()`. Static variables are initialized once and preserve their value
through multiple calls to the function, which is exactly what is needed.

### Conditional If-Else

Conditional if-statements work as expected with the various macros:
```C++
COROUTINE(doIfThenElse) {
  COROUTINE_BEGIN();

  if (condition) {
    ...
    COROUTINE_YIELD();
  } else {
    ...
    COROUTINE_DELAY(100);
  }

  ...

  COROUTINE_END();
}
```

### Switch Statements

Unlike some implementations of stackless coroutines, AceRoutine coroutines are
compatible with `switch` statements:

```C++
COROUTINE(doThingsBasedOnSwitchConditions) {
  COROUTINE_BEGIN();
  ...

  switch (value) {
    case VAL_A:
      ...
      COROUTINE_YIELD();
      break;
    case VAL_B:
      ...
      COROUTINE_DELAY(100);
      break;
    default:
      ...
  }
  ...
  COROUTINE_END();
}
```

### For Loops

You cannot use a local variable in the `for-loop` because the variable counter
would be created on the stack, and the stack gets destroyed as soon as
`COROUTINE_YIELD()`, `COROUTINE_DELAY()`, or `COROUTINE_AWAIT()` is executed.
However, a reasonable solution is to use `static` variables. For example:

```C++
COROUTINE(countToTen) {
  COROUTINE_BEGIN();
  static int i = 0;
  for (i = 0; i < 10; i++) {
    ...
    COROUTINE_DELAY(100);
    ...
  }
  COROUTINE_END();
}
```

### While Loops

You can write a coroutine that loops while certain condition is valid like this,
just like you would normally, except that you call the `COROUTINE_YIELD()`
macro to cooperatively allow other coroutines to execute.

```C++
COROUTINE(loopWhileCondition) {
  COROUTINE_BEGIN();
  while (condition) {
    ...
    COROUTINE_YIELD();
    ...
  }
  COROUTINE_END();
}
```

Make sure that the `condition` expression does not use any local variables,
since local variables are destroyed and recreated after each YIELD, DELAY or
AWAIT.

### Forever Loops

In many cases, you just want to loop forever. You could use a `while (true)`
statement, like this:

```C++
COROUTINE(loopForever) {
  COROUTINE_BEGIN();
  while (true) {
    ...
    COROUTINE_YIELD();
  }
  COROUTINE_END();
}
```

However, a forever-loop occurs so often that I created a convenience macro
named `COROUTINE_LOOP()` to make this easier:

```C++
COROUTINE(loopForever) {
  COROUTINE_LOOP() {
    ...
    COROUTINE_YIELD();
    ...
  }
}
```

Note that the terminating `COROUTINE_END()` is no longer required,
because the loop does not terminate. (Technically, it isn't required with the
`while (true)` version either, but I'm trying hard to preserve the rule that a
`COROUTINE_BEGIN()` must always be matched by a `COROUTINE_END()`).

You could actually exit the loop using `COROUTINE_END()` in the middle of the
loop:
```C++
COROUTINE(loopForever) {
  COROUTINE_LOOP() {
    if (condition) {
      COROUTINE_END();
    }
    ...
    COROUTINE_YIELD();
  }
}
```
I hadn't explicitly designed this syntax to be valid from the start, and was
surprised to find that it actually worked.

### No Nested Coroutine Macros

Coroutines macros **cannot** be nested. In other words, if you call another
function from within a coroutine, you cannot use the various `COROUTINE_XXX()`
macros inside the nested function. The macros will trigger compiler errors if
you try:
```C++
void doSomething() {
  ...
  COROUTINE_YIELD(); // ***compiler error***
  ...
}

COROUTINE(cannotUseNestedMacros) {
  COROUTINE_LOOP() {
    if (condition) {
      doSomething(); // doesn't work
    } else {
      COROUTINE_YIELD();
    }
  }
}
```

### Chaining Coroutines

Coroutines can be chained, in other words, one coroutine *can* explicitly
call another coroutine, like this:
```C++
COROUTINE(inner) {
  COROUTINE_LOOP() {
    ...
    COROUTINE_YIELD();
    ...
  }
}

COROUTINE(outer) {
  COROUTINE_LOOP() {
    ...
    inner.runCoroutine();
    ...
    COROUTINE_YIELD();
  }
}

```
I have yet to find it useful to call a Coroutine defined with the `COROUTINE()`
from another Coroutine defined by the same `COROUTINE()` macro.

However, I have found it useful to chain coroutines when using the **Manual
Coroutines** described in one of the sections below. The ability to chain
coroutines allows us to implement a [Decorator
Pattern](https://en.wikipedia.org/wiki/Decorator_pattern) or a chain of
responsibility. Using manual coroutines, we can wrap one coroutine with another
and delegate to the inner coroutine like this:

```C++
class InnerCoroutine: public Coroutine {
  public:
    InnerCoroutine(..) { ...}

    virtual int runCoroutine override {
      COROUTINE_BEGIN();
      ...
      COROUTINE_END();
      ...
    }
};

class OuterCoroutine: public Coroutine {
  public:
    OuterCoroutine(InnerCoroutine& inner): mInner(inner) {
      ...
    }

    virtual int runCoroutine override {
      // No COROUTINE_BEGIN() and COROUTINE_END() needed if this simply
      // delegates to the InnerCoroutine.
      mInner.runCoroutine();
    }

  private:
    Coroutine& mInner;
};

```
Most likely, only the `OuterCoroutine` would be registered in the
`CoroutineScheduler`. And in the cases that I've come across, the
`OuterCoroutine` doesn't actually use much of the Coroutine functionality
(i.e. doesn't actuall use the `COROUTINE_BEGIN()` and `COROUTINE_END()` macros.
It simply delegates the `runCoroutine()` call to the inner one.

### Running and Scheduling

There are 2 ways to run the coroutines:
* manually calling the coroutines in the `loop()` method, or
* using the `CoroutineScheduler`.

#### Manual Scheduling

If you have only a small number of coroutines, the manual method may be the
easiest. This requires you to explicitly call the `runCoroutine()` method of all
the coroutines that you wish to run in the `loop()` method, like this:
```C++
void loop() {
  blinkLed.runCoroutine();
  printHello.runCoroutine();
  printWorld.runCoroutine();
}
```

#### CoroutineScheduler

If you have a large number of coroutines, especially if some of them are
defined in multiple `.cpp` files, then the `CoroutineScheduler` will
make things easy. You just need to call `CoroutineScheduler::setup()`
in the global `setup()` method, and `CoroutineScheduler::loop()`
in the global `loop()` method, like this:
```C++
void setup() {
  ...
  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
```

The `CoroutineScheduler::setup()` method creates an internal list of active
coroutines that are managed by the scheduler. Each call to
`CoroutineScheduler::loop()` executes one coroutine in that list in a simple
round-robin scheduling algorithm.

The list of scheduled coroutines is initially ordered by using
`Coroutine::getName()` as the sorting key. This makes the scheduling
deterministic, which allows unit tests to work. However,
calling `Coroutine.suspend()` then subsequently calling`Coroutine.resume()` puts
the coroutine at the beginning of the scheduling list, so the ordering may
become mixed up over time if these functions are used.

#### Manual Scheduling or the CoroutineScheduler

Manual scheduling has the smallest context switching overhead between
coroutines. However, it is not possible to `suspend()` or `resume()` a coroutine
because those methods affect how the `CoroutineScheduler` chooses to run a
particular coroutine. Similarly, the list of coroutines in the global `loop()`
is fixed by the code at compile-time. So when a coroutine finishes with the
`COROUTINE_END()` macro, it will continue to be called by the `loop()` method.

The `CoroutineScheduler` is easier to use because it automatically keeps track
of all coroutines defined by the `COROUTINE()` macro, even if they are
defined in multiple files. It allows coroutines to be suspended and resumed (see
below). However, there is a small overhead in switching between coroutines
because the scheduler needs to walk down the list of active coroutines to find
the next one. The scheduler is able to remove coroutines which are not running,
if there are a significant number of these inactive coroutines, then the
`CoroutineScheduler` may actually be more efficient than manually calling the
coroutines through the global `loop()` method.

### Suspend and Resume

The `Coroutine::suspend()` and `Coroutine::resume()` methods are available
*only* if the `CoroutineScheduler` is used. If the coroutines are called
explicitly in the global `loop()` method, then these methods have no impact.

A coroutine can suspend itself or be suspended by another coroutine.
It causes the `CoroutineScheduler` to remove the coroutine from the list of
actively running coroutines, just before the next time the scheduler attempts to
run the coroutine.

If the `Coroutine::suspend()` method is called on the coroutine *before*
`CoroutineScheduler::setup()` is called, the scheduler will not insert the
coroutine into the active list of coroutines at all. This is useful in unit
tests to prevent extraneous coroutines from interfering with test validation.

### Coroutine States

A coroutine has several internal states:
* `kStatusSuspended`: coroutine was suspended using `Coroutine::suspend()`
* `kStatusYielding`: coroutine returned using `COROUTINE_YIELD()` or
  `COROUTINE_AWAIT()`
* `kStatusDelaying`: coroutine returned using `COROUTINE_DELAY()`
* `kStatusRunning`: coroutine is currently running
* `kStatusEnding`: coroutine returned using `COROUTINE_END()`
* `kStatusTerminated`: coroutine has been removed from the scheduler queue and
  is permanently terminated. Set only by the `CoroutineScheduler`.

The finite state diagram looks like this:
```
                     ----------------------------
         Suspended                              ^
         ^       ^                              |
        /         \                             |
       /           \                            |
      v             \       --------            |
Yielding          Delaying         ^            |
     ^               ^             |            |
      \             /              |        accessible
       \           /               |        using
        \         /                |        CoroutineScheduler
         v       v          accessible          |
          Running           by calling          |
             |              runCoroutine()      |
             |              directly            |
             |                     |            |
             v                     |            |
          Ending                   v            |
             |              --------            |
             |                                  |
             v                                  |
        Terminated                              v
                    -----------------------------
```

You can query these internal states using the following methods on the
`Coroutine` class:
* `Coroutine::isSuspended()`
* `Coroutine::isYielding()`
* `Coroutine::isDelaying()`
* `Coroutine::isRunning()`
* `Coroutine::isEnding()`
* `Coroutine::isTerminated()`
* `Coroutine::isDone()`: same as `isEnding() || isTerminated()`. This method
  is preferred because it works when the `Coroutine` is executed manually or
  through the `CoroutineScheduler`.

To call these functions on a specific coroutine, use the `Coroutine` instance
variable that was created using the `COROUTINE()` macro:

```C++
COROUTINE(doSomething) {
  COROUTINE_BEGIN();
  ...
  COROUTINE_END();
}

COROUTINE(doSomethingElse) {
  COROUTINE_BEGIN();

  ...
  COROUTINE_AWAIT(doSomething.isDone());

  ...
  COROUTINE_END();
}
```

### Macros Can Be Used As Statements

The `COROUTINE_YIELD()`, `COROUTINE_DELAY()`, `COROUTINE_AWAIT()` macros have
been designed to allow them to be used almost everywhere a valid C/C++ statement
is allowed. For example, the following is allowed:
```C++
  ...
  if (condition) COROUTINE_YIELD();
  ...
```

### Custom Coroutines

All coroutines are instances of the `Coroutine` class, or one of its subclasses.
You can create custom subclasses of `Coroutine` and create coroutines which are
instances of the custom class. Use the 2-argument version of the `COROUTINE()`
macro like this:
```C++
class CustomCoroutine : public Coroutine {
  public:
    void enable(bool isEnabled) { enabled = isEnabled; }

    // the runCoroutine() method will be defined by the COROUTINE() macro

  protected:
    bool enabled = 0;
};

COROUTINE(CustomCoroutine, blinkSlow) {
  COROUTINE_LOOP() {
    ...
  }
}
...
```
The 2-argument version created an object instance called `blinkSlow` which is an
instance of an internally generated class named `CustomCoroutine_blinkSlow`
which is a subclass of `CustomCoroutine`.

Custom coroutines were intended to be useful if you need to create multiple
coroutines which share methods or data structures. In practice, however, I have
yet to find a use for them. Instead, I have found that the *Manual Coroutines*
described in the next section to be more useful.

### Manual Coroutines

An manual coroutine is a custom coroutine whose body of the coroutine (i.e
the`runCoroutine()` method) is defined manually and the coroutine object is also
instantiated manually, instead of using the `COROUTINE()` macro. This is useful
if the coroutine has external dependencies which need to be injected into the
constructor. The `COROUTINE()` macro does not allow the constructor to be
customized.

```C++
class ManualCoroutine : public Coroutine {
  public:
    // Inject external dependencies into the constructor.
    ManualCoroutine(Params, ..., Objects, ...) {
      ...
    }

  private:
    virtual int runCoroutine() override {
      COROUTINE_BEGIN();
      // insert coroutine code here
      COROUTINE_END();
    }
};

ManualCoroutine manualRoutine(params, ..., objects, ...);
```

A manual coroutine (created without the `COROUTINE()` macro) is *not*
automatically added to the linked list used by the `CoroutineScheduler`. If you
wish to insert it into the scheduler, use the `setupCoroutine()` method just
before calling `CoroutineScheduler::setup()`:
```C++
void setup() {
  ...
  manualRoutine.setupCoroutine("manualRoutine");
  CoroutineScheduler::setup();
  ...
}

void loop() {
  ...
  CoroutineScheduler::loop();
  ...
}
```

There are 2 versions of the `setupCoroutine()` method:
* `setupCoroutine(const char* name)`
* `setupCoroutine(const __FlashStringHelper* name)`

Both have been designed so that they are safe to be called from the constructor
of a `Coroutine` class, even during static initialization time. This is exactly
what the `COROUTINE()` macro does, call the `setupCoroutine()` method from the
generated constructor. However, a manual coroutine is often written as a library
that is supposed to be used by an end-user, and it would be convenient for the
name of the coroutine to be defined by the end-user. The problem is that the
`F()` macro cannot be used outside of the function context, so it is cannot be
passed into the constructor when the coroutine is statically created. The
workaround is to call the `setupCoroutine()` method in the global `setup()`
function, where the `F()` macro is allowed to be used. (The other more obscure
reason is that the constructor of the manual coroutine class will often have a
large number of dependency injection parameters which are required to implement
its functionality, and it is cleaner to avoid mixing in the name of the
`Coroutine` which is an incidental dependency. Anyway, that's my rationale right
now, but this may change in the future if a simpler alternative is discovered.)

If the coroutine is not given a name, the name is stored as a `nullptr`. When
printed (e.g. using the `CoroutineScheduler::list()` method), the name of an
anonymous coroutine is represented by the integer representation of the `this`
pointer of the coroutine object.

A good example of a manual coroutine is
[src/ace_routine/cli/CommandManager.h](src/ace_routine/cli/CommandManager.h)
and you can see how it is configured in
[examples/CommandLineShell](examples/CommandLineShell).

### External Coroutines

A coroutine can be defined in a separate `.cpp` file. However, if you want to
refer to an externally defined coroutine, you must provide an `extern`
declaration for that instance. The macro that makes this easy is
`EXTERN_COROUTINE()`.

For example, supposed we define a coroutine named `external` like
this in a `External.cpp` file:
```C++
COROUTINE(external) {
  ...
}
```

To use this in `Main.ino` file, we must use the `EXTERN_COROUTINE()` macro like
this:
```C++
EXTERN_COROUTINE(external);

COROUTINE(doSomething) {
  ...
  if (!external.isDone()) COROUTINE_DELAY(1000);
  ...
}
```

If the 2-argument version of `COROUTINE()` was used, then the corresponding
2-argument version of `EXTERN_COROUTINE()` must be used, like this in
`External.cpp`:
```C++
COROUTINE(CustomCoroutine, external) {
  ...
}
```

then this in `Main.ino`:
```C++
EXTERN_COROUTINE(CustomCoroutine, external);

COROUTINE(doSomething) {
  ...
  if (!external.isDone()) COROUTINE_DELAY(1000);
  ...
}
```

### Communication Between Coroutines

There are a handful ways that `Coroutine` instances can pass data between
each other.

* The easiest method is to use **global variables** which are modified by
  multiple coroutines.
* To avoid polluting the global namespace, you can subclass the `Coroutine`
  class and define **class static variables** which can be shared among
  coroutines which inherit this custom class
* You can define **methods on the custom Coroutine class**, and pass messages
  back and forth between coroutines using these methods.
* You can use **channels** as explained in the next section.

### Channels

I have provided an early experimental implementation of channels inspired by the
[Go Lang Channels](https://www.golang-book.com/books/intro/10). The `Channel`
class implements an unbuffered, bidirectional channel. The API and features
of the `Channel` class may change significantly in the future.

Just like Go Lang channels, the AceRoutine `Channel` provides a point of
synchronization between coroutines. In other words, the following sequence of
events is guaranteed when interacting with a channel:

* the writer blocks until the reader is ready,
* the reader blocks until the writer is ready,
* when the writer writes, the reader picks up the the message and is allowed
  to continue execution *before* the writer is allowed to continue,
* the writer then continues execution after the reader yields.

Channels will be most likely be used with Manual Coroutines, in other words,
when you define your own subclasses of `Coroutine` and define your own
`runCoroutine()` method, instead of using the `COROUTINE()` macro. The `Channel`
class can be injected into the constructor of the `Coroutine` subclass.

The `Channel` class is templatized on the channel message class written by the
writer and read by the reader. It will often be useful for the message type to
contain a status field which indicates whether the writer encountered an error.
So a message of just an `int` may look like:
```C++
class Message {
  static uint8_t const kStatusOk = 0;
  static uint8_t const kStatusError = 1;

  uint8_t status;
  int value;
};
```

A `Channel` of this type can be created like this:
```C+++
Channel<Message> channel;
```

This channel should be injected into the writer coroutine and reader coroutine:
```C++
class Writer: public Coroutine {
  public:
    Writer(Channel<Message>& channel, ...):
      mChannel(channel),
      ...
    {...}

  private:
    Channel<Message>& mChannel;
};

class Reader: public Coroutine {
  public:
    Reader(Channel<Message>& channel, ...):
      mChannel(channel),
      ...
    {...}

  private:
    Channel<Message>& mChannel;
};
```

Next, implement the `runCoroutine()` methods of both the Writer and Reader
to pass the `Messager` objects. There are 2 new macros to help with writing to
and reading from channels:

* `COROUTINE_CHANNEL_WRITE(channel, value)`: writes the `value` to the given
  channel, blocking (i.e. yielding) until the reader is ready
* `COROUTINE_CHANNEL_READ(channel, value)`: reads from the channel into the
  given `value`, blocking (i.e. yielding) until the writer is ready to write

Here is the sketch of a Writer that sends 10 integers to the Reader:

```C++
class Writer: public Coroutine {
  public:
    Writer(...) {...}

    virtual int runCoroutine() override {
      static int i;
      COROUTINE_BEGIN();
      for (i = 0; i < 9; i++) {
        Message message = { Message::kStatusOk, i };
        COROUTINE_CHANNEL_WRITER(mChannel, message);
      }
      COROUTINE_END();
    }

  private:
    Channel<Message>& mChannel;
};

class Reader: public Coroutine {
  public
    Reader(...) {...}

    virtual int runCoroutine() override {
      COROUTINE_LOOP() {
        Message message;
        COROUTINE_CHANNEL_READ(mChannel, message);
        if (message.status == Message::kStatusOk) {
          Serial.print("Message received: value = ");
          Serial.println(message.value);
        }
      }
    }

  private:
    Channel<Message>& mChannel;
};

...

Writer writer(channel);
Reader reader(channel);

void setup() {
  Serial.begin(115200);
  while (!Serial); // micro/leonardo

  ...
  writer.setupCoroutine("writer");
  reader.setupCoroutine("reader");
  CoroutineScheduler::setup();
  ...
}

void loop() {
  CoroutineScheduler::loop();
}
```

**Examples**

A really good example of using a `Channel` can be found in the
[ace_routine/cli](src/ace_routine/cli) package which uses 2 coroutines
and a channel between them to communicate:

* `StreamLineReader.h`: a coroutine that reads from `Serial` and writes to a
  `Channel`
* `CommandDispatcher.h`: a coroutine that reads from a `Channel` and dispatches
  to a `CommandHandler`

**Limitations**

* Only a single AceRoutine `Coroutine` can write to a `Channel`.
* Only a single AceRoutine `Coroutine` can read from a `Channel`.
* There is equivalent of a
  [Go Lang select statement](https://gobyexample.com/select), so the coroutine
  cannot wait for multiple channels at the same time.
* There is no buffered channel type.
* There is no provision to
  [close a channel](https://gobyexample.com/closing-channels).

Some of these features may be implemented in the future if I find compelling
use-cases and if they are easy to implement.

### Functors

C++ allows the creation of objects that look syntactically like functions.
by defining the `operator()` method on the class. I have not defined this method
in the `Coroutine` class because I have not found a use-case for it. However, if
someone can demonstrate a compelling use-case, then I would be happy to add it.

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

## Resource Consumption

### Memory

All objects are statically allocated (i.e. not heap or stack).

* 8-bit processors (AVR Nano, UNO, etc):
    * `sizeof(Coroutine)`: 14
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
pointers live on the `Coroutine` object, not in the `CoroutineScheduler`.

The `Channel` object requires 2 copies of the parameterized `<T>` type so its
size is equal to `1 + 2 * sizeof(T)`, rounded to the nearest memory alignment
boundary (i.e. 12 for a 32-bit processor).

### CPU

See [examples/AutoBenchmark](examples/AutoBenchmark).

## System Requirements

This library was developed and tested using:
* [Arduino IDE 1.8.7](https://www.arduino.cc/en/Main/Software)
* [Teensyduino 1.41](https://www.pjrc.com/teensy/td_download.html)
* [ESP8266 Arduino Core 2.4.2](https://arduino-esp8266.readthedocs.io/en/2.4.2/)
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

## Feedback and Support

If you have any questions, comments, bug reports, or feature requests, please
file a GitHub ticket or send me an email. I'd love to hear about how this
software and its documentation can be improved. Instead of forking the
repository to modify or add a feature for your own projects, let me have a
chance to incorporate the change into the main repository so that your external
dependencies are simpler and so that others can benefit. I can't promise that I
will incorporate everything, but I will give your ideas serious consideration.

## Authors

Created by Brian T. Park (brian@xparks.net).
