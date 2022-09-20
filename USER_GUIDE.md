# AceRoutine User Guide

See the [README.md](README.md) for installation instructions and other
background information. This document describes how to use the library once it
is installed.

**Version**: 1.5.1 (2022-09-20)

## Table of Contents

* [Coroutine Setup](#Setup)
    * [Include Header and Namespace](#IncludeHeader)
    * [Macros](#Macros)
    * [Overall Structure](#OverallStructure)
    * [Coroutine Class](#CoroutineClass)
    * [Coroutine Instance](#CoroutineInstance)
    * [Coroutine Names](#CoroutineNames)
* [Coroutine Body](#CoroutineBody)
    * [Begin and End Markers](#BeginAndEnd)
    * [Yield](#Yield)
    * [Await](#Await)
    * [Delay](#Delay)
    * [Local Variables](#LocalVariables)
    * [Conditional If-Else](#IfElse)
    * [Switch Statements](#Switch)
    * [For Loops](#ForLoops)
    * [While Loops](#WhileLoops)
    * [Forever Loops](#ForeverLoops)
    * [Macros As Statements](#MacrosAsStatements)
    * [Chaining Coroutines](#ChainingCoroutines)
* [Running and Scheduling](#RunningAndScheduling)
    * [Direct Scheduling](#DirectScheduling)
    * [CoroutineScheduler](#CoroutineScheduler)
    * [Direct Scheduling or CoroutineScheduler](#DirectOrAutomatic)
    * [Suspend and Resume](#SuspendAndResume)
    * [Reset Coroutine](#Reset)
    * [Coroutine States](#States)
* [Customizing](#Customizing)
    * [Custom Coroutines](#CustomCoroutines)
    * [Manual Coroutines](#ManualCoroutines)
    * [Coroutine Setup](#CoroutineSetup)
* [Coroutine Profiling](#CoroutineProfiling)
    * [Creating Profilers Manually](#CreatingProfilersManually)
    * [Creating Profilers Automatically](#CreatingProfilersAutomatically)
    * [Running Coroutine With Profiler](#RunningCoroutineWithProfiler)
    * [Running Scheduler With Profiler](#RunningSchedulerWithProfiler)
    * [Rendering the Profiler Results](#RenderingProfilerResults)
    * [Profiler Resource Consumption](#ProfilerResourceConsumption)
* [Coroutine Communication](#Communication)
    * [Instance Variables](#InstanceVariables)
    * [Channels (Experimental)](#Channels)
* [Miscellaneous](#Miscellaneous)
    * [Comparison To NonBlocking Function](#ComparisonToNonBlockingFunction)
    * [External Coroutines](#External)
    * [Functors](#Functors)
* [Bugs and Limitations](#BugsAndLimitations)
    * [No Nested LOOP Macro](#NoNestedLoop)
    * [No Delegation to Regular Functions](#NoDelegation)
    * [No Creation on Heap](#NoCreationOnHeap)

<a name="Setup"></a>
## Coroutine Setup

<a name="IncludeHeader"></a>
### Include Header and Namespace

Only a single header file `AceRoutine.h` is required to use this library.
To prevent name clashes with other libraries that the calling code may use, all
classes are defined in the `ace_routine` namespace. To use the code without
prepending the `ace_routine::` prefix, use the `using` directive:

```C++
#include <AceRoutine.h>
using namespace ace_routine;
```

There are only 3 classes in this namespace (`Coroutine`, `CoroutineScheduler`,
and `Channel`) so you may also just import one or more of those classes
directly:

```C++
#include <AceRoutine.h>
using ace_routine::Coroutine;
```

<a name="Macros"></a>
### Macros

The following macros are available to hide a lot of boilerplate code:

* `COROUTINE()`: defines an instance of `Coroutine` class or a user-provided
  custom subclass of `Coroutine`
* `COROUTINE_BEGIN()`: must occur at the start of a coroutine body
* `COROUTINE_END()`: must occur at the end of the coroutine body
* `COROUTINE_YIELD()`: yields execution back to the caller
* `COROUTINE_AWAIT(condition)`: yields until `condition` become `true`
* `COROUTINE_DELAY(millis)`: yields back execution for `millis`. The maximum
  allowable delay is 32767 milliseconds.
* `COROUTINE_DELAY_MICROS(micros)`: yields back execution for `micros`. The
  maximum allowable delay is 32767 microseconds.
* `COROUTINE_DELAY_SECONDS(seconds)`: yields back execution for `seconds`. The
  maximum allowable delay is 32767 seconds.
* `COROUTINE_LOOP()`: convenience macro that loops forever, replaces
  `COROUTINE_BEGIN()` and `COROUTINE_END()`
* `COROUTINE_CHANNEL_WRITE()`: writes a message to a `Channel`
* `COROUTINE_CHANNEL_READ()`: reads a message from a `Channel`

<a name="OverallStructure"></a>
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

<a name="CoroutineClass"></a>
### Coroutine Class

The `Coroutine` class looks something like this (not all public methods shown):

```C++
class Coroutine {
  public:
    virtual int runCoroutine() = 0;

    virtual void setupCoroutine() {}

    void suspend();

    void resume();

    void reset();

    bool isSuspended() const;

    bool isYielding() const;

    bool isDelaying() const;

    bool isRunning() const;

    bool isEnding() const;

    bool isTerminated() const;

    bool isDone() const;
};
```

<a name="CoroutineInstance"></a>
### Coroutine Instance

All coroutines are instances of the `Coroutine` class or one of its
subclasses. There are 2 recommended ways of creating coroutines:

* Using the `COROUTINE()` macro
* Manually subclassing the `Coroutine` class.

(The third option is useful mostly for unit testing purposes, and is explained
later in the [Custom Coroutines](#CustomCoroutines) section below.)

**Using COROUTINE() Macro**

The name of the coroutine instance is the name provided
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
`Coroutine_doSomething` but it is unlikely that you will need to know the exact
name of this generated class.

If you expand the `COROUTINE()` macro from `Coroutine.h`, the above code is
equivalent to writing out the following by hand:

```C++
class Coroutine_doSomething : public Coroutine {
  public:
    Coroutine_doSomething() {}

    int runCoroutine() override {
        COROUTINE_BEGIN();
        ...
        COROUTINE_END();
    }
};

Coroutine_doSomething doSomething;
```

**Manually Subclassing the Coroutine Class**

After seeing how the `COROUTINE()` macro expands out the code, it should
relatively straightforward to see how we can create our own subclasses of
`Coroutine` class, and create multiple instances of that subclass:

```C++
class MyCoroutine : public Coroutine {
  public:
    int runCoroutine() override {
        COROUTINE_BEGIN();
        ...
        COROUTINE_END();
    }
};


MyCoroutine routine1;
MyCoroutine routine2;
```

For more details on manual Coroutine instances, see the
[Manual Coroutines](#ManualCoroutines) section below.

<a name="CoroutineNames"></a>
### Coroutine Names

(Added in v1.5. Prior to v1.3, the name was automatically assigned to the
coroutine using the `COROUTINE()` macro. It was removed in v1.3 to save flash
memory. In v1.5, the feature is added back as an optional feature so that the
end-user can decide whether they want to spend the extra flash storage for this
feature.)

A coroutine can be assigned a human-readable name through the following methods:

```C++
class Coroutine {
  public:
    static const uint8_t kNameTypeCString = 0;
    static const uint8_t kNameTypeFString = 1;

  public:
    void Coroutine::setName(const char* name);
    void Coroutine::setName(const __FlashStringHelper* name);

    const char* Coroutine::getCName() const;
    const __FlashStringHelper* Coroutine::getFName() const;

    uint8_t getNameType() const;
    void printNameTo(Print& printer) const;
};
```

It is expected that the `setName()` will be called in the global `setup()`
function.

On most 32-bit processors, it makes little difference whether a C-string or an
F-string is used. (The exception is the ESP8266.) On AVR processors, using the
F-string will prevent those strings from consuming precious static RAM.

The `printNameTo()` method prints the coroutine name to the given `Print`
object, which will usually be the `Serial` object. If the name is not set (hence
is the `nullptr`), `printNameTo()` will print the hexadecimal representation of
the pointer to the Coroutine (e.g. "0xE38A").

The `CoroutineScheduler::list()` method will now print the coroutine name if it
is defined.

<a name="CoroutineBody"></a>
## Coroutine Body

The code immediately following the `COROUTINE()` macro becomes the body of the
`Coroutine::runCoroutine()` virtual method. Within this `runCoroutine()` method,
various helper macros (e.g. `COROUTINE_BEGIN()`, `COROUTINE_YIELD()`,
`COROUTINE_DELAY()`, etc) can be used. These helper macros are described below.

<a name="BeginAndEnd"></a>
### Begin and End Markers

Within the `COROUTINE()` macro, the beginning of the coroutine code must start
with the `COROUTINE_BEGIN()` macro and the end of the coroutine code must end
with the `COROUTINE_END()` macro. They initialize various bookkeeping variables
in the `Coroutine` class that enable coroutines to be implemented. All other
`COROUTINE_xxx()` macros must appear between these BEGIN and END macros.

The `COROUTINE_LOOP()` macro is a special case that replaces the
`COROUTINE_BEGIN()` and `COROUTINE_END()` macros. See the
[Forever Loops](#ForeverLoops) section below.

<a name="Yield"></a>
### Yield

`COROUTINE_YIELD()` returns control to the `CoroutineScheduler` which is then
able to run another coroutines. Upon the next iteration, execution continues
just after `COROUTINE_YIELD()`. (Technically, the execution always begins at the
top of the function, but the `COROUTINE_BEGIN()` contains a dispatcher that
gives the illusion that the execution continues further down the function.)

<a name="Await"></a>
### Await

`COROUTINE_AWAIT(condition)` yields until the `condition` evaluates to `true`.
This is a convenience macro that is identical to:
```C++
while (!condition) COROUTINE_YIELD();
```

<a name="Delay"></a>
### Delay

**Delay Milliseconds**

The `COROUTINE_DELAY(millis)` macro yields back control to other coroutines
until `millis` milliseconds have elapsed. This is analogous to the built-in
Arduino `delay()` function, except that this is non-blocking. The following
waits for 100 milliseconds:

```C++
COROUTINE(waitMillis) {
  COROUTINE_BEGIN();
  ...
  COROUTINE_DELAY(100);
  ...
  COROUTINE_END();
}
```

The `millis` argument is a `uint16_t`, a 16-bit unsigned integer, which saves
the size of each coroutine instance by 4 bytes (8-bit processors) or 8 bytes
(32-bit processors) compared to using an `uint32_t`. The largest number that
can be represented by a `uint16_t` is 65535, but the actual maximum delay is
limited to 32767 milliseconds to avoid overflow situations if the other
coroutines in the system take too much time for their work before returning
control to the waiting coroutine. With this limit, the other coroutines have as
much as 32767 milliseconds before it must yield, which should be more than
enough time for any conceivable situation. In practice, coroutines should
complete their work within several milliseconds and yield control to the other
coroutines as soon as possible.

**Delay Microseconds**

On faster microcontrollers, it might be useful to yield for microseconds using
the `COROUTINE_DELAY_MICROS(delayMicros)`.  The following example waits for 300
microseconds:

```C++
COROUTINE(waitMicros) {
  COROUTINE_BEGIN();
  ...
  COROUTINE_DELAY_MICROS(300);
  ...
  COROUTINE_END();
}
```
This macro has a number constraints:

* The maximum delay is 32767 micros.
* All other coroutines in the program *must* yield within 32767 microsecond,
  otherwise the internal timing variable will overflow and an incorrect delay
  will occur.
* The accuracy of `COROUTINE_DELAY_MICROS()` is not guaranteed because the
  overhead of context switching and checking the delay's expiration may
  consume a significant portion of the requested delay in microseconds.

**Delay Seconds**

For delays greater than 32767 milliseconds, we can use the
`COROUTINE_DELAY_SECONDS(seconds)` convenience macro. The following example
waits for 200 seconds:

```C++
COROUTINE(waitSeconds) {
  COROUTINE_BEGIN();
  ...
  COROUTINE_DELAY_SECONDS(200);
  ...
  COROUTINE_END();
}
```

This macro has some constraints and caveats:

* The maximum number of seconds is 32767 seconds.
* The delay is implemented using the `millis()` clock divided by 1000.
    * On 8-bit processors without hardware division instruction, the software
      division consumes CPU time and flash memory, about 100 bytes on an AVR.
    * The `unsigned long` returned by `millis()` rolls over every
      4294967.296 seconds (49.7 days). During that rollover, the
      `COROUTINE_DELAY_SECONDS()` will return 0.704 seconds too early. If the
      delay value is relatively large, e.g. 100 seconds, this inaccuracy
      probably won't matter too much.

We can also use an explicit for-loop. For example, to delay for 1000 seconds, we
can loop 100 times over a `COROUTINE_DELAY()` of 10 seconds, like this:

```C++
COROUTINE(waitThousandSeconds) {
  static uint16_t i;

  COROUTINE_BEGIN();
  for (i = 0; i < 100; i++) {
    COROUTINE_DELAY(10000); // 10 seconds
  }
  ...
  COROUTINE_END();
}
```

See [For Loops](#ForLoops) section below for a description of the for-loop
construct.

<a name="LocalVariables"></a>
### Local Variables

Each coroutine is stackless. More accurately, the stack of the coroutine
is destroyed and recreated on every invocation of the coroutine. Therefore,
any local variable created on the stack in the coroutine will not preserve
its value after a `COROUTINE_YIELD()` or a `COROUTINE_DELAY()`.

The problem is worse for local *objects* (with non-trivial destructors). If the
lifetime of the object straddles a continuation point of the Coroutine
(`COROUTINE_YIELD()`, `COROUTINE_DELAY()`, `COROUTINE_END()`), the destructor of
the object will be called incorrectly when the coroutine is resumed, and will
probably crash the program. In other words, do **not** do this:

```C++
COROUTINE(doSomething) {
  COROUTINE_BEGIN();
  String s = "hello world"; // ***crashes when 'doSomething' is resumed***
  Serial.println(s);
  COROUTINE_DELAY(1000);
  ...
  COROUTINE_END();
}
```

Instead, place any local variable or object completely inside a `{ }` block
before the `COROUTINE_YIELD()` or `COROUTINE_DELAY()`, like this:

```C++
COROUTINE(doSomething) {
  COROUTINE_BEGIN();
  {
    String s = "hello world"; // ok, because String is properly destroyed
    Serial.println(s);
  }
  COROUTINE_DELAY(1000);
  ...
  COROUTINE_END();
}
```

One way around these problem is to avoid local variables and use `static`
variables inside a `COROUTINE()`. Function-static variables are initialized once
and preserve their value through multiple calls to the function, which is
exactly what is needed.

For Manual Coroutines created from your own subclass, using a function-static
variable inside the `runCoroutine()` method may not be an option if you create
multiple instances. This is because the function-static variable will be shared
among multiple instances which may conflict with each other. Instead, you can
add a private member variable to the custom class and treat it like a local
variable inside the `runCoroutine()` function, like this:

```C++
class MyCoroutine : public Coroutine {
  public:
    MyCoroutine(int val):
        internal(val) {
      ...
    }

    int runCoroutine() override {
      COROUTINE_LOOP() {
        ...
        internal++; // operate on 'internal' variable
        ...
        COROUTINE_YIELD();
      }
    }

  private:
    int internal;
};

MyCoroutine a(1);
MyCoroutine b(2);
```

<a name="IfElse"></a>
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

<a name="Switch"></a>
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

<a name="ForLoops"></a>
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

<a name="WhileLoops"></a>
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

<a name="ForeverLoops"></a>
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

<a name="MacrosAsStatements"></a>
### Macros As Statements

The various macros (`COROUTINE_YIELD()`, `COROUTINE_DELAY()`,
`COROUTINE_AWAIT()`, etc.) have been designed to allow them to be used almost
everywhere a valid C/C++ statement is allowed. For example, the following is
allowed:

```C++
  ...
  if (condition) COROUTINE_YIELD();
  ...
```

<a name="ChainingCoroutines"></a>
### Chaining Coroutines

Coroutines can be chained, in other words, the `runCoroutine()` of one coroutine
*can* explicitly call the `runCoroutine()` of another coroutine.

I have found it useful to chain coroutines when using the [Manual
Coroutines](#ManualCoroutines)
described in the section below. The ability to chain coroutines allows us to
implement a [Decorator
Pattern](https://en.wikipedia.org/wiki/Decorator_pattern), also known as "a
chain of responsibility". Using manual coroutines, we can wrap one coroutine
with another and delegate to the inner coroutine like this:

```C++
class InnerCoroutine: public Coroutine {
  public:
    InnerCoroutine(..) { ...}

    int runCoroutine override {
      COROUTINE_BEGIN();
      ...
      COROUTINE_END();
      ...
    }
};

class OuterCoroutine: public Coroutine {
  public:
    OuterCoroutine(InnerCoroutine& inner):
        mInner(inner) {
      ...
    }

    int runCoroutine override {
      // No COROUTINE_BEGIN() and COROUTINE_END() needed if this simply
      // delegates to the InnerCoroutine.
      mInner.runCoroutine();
    }

  private:
    Coroutine& mInner;
};

```
In situations like this, it is likely only the `OuterCoroutine` would be
registered in the `CoroutineScheduler` since we do not want to call the
`InnerCoroutine` directly. And in the cases that I've come across, the
`OuterCoroutine` doesn't actually use much of the Coroutine functionality (i.e.
it doesn't actually use the `COROUTINE_BEGIN()` and `COROUTINE_END()` macros. It
simply delegates the `runCoroutine()` call to the inner one.

This type of chaining is also allowed for coroutines defined using the
`COROUTINE()` macro, like this:

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

But I have yet to come across a situation where this was useful.

<a name="RunningAndScheduling"></a>
## Running and Scheduling

There are 2 ways to run the coroutines:
* manually calling the coroutines in the `loop()` method, or
* automatically scheduling and running them using the `CoroutineScheduler`.

<a name="DirectScheduling"></a>
### Direct Scheduling

If you have only a small number of coroutines, the manual method is the
easiest and fastest way. This requires you to explicitly call the
`runCoroutine()` method of all the coroutines that you wish to run in the
`loop()` method, like this:

```C++
void loop() {
  blinkLed.runCoroutine();
  printHello.runCoroutine();
  printWorld.runCoroutine();
}
```

Because the `runCoroutine()` method is called directly, instead of through the
`Coroutine` pointer, the call does *not* suffer the overhead of the `virtual`
dispatch. It is as if the `virtual` keyword did not exist.

<a name="CoroutineScheduler"></a>
### CoroutineScheduler

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

**Historical Notes**:

Prior to v1.2, the initial ordering was sorted by the `Coroutine::getName()`.
And calling `suspend()` would remove the coroutine from the internal list
of coroutines, and `resume()` would add the coroutine back into the list.
This behavior turned out to be
[fatally flawed](https://github.com/bxparks/AceRoutine/issues/19)

Starting with v1.2, the ordering of the coroutines in the internal list is
officially undefined. As well, the actual properties of the coroutine list is
also considered to be an internal implementation detail that may change in the
future. Client code should not depend on the implementation details of this
internal list.

Prior to v1.3, if you manually subclass the `Coroutine` class to create your own
[Manual Coroutines](#ManualCoroutines), the `Coroutine::setupCoroutine()` method
must be called in the global `setup()` so that the coroutine instance is added
to the `CoroutineScheduler`. In v1.3, calling `setupCoroutine()` is no longer
necessary because the `Coroutine::Coroutine()` constructor automatically inserts
itself into the internal singly-linked list. The `setupCoroutine()` is retained
for backwards compatibility, but is now marked deprecated.

<a name="DirectOrAutomatic"></a>
### Direct Scheduling or CoroutineScheduler

Direct scheduling has the smallest context switching overhead between
coroutines. However, it is not possible to use `Coroutine::suspend()` or
`Coroutine::resume()` (see below) because those methods change states which are
used only by the `CoroutineScheduler`. Each time you create a new coroutine, you
must remember to call its `runCoroutine()` method from the global `loop()`
function.

Using the `CoroutineScheduler` to call `Coroutine::runCoroutine()` is easier
because the `CoroutineScheduler` automatically keeps track of all coroutines
defined by the `COROUTINE()` macro, even if they are defined in multiple files.
The `CoroutineScheduler` also allows coroutines to be suspended and resumed
using the `Coroutine::suspend()` and `Coroutine::resume()` methods. However,
using the `CoroutineScheduler` consumes more memory resources and CPU overhead:

* The `CoroutineScheduler` needs to walk down a linked list of `Coroutine`
  instances to find the next one.
* The `CoroutineScheduler` calls `Coroutine::runCoroutine()` through the
  `Coroutine` pointer, which causes the `virtual` method dispatch to be used.
  That consumes several extra cycles of CPU (a few microseconds) and extra
  memory (probably in the 20-40 byte range).

The [MemoryBenchmark](examples/MemoryBenchmark) results show that using a
`CoroutineScheduler` consumes about 30-70 extra bytes of flash memory per
`Coroutine` instance, compared to directly calling the
`Coroutine::runRoutine()`.

My recommendation is that on 8-bit processors (e.g. Arduino Nano, Uno, SparkFun
ProMicro) with limited memory, the Direct Scheduling should be used where
`Coroutine::runCoroutine()` is directly called from the global `loop()`. On
32-bit processors with enough flash memory, the `CoroutineScheduler` can be used
if you want the convenience and extra flexibility that `CoroutineScheduler`, and
you don't mind the extra flash memory and CPU overhead.

<a name="SuspendAndResume"></a>
### Suspend and Resume

The `Coroutine::suspend()` and `Coroutine::resume()` methods are available
*only* if the `CoroutineScheduler` is used. If the coroutines are called
explicitly in the global `loop()` method, then these methods have no impact.

The `Coroutine::suspend()` and `Coroutine::resume()` **should not** be called
from inside the coroutine. Fortunately, if they are accidentally called,
they will have no effect. They must be called from outside of the coroutine.
When a coroutine is suspended,  the `CoroutineScheduler` will skip over this
coroutine and `Coroutine::runCoroutine()` will not be called.

As of v1.2, it is not possible to suspend a coroutine from inside itself. I have
some ideas on how to fix this in the future.

I have personally never needed to use `suspend()` and `resume()` so this
functionality may not be tested well. See for example
[Issue #19](https://github.com/bxparks/AceRoutine/issues/19).

<a name="Reset"></a>
### Reset Coroutine

A coroutine can be reset to its initial state using the `Coroutine::reset()`
method so that the next time `runCoroutine()` is called, it begins execution
from the start of that method instead of the most recent continuation point
(i.e. after a `COROUTINE_YIELD()`, `COROUTINE_DELAY()`, etc).

If the coroutine object has any other state variables, for example, additional
member variables of the Coroutine subclass, or static variables inside the
`runCoroutine()` method, you may need to manually reset those variables to their
initial states as well.

I have not personally needed the `reset()` functionality so it has not been
tested as much as I would like, but it is apparently useful for some people. See
for example:

* [Issue #13](https://github.com/bxparks/AceRoutine/issues/13)
* [Issue #14](https://github.com/bxparks/AceRoutine/issues/14)
* [Issue #20](https://github.com/bxparks/AceRoutine/issues/20)

A good example of how to use the `reset()` can be seen in
[examples/SoundManager](examples/SoundManager).

<a name="States"></a>
### Coroutine States

A coroutine has several internal states:
* `kStatusSuspended`: coroutine was suspended using `Coroutine::suspend()`
* `kStatusYielding`: coroutine returned using `COROUTINE_YIELD()` or
  `COROUTINE_AWAIT()`. This is also the initial state of a new coroutine,
  or an old coroutine after `reset()`.
* `kStatusDelaying`: coroutine returned using `COROUTINE_DELAY()`
* `kStatusRunning`: coroutine is currently running
* `kStatusEnding`: coroutine returned using `COROUTINE_END()`
* `kStatusTerminated`: coroutine is permanently terminated. Set only by the
  `CoroutineScheduler`.

The finite state diagram looks like this:
```
                     <--------------------------+
         Suspended                              |
         ^       ^                              |
        /         \                             |
       /           \                            |
      v             \       <------+            |
Yielding          Delaying         |            |
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
          Ending                   |            |
             |              <------+            |
             |                                  |
             v                                  |
        Terminated                              |
                    <---------------------------+
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
  is preferred because it works when the `Coroutine::runCoroutine()` is executed
  directly or through the `CoroutineScheduler`.

Prior to v1.2, there was a small operational difference between `kStatusEnding`
and `kStatusTerminated`. A terminated coroutine was removed from the internal
linked list of "active" coroutines that was managed by the `CoroutineScheduler`.
However, there was a serious flaw with this design
([Issue #19](https://github.com/bxparks/AceRoutine/issues/19))
so with v1.2, there is now no practical difference between these 2 states. It is
possible that a future design change (something I am noodling over in my mind)
may reintroduce a difference. Regardless, I recommended that the `isDone()`
method should be used to detect a coroutine that has "finished".

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

<a name="Customizing"></a>
## Customizing

<a name="CustomCoroutines"></a>
### Custom Coroutines (Not Recommended)

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
coroutines which share methods or data structures. In practice, the only place
where I have found this feature to be useful is in writing the
[tests/AceRoutineTest](tests/AceRoutineTest) unit tests. In any other normal
situation, I suspect that the **Manual Coroutines** section described in
below will be more useful and easier to understand.

<a name="ManualCoroutines"></a>
### Manual Coroutines (Recommended)

A manual coroutine is a custom coroutine whose body of the coroutine (i.e
the `runCoroutine()` method) is defined manually and the coroutine object is
also instantiated manually, instead of using the `COROUTINE()` macro. This is
useful if the coroutine has external dependencies which need to be injected into
the constructor. The `COROUTINE()` macro does not allow the constructor to be
customized.

```C++
class ManualCoroutine : public Coroutine {
  public:
    // Inject external dependencies into the constructor.
    ManualCoroutine(Params, ..., Objects, ...) {
      ...
    }

    int runCoroutine() override {
      COROUTINE_BEGIN();
      // insert coroutine code here
      COROUTINE_END();
    }
};

ManualCoroutine manualRoutine(params, ..., objects, ...);
```

The `Coroutine::Coroutine()` constructor automatically adds itself to the
internal list of coroutines, so manually created coroutines are available to the
`CoroutineScheduler` automatically just like the coroutines defined by the
`COROUTINE()` macro.

Some examples of manual coroutines:

* [BlinkSlowFastManualRoutine](examples/BlinkSlowFastManualRoutine) which shows
  the same functionality as
  [BlinkSlowFastRoutine](examples/BlinkSlowFastRoutine)
* [HelloManualCoroutine](examples/HelloManualCoroutine) which shows the same
  functionality as [HelloCoroutine](examples/HelloCoroutine).
* [SoundManager](examples/SoundManager) which uses both the automatic coroutine
  defined by `COROUTINE()` macro and an explicitly subclasses manual coroutine.

<a name="CoroutineSetup"></a>
### Coroutine Setup

When creating custom subclasses through the Manual Coroutine workflow (see
above), you have the ability to override the `setupCoroutine()` method if you
need to:

```C++
class ManualCoroutine : public Coroutine {
  public:
    // Inject external dependencies into the constructor.
    ManualCoroutine(Params, ..., Objects, ...) {
      ...
    }

    int runCoroutine() override {
      COROUTINE_BEGIN();
      // insert coroutine code here
      COROUTINE_END();
    }

    void setupCoroutine() override {
      ...
    }
};
```

If you have only a few coroutines that need to override the `setupCoroutine()`,
it is probably easiest to just call it directly from the global `setup()`:

```C++
ManualCoroutine1 coroutine1;
ManualCoroutine2 coroutine2;

void setup() {
  coroutine1.setupCoroutine();
  coroutine2.setupCoroutine();

  CoroutineScheduler::setup();
  ...
}
```

If you have significant number of coroutines, or if you have enough flash and
static memory that you don't need to worry about memory consumption, then you
can call the `CoroutineScheduler::setupCoroutines()`. It will loop through the
list of coroutines, and call the `setupCoroutine()` method of each coroutine
automatically:

```C++
ManualCoroutine1 coroutine1;
ManualCoroutine2 coroutine2;

void setup() {
  CoroutineScheduler::setupCoroutines();

  CoroutineScheduler::setup();
  ...
}
```

You need to call `CoroutineScheduler::setupCoroutines()` explicitly if you want
it. The `CoroutineScheduler::setup()` method does *not* call `setupCoroutines()`
automatically in order to save flash memory if the feature is not used.

**Warning**: The `Coroutine::setupCoroutine()` can consume significant amounts
of memory, especially on AVR processors. On AVR processors, each
`setupCoroutine()` overridden in the subclass consumes at least 50-60 bytes of
flash per coroutine. On 32-bit processors, it takes slightly less memory, about
30-40 bytes per coroutine. The looping code in
`CoroutineScheduler::setupCoroutines()` consumes about 20-30 bytes of flash on
AVR processors. The virtual dispatch on `Coroutine::setupCoroutine()` consumes
about 14 bytes of flash per invocation.

<a name="CoroutineProfiling"></a>
## Coroutine Profiling

Version 1.5 added the ability to profile the execution time of
`Coroutine::runCoroutine()` and render the information as a formatted table, or
as a JSON object. (Thanks to peufeu2@ who proposed the idea and provided the
initial proof of concept in
[Discussion#50](https://github.com/bxparks/AceRoutine/discussions/50)).

If the profiling feature is not used, no additional flash memory is consumed.
The static RAM usage does increase by 2 bytes (8-bits) and 4 bytes (32-bits) per
coroutine even if this feature is not used. The feature seemed useful enough to
accept this small increase in static memory size, because most applications will
not use more than 5-10 coroutines, and that translates into only 10-20 bytes of
additional static RAM usage on 8-bit processors.

The following classes and API methods were added to support the profiling
feature. The `CoroutineProfiler` class is an interface whose
`updateElapsedMicros()` should be called with the execution time of the
`Coroutine::runCoroutine()` method:

```C++
class CoroutineProfiler {
  public:
    virtual void updateElapsedMicros(uint32_t micros) = 0;
};
```

Each `Coroutine` object has the ability to hold a pointer to a
`CoroutineProfiler` object:

```C++
class Coroutine {
  ...
  public:
    void setProfiler(CoroutineProfiler* profiler);
    CoroutineProfiler* getProfiler() const;

    int runCoroutineWithProfiler();
  ...
};
```

The `runCoroutineWithProfiler()` method calls `runCoroutine()`, measures the
elapsed time in microseconds, then calls the `profiler->updateElapsedMicros()`.

**Note**: When creating Coroutines with profiling enabled, it will probably be
necessary to assign human-readable names to each coroutine for identification
purposes. See [Coroutine Names](#CoroutineNames) for information on the
`setName()`, `getCName()`, `getFName()`, `getNameType()`, and `printNameTo()`
methods. Each coroutine name will consume additional flash memory.

Currently only a single implementation of `CoroutineProfiler` is provided, the
`LogBinProfiler`. It contains 32 bins of `uint16_t` which track the number of
times a `micros` was seen. The bins are logarithmically scaled, so that Bin 0
collects all events `<2us`, Bin 1 collects events `<4us`, Bin 2 collects events
`<8us`, ..., Bin 30 collects events `<2147s`, and the last Bin 31 collects
events `<4295s`.

```C++
class LogBinProfiler : public CoroutineProfiler {
  public:
    static const uint8_t kNumBins = 32;

  public:
    LogBinProfiler();

    void updateElapsedMicros(uint32_t micros) override;
    void clear();

    static void createProfilers();
    static void deleteProfilers();
    static void clearProfilers();

  public:
    uint16_t mBins[kNumBins];
};
```

Details on how to configure and use these are are provided below, but it may
help to look at 2 examples while looking through the following subsections:

* [HelloCoroutineWithProfiler](examples/HelloCoroutineWithProfiler)
* [HelloSchedulerWithProfiler](examples/HelloSchedulerWithProfiler)

<a name="CreatingProfilersManually"></a>
### Creating Profilers Manually

By default, a `Coroutine` has no reference to a `CoroutineProfiler`. The user
can directly assign a profiler instance by creating it statically, then calling
`Coroutine::setProfiler()` like this:

```C++
#include <AceRoutine.h>
using namespace ace_routine;

COROUTINE(coroutine1) {
  ...
}

COROUTINE(coroutine2) {
  ...
}

LogBinProfiler profiler1;
LogBinProfiler profiler2;

void setup() {
  ...
  coroutine1.setProfiler(&profiler1);
  coroutine2.setProfiler(&profiler2);
  ...
}
```

This technique works well if you have a small number of coroutines.

<a name="CreatingProfilersAutomatically"></a>
### Creating Profilers Automatically

If you are using a substantial number of coroutines, it is cumbersome to
manually create these profilers for all coroutines. In that case, you can use
the `LogBinProfiler::createProfilers()` convenience function which loops through
every coroutine and creates an instance of `LogBinProfiler` on the heap:

```C++
#include <AceRoutine.h>
using namespace ace_routine;

COROUTINE(coroutine1) {
  ...
}

COROUTINE(coroutine2) {
  ...
}

...

void setup() {
  ...
  LogBinProfiler::createProfilers();
}
```

In the unlikely event that you need to delete the Profiler instances which were
created on the heap, you can call the `LogBinProfiler::deleteProfilers()` static
method.

Finally, the `LogBinProfiler::clearProfilers()` static method calls the
`LogBinProfiler::clear()` method on every profiler attached to every coroutine
so that the event count in all the bins are cleared to 0.

<a name="RunningCoroutineWithProfiler"></a>
### Running Coroutine with Profiler

Once a `Coroutine` is assigned a `CoroutineProfiler`, the statistics can be
gathered in a couple of ways. The simplest is to call the new
`Coroutine::runCoroutineWithProfiler()` instead of the normal
`Coroutine::runCoroutine()` in the global `loop()` function like this:

```C++
#include <AceRoutine.h>
using namespace ace_routine;

COROUTINE(myCoroutine) {
  ...
}

LogBinProfiler profiler;

void setup() {
  ...
  myCoroutine.setName(F("myCoroutine"));
  myCoroutine.setProfiler(&profiler);
  ...
}

void loop() {
  myCoroutine.runCoroutineWithProfiler(); // <---- instead of runCoroutine()
  ...
}
```

<a name="RunningSchedulerWithProfiler"></a>
### Running Scheduler with Profiler

To activate profiling when using the `CoroutineScheduler`, just replace the call
to `CoroutineScheduler::loop()` with `CoroutineScheduler::loopWithProfiler()`.

```C++
#include <AceRoutine.h>
using namespace ace_routine;

COROUTINE(myCoroutine) {
  ...
}

void setup() {
  ...
  myCoroutine.setName(F("myCoroutine"));
  myCoroutine.setProfiler(&profiler);

  LogBinProfiler::createProfilers();
  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loopWithProfiler(); // <---- instead of loop()
}
```

<a name="RenderingProfilerResults"></a>
### Rendering the Profiler Results

The `LogBinProfiler` comes with 2 rendering classes:

* `LogBinTableRenderer`
    * displays the event count in a human-readable formatted table with fixed
      column widths
* `LogBinJsonRenderer`
    * prints the same info as a JSON object

These classes expose a simple `printTo()` static function like this:

```C++
class LogBinTableRenderer {
  public:
    static void printTo(
        Print& printer,
        uint8_t startBin,
        uint8_t endBin,
        bool clear = true,
        bool rollup = true
    );
};

class LogBinJsonRenderer{
  public:
    static void printTo(
        Print& printer,
        uint8_t startBin,
        uint8_t endBin,
        bool clear = true,
        bool rollup = true
    );
};
```

* The `printer` is usually the `Serial` object, but can be changed to something
  else if needed.
* The `startBin` [0-31] and `endBin` [0-32] identify the bins which should be
  printed.
    * A range of something like [2, 13) is useful to keep the width of the table
      reasonable.
    * Often the bins below and above than this range do not contain any events.
* The `clear` flag (default true) causes the bins to be cleared (through the
  `LogBinProfiler::clear()` method) so that new events can be tracked.
* The `rollup` flag (default true) causes roll up of the exterior bins.
    * Events before `startBin` are added to the first bin.
    * Events at or after `endBin` are added to the last bin (at `endBin-1`)

For example, calling `LogBinTableRenderer::printTo(Serial, 2, 13)` prints
something like this:

```
name         <16us <32us <64us<128us<256us<512us  <1ms  <2ms  <4ms  <8ms    >>
0x1DB        16898 52688     0     0     0     0     0     0     0     0     1
readPin      65535  1128     0     0     0     0     0     0     0     0     0
blinkLed     65535   800     0     0     0     0     0     0     0     0     0
```

And calling `LogBinJsonRenderer::printTo(Serial, 2, 13)` prints something like
this:

```
{
"0x1DB":[16898,52688,0,0,0,0,0,0,0,0,1],
"readPin":[65535,1128,0,0,0,0,0,0,0,0,0],
"blinkLed":[65535,800,0,0,0,0,0,0,0,0,0]
}
```

The `LogBinProfiler` uses a `uint16_t` counter, so the maximum value is
saturated to `65535`.

<a name="ProfilerResourceConsumption"></a>
### Profiler Resource Consumption

The ability to profile the execution time of coroutines does not come for free,
but I have tried to make it as cheap as possible.

**Memory consumption**

If the profiling feature is not wanted, you can continue to use the
`Coroutine::runCoroutine()` or the `CoroutineScheduler::loop()` functions, and
the flash usage will not increase. The only additional resource is 2 extra bytes
(8-bit processors) or 4 extra bytes (32-bit processors) of static RAM, *per
coroutine*, because each coroutine holds a pointer to the `CoroutineProfiler`,
even if it is not used.

If the profiling feature is enabled, the
[MemoryBenchmark](examples/MemoryBenchmark) program shows that:

* Using `Coroutine::runCoroutineWithProfiler()` consumes 50-80 bytes of extra
  bytes of flash *per coroutine* compared to the normal
  `Coroutine::runCoroutine()`, probably due to the virtual dispatch.
* Using `CoroutineScheduler::loopWithProfiler()` consumes an additional 50-100
  bytes of flash compared to using `CoroutineScheduler::loop()`, *plus* the
  additional 50-80 bytes of flash *per coroutine* because the dispatching is
  routed to `Coroutine::runCoroutineWithProfiler()`.
* The `LogBinProfiler` consumes at least 64 bytes of static RAM per instance
  because it holds an array of 32 bin of `uint16_t` integers. It also increases
  the flash usage by 120-150 bytes, but that's a one-time hit, not per profiler
  or coroutine.
* The `LogBinTableRenderer` increases flash usage by 1300-2000 bytes.
* The `LogBinJsonRenderer` increases flash usage by 700-1200 bytes.

**CPU consumption**

The [AutoBenchmark](examples/AutoBenchmark) program shows that calling the
profiler-enabled methods, `Coroutine::runCoroutineWithProfiler()` and
`CoroutineScheduler::loopWithProfiler()`, increases latency by:

* 2.2 - 3.0 micros on AVR
* 0.4 micros on STM32F1
* 0.2 - 0.3 micros on ESP8266
* 0.1 micros on ESP32
* 0.03 - 0.17 micros on Teensy 3.2

On 32-bit processors, the overhead seems neglegible. On 8-bit processors, the 3
microsecond of overhead might be an issue with sensitive applications.

<a name="Communication"></a>
## Coroutine Communication

There are a handful ways that `Coroutine` instances can pass data between
each other.

* The easiest method is to use **global variables** which are modified by
  multiple coroutines.
* To avoid polluting the global namespace, you can subclass the `Coroutine`
  class and define **class static variables** which can be shared among
  coroutines which inherit this custom class
* You can define **methods on the manual Coroutine class**, inject the
  reference/pointer of one coroutine into the constructor of another, and
  call the methods from one coroutine to the other. See skeleton code below.
* You can use **channels** as explained in the next section.

<a name="InstanceVariables"></a>
### Communication Using Instance Variables

```C++
class RoutineA: public Coroutine {
  public:
    int runCoroutine() override {
      COROUTINE_LOOP() {
        ...
      }
    }

    void setState(bool s) {
      state = s;
    }

  private:
    bool state;
};

class RoutineB: public Coroutine {
  public:
    RoutineB(RoutineA& routineACoroutine):
      routineA(routineACoroutine)
    {}

    int runCoroutine() override {
      COROUTINE_LOOP() {
        ...
        routineA.setState(state);
        COROUTINE_YIELD();
      }
    }

  private:
    RoutineA& routineA;
};


RoutineA routineA;
RoutineB routineB(routineA);

void setup() {
  ...
  CoroutineScheduler::setup();
  ...
}

void loop() {
  CoroutineScheduler::loop();
}
```

<a name="Channels"></a>
### Channels (Experimental)

I have included an experimental implementation of channels inspired by the
[Go Lang Channels](https://www.golang-book.com/books/intro/10). The `Channel`
class implements an unbuffered, bidirectional channel. The API and features
of the `Channel` class may change significantly in the future.

Just like Go Lang channels, the AceRoutine `Channel` provides a point of
synchronization between coroutines. In other words, the following sequence of
events is guaranteed when interacting with a channel:

* the writer blocks until the reader is ready,
* the reader blocks until the writer is ready,
* when the writer writes, the reader picks up the message and is allowed
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
```C++
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

    int runCoroutine() override {
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

    int runCoroutine() override {
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
  CoroutineScheduler::setup();
  ...
}

void loop() {
  CoroutineScheduler::loop();
}
```

**Examples**

The CommandLineInterface package in the AceUtils library
(https://github.com/bxparks/AceUtils) uses 2 Coroutines which communicate with
each other using a Channel. One coroutine reads from the `Serial` port, while
the other coroutine writes the output of the command to the `Serial` port.
Neither coroutines are blocking, which allows other coroutines to do other
things.

**Limitations**

* Only a single AceRoutine `Coroutine` can write to a `Channel`.
* Only a single AceRoutine `Coroutine` can read from a `Channel`.
* There is no equivalent of a
  [Go Lang select statement](https://gobyexample.com/select), so the coroutine
  cannot wait for multiple channels at the same time.
* There is no buffered channel type.
* There is no provision to
  [close a channel](https://gobyexample.com/closing-channels).

Some of these features may be implemented in the future if I find compelling
use-cases and if they are easy to implement.

<a name="Miscellaneous"></a>
## Miscellaneous

<a name="ComparisonToNonBlockingFunction"></a>
### Comparison To NonBlocking Function

It is useful to compare a `Coroutine` to a normal, non-blocking delay function
that implements the same algorithm. Here is the Coroutine for blinking the LED
asymmetrically. The HIGH and LOW occurring for different durations:

```C++
COROUTINE(blink) {
  COROUTINE_LOOP() {
    digitalWrite(LED_BUILTIN, HIGH);
    COROUTINE_DELAY(100);
    digitalWrite(LED_BUILTIN, LOW);
    COROUTINE_DELAY(500);
  }
}

void loop() {
  blink.runCoroutine();
}
```

Here is the equivalent code using a non-blocking delay function:

```C++
void blink() {
  static uint16_t prevMillis;
  static uint8_t blinkState;
  const uint8_t kBlinkStateLow = 0;
  const uint8_t kBlinkStateHigh = 1;

  if (blinkState == kBlinkStateHigh) {
    uint16_t nowMillis = millis();
    if ((uint16_t) (nowMillis - prevMillis) >= 100) {
      prevMillis = nowMillis;
      digitalWrite(LED_BUILTIN, LOW);
      blinkState = kBlinkStateLow;
    }
  } else {
    uint16_t nowMillis = millis();
    if ((uint16_t) (nowMillis - prevMillis) >= 500) {
      prevMillis = nowMillis;
      digitalWrite(LED_BUILTIN, HIGH);
      blinkState = kBlinkStateHigh;
    }
  }
}

void loop() {
  blink();
}
```

I think most people would agree that the coroutine version is is far easier to
understand, maintain, and extend to more complex algorithms. According to
[MemoryBenchmark](examples/MemoryBenchmark/), the memory consumption of these
two versions are:

**AVR**
```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Blink Function                  |    938/   14 |   332/    3 |
| Blink Coroutine                 |   1154/   30 |   548/   19 |
+--------------------------------------------------------------+
```

**ESP8266**
```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| Blink Function                  | 257424/26816 |   500/   16 |
| Blink Coroutine                 | 257556/26836 |   632/   36 |
+--------------------------------------------------------------+
```

On the AVR, the coroutine version takes 220 additional bytes of flash which
seems a bit high. But in some situations, it may be worth paying that memory
cost in return for the better maintainability of the code.

<a name="External"></a>
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

For manual coroutines with an explicit `Coroutine` subclass, you can
reference the coroutine instance using the normal C++ mechanism. In other words,
import the header file, then reference the instance:

```C++
// MyCoroutine.h
class MyCoroutine: public Coroutine {
  ...
};

// MyCoroutine.cpp
MyCoroutine myCoroutine;
...

// Application.ino
#include "MyCoroutine.h"
extern MyCoroutine myCoroutine;
...
```

<a name="Functors"></a>
### Functors

C++ allows the creation of objects that look syntactically like functions
by defining the `operator()` method on the class. I have not defined this method
in the `Coroutine` class because I have not found a use-case for it. However, if
someone can demonstrate a compelling use-case, then I would be happy to add it.

<a name="BugsAndLimitations"></a>
## Bugs and Limitations

<a name="NoNestedLoop"></a>
### No Nested LOOP Macro

The `COROUTINE_LOOP()` macro cannot be nested. In other words, the following is
**not** allowed:

```C++
COROUTINE(routine) {
  COROUTINE_LOOP() {
    ...
    if (condition) {
      COROUTINE_LOOP() { // <----- NOT ALLOWED
        ...
        COROUTINE_YIELD();
      }
    }
    COROUTINE_YIELD();
  }
}
```

<a name="NoDelegation"></a>
### No Delegation to Regular Functions

Coroutines macros inside the `runCoroutine()` **cannot** be delegated to another
C/C++ function, even though this becomes tempting when the `runCoroutine()`
implementation becomes complex. In other words, if you call another function
from within the `runCoroutine()`, you cannot use the various `COROUTINE_XXX()`
macros inside the delegated function. The macros were designed to trigger a
compiler error in most cases, but this is not guaranteed:

```C++
void doSomething() {
  ...
  COROUTINE_YIELD(); // <--- ***compiler error***
  ...
}

COROUTINE(cannotUseNestedMacros) {
  COROUTINE_LOOP() {
    if (condition) {
      doSomething(); // <--- doesn't work
    } else {
      COROUTINE_YIELD();
    }
  }
}
```

<a name="NoCreationOnHeap"></a>
### No Creation on Heap

Prior to v1.3, the `Coroutine` class contained a virtual destructor, because I
thought that I would extend this library in the future to support dynamic
creation of coroutines. However, a virtual destructor increases flash memory
usage by 500-600 bytes on 8-bit AVR processors, because it pulls in the
`malloc()` and `free()` functions. On the 32-bit SAMD21 (using the SparkFun
SAMD21 Core), the flash memory increases by ~350 bytes. On other 32-bit
processors (STM32, ESP8266, ESP32, Teensy 3.2), the flash memory increases
modestly, between 50-150 bytes. These flash memory savings, especially on the
AVR processors, is significant, so starting from v1.3, the destructor is now
non-virtual.

If dynamic coroutine on the heap is desired in the future, I think a new class
(e.g. `DynamicCoroutine`) can be created.
