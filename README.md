# AceRoutine

A low-memory, fast-switching, cooperative multitasking library using
stackless coroutines on Arduino platforms.

Version: 0.1 (2018-08-07)

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

There are only 2 classes in this library:
* each coroutine is an instance of the `Coroutine` class (or a subclass),
* the `CoroutineScheduler` class handles the scheduling.

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
* `COROUTINE_LOOP()`: convenience macro that loops forever

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
* coroutines cannot return any values
* coroutines are stackless, so they cannot preserve local stack varaibles
  across multiple calls. Often the class member variables or function static
  variables are reasonable substitutes.

After I had completed most of this library, I discovered that I had essentially
reimplemented the `<ProtoThread.h>` library in the
[Cosa framework](https://github.com/mikaelpatel/Cosa). The difference is that
AceRoutine is a self-contained library that works on any platform supporting the
Arduino API (AVR, Teensy, ESP8266, ESP32, etc), and it provides a handful of
additional macros that can reduce boilerplate code.

### HelloCoroutine

This is the [HelloCoroutine.ino](examples/HelloCoroutine) sample sketch.

```
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
  blinkLed.run();
  printHello.run();
  printWorld.run();
}
```

This prints "Hello, ", then waits one second, and then prints "World!".
At the same time, the LED blinks on and off.

The [HelloScheduler.ino](examples/HelloScheduler) sketch implements the same
thing using the `CoroutineScheduler`:

```
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

The latest stable release will eventually be available in the Arduino IDE
Library Manager. Search for "AceRoutine". Click Install.
(Unfortunately, it is not there yet.)

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

* [HelloCoroutine.ino](examples/HelloCoroutine)
* [HelloScheduler.ino](examples/HelloScheduler): same as `HelloCoroutine`
  except using the `CoroutineScheduler` instead of manually running the
  coroutines
* [BlinkSlowFastRoutine.ino](examples/BlinkSlowFastRoutine): use coroutines
  to read a button and control how the LED blinks
* [BlinkSlowFastCustomRoutine.ino](examples/BlinkSlowFastCustomRoutine): same
  as BlinkSlowFastRoutine but using a custom `Coroutine` class
* [CountAndBlink.ino](examples/CountAndBlink): count and blink at the same time
* [AutoBenchmark.ino](examples/AutoBenchmark):
  a program that performs CPU benchmarking
* [CommandLineInterface.ino](examples/CommandLineInterface): uses the
  `src/ace_routine/cli` classes to implement a command line interface that
  accepts a number of commands on the serial port. In other words, it is a
  primitive "shell". The shell is non-blocking and uses coroutines so that other
  coroutines continue to run while the board waits for commands to be typed on
  the serial port.

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
* `COROUTINE()`: defines an instance of `Coroutine` class or a user-provided
  custom subclass of `Coroutine`
* `COROUTINE_BEGIN()`: must occur at the start of a coroutine body
* `COROUTINE_END()`: must occur at the end of the coroutine body
* `COROUTINE_YIELD()`: yields execution back to the `CoroutineScheduler`
* `COROUTINE_AWAIT(condition)`: yield until `condition` become `true`
* `COROUTINE_DELAY(millis)`: yields back execution for `millis`. The maximum
  allowable delay is 32767 milliseconds.
* `COROUTINE_LOOP()`: convenience macro that loops forever, replaces
  `COROUTINE_BEGIN()` and `COROUTINE_END()`

### Overall Structure

The overall structure looks like this:
```
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

### Coroutine Body

The `COROUTINE(name)` macro defines an instance of the `Coroutine` class named
`name`. The code immediately following the macro, in the `{ ... }`, becomes the
body of the `Coroutine::run()` virtual method. Within this `run()` method,
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
```
while (!condition) COROUTINE_YIELD();
```

### Delay

`COROUTINE_DELAY(millis)` delays the return of control until `millis`
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
```
COROUTINE(waitThousandSeconds) {
  COROUTINE_BEGIN();
  static i = 0;
  for (i = 0; i < 100; i++) {
    COROUTINE_DELAY(10000);
  }
  ...
  COROUTINE_END();
}
```
See **For Loop** section below for a description of the for-loop construct.

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
```
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

```
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

```
COROUTINE(countToTen) {
  COROUTINE_BEGIN();
  static i = 0;
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

```
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

```
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

```
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
```
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
```
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
```
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
    inner.run();
    ...
    COROUTINE_YIELD();
  }
}

```

Although this is techically allowed, I have not yet discovered a practical
use-case for this feature.

### Coroutine Instance

All coroutines are instances of the `Coroutine` class or one of its
subclasses. The name of the coroutine instance is the name provided
in the `COROUTINE()` macro. For example, in the following example:
```
COROUTINE(doSomething) {
  COROUTINE_BEGIN();
  ...
  COROUTINE_END();
}
```
there is a globally scoped instance of a subclass of `Coroutine` named
`doSomething`. The name of this subclass is `Coroutine_doSomething` but it is
unlikely that you will need know the exact name of this generated class.

### Running and Scheduling

There are 2 ways to run the coroutines:
* manually calling the coroutines in the `loop()` method, or
* using the `CoroutineScheduler`.

#### Manual Scheduling

If you have only a small number of coroutines, the manual method may be the
easiest. This requires you to explicitly call the `run()` method of all the
coroutines that you wish to run in the `loop()` method, like this:
```
void loop() {
  blinkLed.run();
  printHello.run();
  printWorld.run();
}
```

#### CoroutineScheduler

If you have a large number of coroutines, especially if some of them are
defined in multiple `.cpp` files, then the `CoroutineScheduler` will
make things easy. You just need to call `CoroutineScheduler::setup()`
in the global `setup()` method, and `CoroutineScheduler::loop()`
in the global `loop()` method, like this:
```
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
* `kStatusYielding`: coroutine returned using `COROUTINE_YIELD()`
* `kStatusAwaiting`: coroutine returned using `COROUTINE_AWAIT()`
* `kStatusDelaying`: coroutine returned using `COROUTINE_DELAY()`
* `kStatusRunning`: coroutine is currently running
* `kStatusEnding`: coroutine returned using `COROUTINE_END()`
* `kStatusTerminated`: coroutine has been removed from the scheduler queue and
  is permanently terminated. Set only by the `CoroutineScheduler`.

The finite state diagram looks like this:
```
         Suspended
         ^   ^   ^
        /    |    \
       /     |     \
      v      |      \
Yielding Awaiting Delaying
     ^       ^       ^
      \      |      /
       \     |     /
        \    |    /
         v   v   v
          Running
             |
             |
             v
          Ending
             |
             |
             v
        Terminated
```

You can query these internal states using the following methods on the
`Coroutine` class:
* `Coroutine::isSuspended()`
* `Coroutine::isYielding()`
* `Coroutine::isAwaiting()`
* `Coroutine::isDelaying()`
* `Coroutine::isRunning()`
* `Coroutine::isEnding()`
* `Coroutine::isTerminated()`
* `Coroutine::isDone()`: same as `isEnding() || isTerminated()`. This method
  is preferred because it works when the `Coroutine` is executed manually or
  through the `CoroutineScheduler`.

To call these functions on a specific coroutine, use the `Coroutine` instance
variable that was created using the `COROUTINE()` macro:

```
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
```
  ...
  if (condition) COROUTINE_YIELD();
  ...
```

### Custom Coroutines

All coroutines are instances of the `Coroutine` class, or one of its subclasses.
You can create custom subclasses of `Coroutine` and create coroutines which are
instances of the custom class. Use the 2-argument version of the `COROUTINE()`
macro like this:
```
class CustomCoroutine : public Coroutine {
  public:
    void enable(bool isEnabled) { enabled = isEnabled; }

    // the run() method will be defined by the COROUTINE() macro

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

Custom coroutines are useful if you need to create multiple coroutines which
share methods or data structures.

### Manual Coroutines

An manual coroutine is a custom coroutine whose body of the coroutine (i.e
the`run()` method) is defined manually and the coroutine object is also
instantiated manually, instead of using the `COROUTINE()` macro. This is useful
if the coroutine has external dependencies which need to be injected into the
constructor. The `COROUTINE()` macro does not allow the constructor to be
customized.

```
class ManualCoroutine : public Coroutine {
  public:
    // Inject external dependencies into the constructor.
    ManualCoroutine(...) {
      ...
    }

  private:
    virtual int run() override {
      COROUTINE_BEGIN();
      // insert coroutine code here
      COROUTINE_END();
    }
};

ManualCoroutine manualRoutine;
```

A manual coroutine (created without the `COROUTINE()` macro) is *not*
automatically added to the linked list used by the `CoroutineScheduler`. If you
wish to insert it into the scheduler, use the `resume()` method just before
calling `CoroutineScheduler::setup()`:
```
void setup() {
  ...
  manualRoutine.resume();
  CoroutineScheduler::setup();
  ...
}

void loop() {
  ...
  CoroutineScheduler::loop();
  ...
}
```

The `Coroutine::resume()` method can be called at anytime to insert into the
scheduler, but calling it in the global `setup()` makes things simple.

The name of a manually created coroutine is set to be `nullptr` because the
`COROUTINE()` macro was not used. When printed (e.g. using the
`CoroutineScheduler::list()` method), the name is represented by the integer
representation of the `this` pointer of the coroutine object.

A good example of a manual coroutine is
[src/ace_routine/cli/CommandDispatcher.h](src/ace_routine/cli/CommandDispatcher.h).

### External Coroutines

A coroutine can be defined in a separate `.cpp` file. However, if you want to
refer to an externally defined coroutine, you must provide an `extern`
declaration for that instance. The macro that makes this easy is
`EXTERN_COROUTINE()`.

For example, supposed we define a coroutine named `external` like
this in a `External.cpp` file:
```
COROUTINE(external) {
  ...
}
```

To use this in `Main.ino` file, we must use the `EXTERN_COROUTINE()` macro like
this:
```
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
```
COROUTINE(CustomCoroutine, external) {
  ...
}
```

then this in `Main.ino`:
```
EXTERN_COROUTINE(CustomCoroutine, external);

COROUTINE(doSomething) {
  ...
  if (!external.isDone()) COROUTINE_DELAY(1000);
  ...
}
```

### Communication Between Coroutines

The AceRoutine library does not provide any internal mechanism to
pass data between coroutines. Here are some options:

* The easiest method is to use **global variables** which are modified by
  multiple coroutines.
* You can subclass the `Coroutine` class and define a class static variables
  which can be shared among coroutines which inherit this custom class
* You can define methods on the custom Coroutine class, and pass messages back
  and forth between coroutines using these methods.

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
    * sizeof(Coroutine): 14
    * sizeof(CoroutineScheduler): 2
* 32-bit processors (e.g. Teensy ARM, ESP8266, ESP32)
    * sizeof(Coroutine): 28
    * sizeof(CoroutineScheduler): 4

In other words, you can create 100 `Coroutine` instances and they would use only
1400 bytes of static RAM on an 8-bit AVR processor.

The `CoroutineScheduler` consumes only 2 bytes of memory no matter how many
coroutines are created. That's because it depends on a singly-linked list whose
pointers live on the `Coroutine` object, not in the `CoroutineScheduler`.

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
