# Command Line Shell

A demo of a primitive command line "shell" for Arduino using the classes in
`src/ace_routine/cli`. It currently supports 5 commands:

* `help [command]` - list the available commands
* `delay (on | off) millis` - change the LED on or off duration
* `list` - list the coroutines managed by the `CoroutineScheduler`
* `free` - print the amount of free memory
* `echo [args ...]` - echo the arguments on the command line

The shell is non-blocking. In other words, the Arduino board is able to do other
things using AceRoutine coroutines while waiting for the user to type in the
next command. In this demo, the built-in LED blinks on and off using a second
coroutine. The duration of the on and off states can be controlled interactively
using the `delay` command described below.

## Help

The `help` command prints help string of all available commands:
```
> help
Usage: help [command]
Commands:
  help [command]
  delay (on | off) millis
  list
  free
  echo args ...
```

Details about a specific command can be retrieved by specifying the command:
```
> help delay
Usage: delay (on | off) millis
```

## Delay

Set the duration of the LED on or off states. For example, the following
commands set the LED on state to 500 millis, and the LED off state to 100
millis.

```
> delay on 500
> delay off 100
```

## List

List the coroutines managed by the `CoroutineScheduler`:

```
> list
Coroutine 567; status: 1
Coroutine blinkLed; status: 4
```

## Free

Print the amount of free memory between the stack and the heap.
On an Arduino Pro Micro clone, this is:
```
> free
Free memory: 2108
```

On an ESP32, I get:
```
> free
Free memory: 222896
```

## Echo

Echo the arguments.
```
> echo hello world!
hello world!
```
