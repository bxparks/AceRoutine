# User Guide

See the [README.md](README.md) for installation instructions and other
background information. This document describes how to use the library once it
is installed.

**Version**: 1.2 (2020-11-10)

**Table of Contents**

* [Coroutine Setup](#Setup)
    * [Include Header and Namespace](#Include)
    * [Macros](#Macros)
    * [Overall Structure](#OverallStructure)
    * [Coroutine Class](#CoroutineClass)
    * [Coroutine Instance](#CoroutineInstance)
* [Coroutine Body](#CoroutineBody)
    * [Begin and End Markers](#BeginAndEnd)
    * [Yield](#Yield)
    * [Await](#Await)
    * [Delay](#Delay)
    * [Local Variables](#LocalVariables)
    * [Conditional If-Else](#IfElse)
    * [Switch Statements](#Switch)
    * [For Loops](#For)
    * [While Loops](#While)
    * [Forever Loops](#Forever)
    * [Macros As Statements](#MacrosAsStatements)
* [Coroutine Chaining](#CoroutineChaining)
    * [No Nested](#NoNested)
    * [Chaining](#Chaining)
* [Running and Scheduling](#RunningAndScheduling)
    * [Manual Scheduling](#ManualScheduling)
    * [CoroutineScheduler](#CoroutineScheduler)
    * [Manual Scheduling or CoroutineScheduler](#ManualOrAutomatic)
    * [Suspend and Resume](#SuspendAndResume)
    * [Reset Coroutine](#Reset)
    * [Coroutine States](#States)
* [Customizing](#Customizing)
    * [Custom Coroutines](#Custom)
    * [Manual Coroutines](#Manual)
* [Coroutine Communication](#Communication)
    * [Instance Variables](#InstanceVariables)
    * [Channels (Experimental)](#Channels)
* [Miscellaneous](#Miscellaneous)
    * [External Coroutines](#External)
    * [Functors](#Functors)

<a name="Setup"></a>
## Coroutine Setup

<a name="Include"></a>
### Include Header and Namespace

Only a single header file `AceRoutine.h` is required to use this library.
To prevent name clashes with other libraries that the calling code may use, all
classes are defined in the `ace_routine` namespace. To use the code without
prepending the `ace_routine::` prefix, use the `using` directive:

```C++
#include <AceRoutine.h>
using namespace ace_routine;
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

<a name="OverallStucture"></a>
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
    const ace_common::FCString& getName() const;

    virtual int runCoroutine() = 0;

    virtual unsigned long coroutineMillis() const;

    virtual unsigned long coroutineMicros() const;

    virtual unsigned long coroutineSeconds() const;

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

    void setupCoroutine(const char* name);

    void setupCoroutine(const __FlashStringHelper* name);
};
```

<a name="CoroutineInstance"></a>
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

If you expand the `COROUTINE()` macro from `Coroutine.h`, the above code is
equivalent to writing out the following by hand:

```C++
struct Coroutine_doSomething: Coroutine {
  Coroutine_doSomething() {
    setupCoroutine(F("doSomething"));
  }

  int runCoroutine() override {
    COROUTINE_BEGIN();
    ...
    COROUTINE_END();
  }
};

Coroutine_doSomething doSomething;
```

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
`COROUTINE_BEGIN()` and `COROUTINE_END()` macros. See the **Forever Loops**
section below.

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

The `COROUTINE_DELAY(millis)` macro yields back control to other coroutines
until `millis` milliseconds have elapsed. The following waits for 100
milliseconds:

```C++
COROUTINE(waitMillis) {
  COROUTINE_BEGIN();
  ...
  COROUTINE_DELAY(100);
  ...
  COROUTINE_END();
}
```

The `millis` argument is a `uint16_t`, a 16-bit unsigned integer, which reduces
the size of each coroutine instance by 4 bytes (8-bit processors) or 8 bytes
(32-bit processors). However, the actual maximum delay is limited to 32767
milliseconds to avoid overflow situations if the other coroutines in the system
take too much time for their work before returning control to the waiting
coroutine. With this limit, the other coroutines have as much as 32767
milliseconds before it must yield, which should be more than enough time for any
conceivable situation. In practice, coroutines should complete their work within
several milliseconds and yield control to the other coroutines as soon as
possible.

To delay for longer period of time, we can use the
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
The maximum number of seconds is 32767 seconds.

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

If the above convenience macros are not sufficient, you can choose to write an
explicit for-loop. For example, to delay for 100,000 seconds, instead of using
the `COROUTINE_DELAY_SECONDS()`, we can do this:

```C++
COROUTINE(waitThousandSeconds) {
  COROUTINE_BEGIN();
  static uint32_t i;
  for (i = 0; i < 100000; i++) {
    COROUTINE_DELAY(1000);
  }
  ...
  COROUTINE_END();
}
```

See **For Loop** section below for a description of the for-loop construct.

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
  String s = "hello world"; // ***crashes when doSomething() is resumed***
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

The easiest way to get around these problems is to avoid local variables
and just use `static` variables inside a `COROUTINE()`. Static variables are
initialized once and preserve their value through multiple calls to the
function, which is exactly what is needed.

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

<a name="For"></a>
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

<a name="While"></a>
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

<a name="Forever"></a>
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

The `COROUTINE_YIELD()`, `COROUTINE_DELAY()`, `COROUTINE_AWAIT()` macros have
been designed to allow them to be used almost everywhere a valid C/C++ statement
is allowed. For example, the following is allowed:
```C++
  ...
  if (condition) COROUTINE_YIELD();
  ...
```

<a name="CoroutineChaining"></a>
## Coroutine Chaining

<a name="NoNested"></a>
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

<a name="Chaining"></a>
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

    int runCoroutine override {
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

    int runCoroutine override {
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

<a name="RunningAndScheduling"></a>
## Running and Scheduling

There are 2 ways to run the coroutines:
* manually calling the coroutines in the `loop()` method, or
* automatically scheduling and running them using the `CoroutineScheduler`.

<a name="ManualScheduling"></a>
### Manual Scheduling

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

Prior to v1.2, the initial ordering was sorted by the `Coroutine::getName()`.
And calling `suspend()` would remove the coroutine from the internal list
of coroutines, and `resume()` would add the the coroutine back into the list.
This behavior turned out to be
[fatally flawed](https://github.com/bxparks/AceRoutine/issues/19)

Starting with v1.2, the ordering of the coroutines in the internal list is
officially undefined. As well, the actual properties of the coroutine list is
also considered to be an internal implementation detail that may change in the
future. Client code should not depend on the implementation details of this
internal list.

<a name="ManualOrAutomatic"></a>
### Manual Scheduling or CoroutineScheduler

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
the next one.

The scheduler may choose to remove coroutines which are not running from the
active list. If there are a significant number of these inactive coroutines,
then the `CoroutineScheduler` can be more efficient than manually calling the
coroutines through the global `loop()` method. However, as of v1.2, suspended
coroutines are *not* removed from the scheduling list (see
[Issue #19](https://github.com/bxparks/AceRoutine/issues/19)
for reasons), so the `CoroutineScheduler` is actually slightly less efficient,
but the difference is probably not worth worrying about for almost all cases.

<a name="SuspendAndResume"></a>
### Suspend and Resume

The `Coroutine::suspend()` and `Coroutine::resume()` methods are available
*only* if the `CoroutineScheduler` is used. If the coroutines are called
explicitly in the global `loop()` method, then these methods have no impact.

The `Coroutine::suspend()` and `Coroutine::resume()` **should not** be called
from inside the coroutine. Fortunately, if they are accidentially called,
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
`runCoroutine()` method, you may ned to manually reset those variables to their
initial states as well.

Personally, I have never needed the `reset()` functionalty (so it is
unfortunatesly not tested as much as it could be), but it is apparently useful
for some people. See for example:

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
  `COROUTINE_AWAIT()`
* `kStatusDelaying`: coroutine returned using `COROUTINE_DELAY()`
* `kStatusRunning`: coroutine is currently running
* `kStatusEnding`: coroutine returned using `COROUTINE_END()`
* `kStatusTerminated`: coroutine is permanently terminated. Set only by the
  `CoroutineScheduler`.

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

Prior to v1.2, there was a small operational difference between `kStatusEnding`
and `kStatusTerminated`. A terminated coroutine was removed from the internal
linked list of "active" coroutines that was managed by the `CoroutineScheduler`.
However, there was a serious flaw with this design ([Issue
#19](https://github.com/bxparks/AceRoutine/issues/19)) so with v1.2, there is
now no practical difference between these 2 states. It is possible that a future
design change (something I am noodling over in my mind) may reintroduce a
difference. Regardless, I recommended that the `isDone()` method should be used
to detect a coroutine that has "finished".

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

<a name="Custom"></a>
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
situation, I suspect that the *Manual Coroutines* described in the next section
will be more useful and easier to understand.

<a name="Manual"></a>
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

  private:
    int runCoroutine() override {
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
[BlinkSlowFastManualRoutine](examples/BlinkSlowFastManualRoutine) which shows
the same functionality as [BlinkSlowFastRoutine](examples/BlinkSlowFastRoutine)
rewritten using manual coroutines.

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
  routineA.setupCoroutine("routineA");
  routineB.setupCoroutine("routineB");
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

C++ allows the creation of objects that look syntactically like functions.
by defining the `operator()` method on the class. I have not defined this method
in the `Coroutine` class because I have not found a use-case for it. However, if
someone can demonstrate a compelling use-case, then I would be happy to add it.
