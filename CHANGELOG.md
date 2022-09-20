# Changelog

* Unreleased
* 1.5.1 (2022-09-20)
    * Add Adafruit nRF52 boards to "Tier 2" after validation by community
      member.
    * Upgrade tool chain
        * Arduino CLI from 0.19.2 to 0.27.1
        * Arduino AVR Core from 1.8.4 to 1.8.5
        * STM32duino from 2.2.0 to 2.3.0
        * ESP32 Core from 2.0.2 to 2.0.5
        * Teensyduino from 1.56 to 1.57
* 1.5.0 (2022-03-19)
    * (Re)add support for human-readable coroutine names.
        * See [Coroutine Names](USER_GUIDE.md#CoroutineNames) in the
          `USER_GUIDE.md`.
        * Adds the following methods to the `Coroutine` class: `setName()`,
          `setName()`, `getCName()`, `getFName()`, `getNameType()`, and
          `printNameTo()`.
        * Resource consumption
            * Increases flash usage by 6-10 bytes per coroutine.
            * Increases static ram usage by 3 bytes (AVR) or 4 bytes (32-bit)
              per coroutine, plus the additional storage for the string itself.
    * Support profiling of `Coroutine::runCoroutine()` execution time.
        * See [Coroutine Profiling](USER_GUIDE.md#CoroutineProfiling) in the
          `USER_GUIDE.md`.
        * API changes
            * Add `CoroutineProfiler` interface with a `updateElapsedMicros()`
              method.
            * Add `Coroutine::setProfiler()` to store the profiler pointer.
            * Add `Coroutine::runCoroutineWithProfiler()` which measures the
              elapsed time of `runCoroutine()` and calls
              `CoroutineProfiler::updateElapsedMicros()`.
            * Add `CoroutineScheduler::runCoroutineWithProfiler()` which calls
              `Coroutine::runCoroutineWithProfiler()` instead of the normal
              `Coroutine::runCoroutine()`.
            * Add `CoroutineScheduler::loopWithProfiler()` public static method
              which calls `runCoroutineWithProfiler()`.
        * Provide `LogBinProfiler` subclass of `CoroutineProfiler`.
            * Keeps a frequency count of the elapsed microseconds using 32 bins
              representing the `log2()` function of the elapsed microseconds.
        * Provide 2 renderers of `LogBinProfiler`:
            * `LogBinTableRenderer::printTo()` prints a formatted table of the
              frequency count over all coroutines. This represents a poor-man's
              version of the log-log graph of the frequency count.
            * `LogBinJsonRenderer::printTo()` prints the frequency count
              in JSON format.
        * See [HelloCoroutineWithProfiler](examples/HelloCoroutineWithProfiler)
          and [HelloSchedulerWithProfiler](examples/HelloSchedulerWithProfiler).
        * Thanks to peufeu2@ who provided the ideas and proof of concept in
          [Discussion#50](https://github.com/bxparks/AceRoutine/discussions/50).
    * Coroutine consumes more static RAM
        * due to extra pointers to support `setName()` and `setProfiler()`
        * 8-bits: increases from 11 bytes to 16 bytes per coroutine
        * 32-bits: increases from 20 bytes to 28 bytes per coroutine
    * Move `Coroutine::sStatusStrings` to `PROGMEM`
        * Saves 12 bytes of static RAM on AVR and 24 bytes on ESP8266, if
          `CoroutineScheduler::list()` is used.
* 1.4.2 (2022-02-04)
    * Remove dependency to AceCommon library in `libraries.properties`.
        * AceRoutine core no longer depends on AceCommon.
        * A few programs in `tests/` and `examples/` still depend it though,
          but those are used mainly by the developers, not end-users.
    * Clean up and update `examples/ChannelBenchmark` and its `README.md`.
        * Use `generate_readme.py` and `generate_table.awk` scripts, just like
          `examples/AutoBenchmark`.
* 1.4.1 (2022-02-02)
    * Update `examples/SoundManager` to demonstrate coroutines defined in
      separate files (see
      [Discussion#40](https://github.com/bxparks/AceRoutine/discussions/40)).
    * Update various unit tests to conform to EpoxyDuino v1.2.0 which
      reverts `Print::println()` to print `\r\n` instead of just `\n`.
    * Remove SAMD21 boards into new "Tier 3: May work but unsupported" tier
      because the SAMD21 tool chain no longer works for me, and Arduino SAMD21
      boards use the ArduinoCore-api which is incompatible.
    * Upgrade tool chain
        * Arduino IDE from 1.8.13 to 1.8.19
        * Arduino CLI from 0.14.0 to 0.19.2
        * Arduino AVR Core from 1.8.3 to 1.8.4
        * STM32duino from 2.0.0 to 2.2.0
        * ESP8266 Core from 2.7.4 to 3.0.2
        * ESP32 Core from 1.0.6 to 2.0.2
        * Teensyduino from 1.54 to 1.56
* 1.4.0 (2021-07-29)
    * Upgrade STM32duino Core from 1.9.0 to 2.0.0.
        * MemoryBenchmark: Flash usage increases by 2.3kB across the board, but
          static RAM goes down by 250 bytes. Very little change to AceRoutine
          code itself.
        * AutoBenchmark: No change.
    * Upgrade SparkFun SAMD Core from 1.8.1 to 1.8.3.
        * No change observed in MemoryBenchmark or AutoBenchmark.
    * Add virtual `Coroutine::setupCoroutine()` with a default empty
      implementation, and optional `CoroutineScheduler::setupCoroutines()` to
      automatically loop over all coroutines.
        * See [Issue #36](https://github.com/bxparks/AceRoutine/issues/36) for
          motivation.
        * If not used, `Coroutine::setupCoroutine()` increases flash consumption
          by 4 bytes, and static memory by 2 bytes per coroutine on AVR
          processors.
        * If used, `Coroutine::setupCoroutine()` can consume a significant
          amount of memory resources. On AVR, at least 50-60 bytes per
          coroutine. On 32-bit processors, about 30-40 bytes per coroutine.
* 1.3.1 (2021-06-02)
    * Bring back `COROUTINE_DELAY_MICROS()` and `COROUTINE_DELAY_SECONDS()`
      with an alternate implemenation that increases flash and static memory
      *only* if they are used.
        * The `Coroutine` itself knows whether it is delaying in units of
          milliseconds, microseconds, or seconds, based on its continuation
          point.
        * Make `CoroutineScheduler::runCoroutine()` always call into
          `Coroutine::runCoroutine()` when the `Coroutine::mStatus` is delaying,
          instead of preemptively trying to figure out if the delay has expired.
        * `Coroutine` no longer needs a runtime `mDelayType` descriminator.
        * The result is that the code to support `COROUTINE_DELAY_MICROS()` and
          `COROUTINE_DELAY_SECONDS()` is not pulled into the program if they are
          not used.
* 1.3.0 (2021-06-02)
    * Activate GitHub Discussions for the project.
    * **Potentially Breaking**: Change `Coroutine` destructor from virtual to
      non-virtual.
        * Saves 500-600 bytes on AVR processors, 350 bytes on SAMD21, and 50-150
          bytes on other 32-bit processors. 
        * Coroutines can now be created only statically, not dynamically on the
          heap.
    * **Potentially Breaking**: Lift `Coroutine` into `CoroutineTemplate` class.
      Lift `CoroutineScheduler` into `CoroutineSchedulerTemplate` class.
        * Define `Coroutine` to be `CoroutineTemplate<ClockInterface>`, almost
          fully backwards compatible with previous implementation.
        * Define `CoroutineScheduler` to be
          `CoroutineSchedulerTemplate<Coroutine>`, almost fully backwards
          compatible with previous implementation.
        * All macros (e.g. `COROUTINE()`, `COROUTINE_DELAY()`,
          `COROUTINE_YIELD()`, etc) should work as before.
        * Replace the 3 clock virtual methods on
          `Coroutine` (`coroutineMicros()`, `coroutineMillis()`,
          `coroutineSeconds()`) with a injectable `ClockInterface` template
          parameter.
        * **Breaking**: Convert `Coroutine::coroutineMicros()`,
          `Coroutine::coroutineMillis()`, and `Coroutine::coroutineSeconds()`
          into `private static` functions which delegate to
          `ClockInterface::micros()`, `ClockInterface::millis()`, and
          `ClockInterface::sesconds()`.
        * Create `TestableClockInterface` for testing.
        * Create `TestableCoroutine` for testing.
        * Create `TestableCoroutineScheduler` for testing.
        * Only 0-40 bytes of flash memory reduction on AVR processors, to my
          surprise.
        * But 100-1500 bytes of flash memory reduction on various 32-bit
          processors.
    * **Breaking**: Remove `COROUTINE_DELAY_SECONDS()`.
        * Saves ~200 bytes on AVR processors, about 40%.
        * Saves about 20-30 bytes on 32-bit processors.
        * The replacement is a for-loop around a `COROUTINE_DELAY()`,
          as shown in [USER_GUIDE.md#Delay](USER_GUIDE.md#Delay).
    * **Breaking**: Remove `COROUTINE_DELAY_MICROS()`.
        * Saves about 15-20 bytes of flash and 1 byte of static memory
          per coroutine on AVR processors.
        * Saves about 80-100 bytes of flash on 32-bit processors, plus an
          additional 20-30 bytes of flash per coroutine on 32-bit processors.
        * The `COROUTINE_DELAY_MICROS()` was never reliable because it depended
          on other coroutines to release control of execution faster than the
          value of the delay microseconds. This is very difficult to guarantee
          in a cooperative multitasking environment.
        * Removing this simplifies the code a fair amount.
    * **Breaking**: Remove `Coroutine::getName()` and `Coroutine::mName`. The
      human-readable name of the coroutine is no longer retained, to reduce
      flash and static memory consumption.
        * Remove `setupCoroutineOrderedByName()`, since it is no longer possible
          to sort by name.
        * Since we don't need to capture the name of the coroutine, we can
          move `setupCoroutine()` functionality directly into
          `Coroutine::Coroutine()` constructor.
        * Deprecate `setupCoroutine(const char*)` and `setupCoroutine(const
          __FlashStringHelper*)` into no-ops. They are retained for backwards
          compatibility.
        * Print the coroutine pointer address instead of its name in
          `CoroutineScheduler::list()`, since the name is no longer retained.
        * Saves 10-30 bytes of flash and 3 bytes of static memory per coroutine
          instance on AVR.
        * Saves 10-40 bytes of flash and 8 bytes of static memory per coroutine
          instance on 32-bit processors.
    * Blacklist platforms using the https://github.com/arduino/ArduinoCore-api
      to give a user-friendly message instead of pages and pages of compiler
      errors.
    * Update
      [Direct Scheduling or CoroutineScheduler](USER_GUIDE.md#DirectOrAutomatic)
      section to recommend direct calls to `Coroutine::runCoroutine()` on 8-bit
      processors, and limit the `CoroutineScheduler` to 32-bit processors with
      sufficient flash memory.
    * Add preliminary support for ATtiny85.
* 1.2.4 (2021-01-22)
    * Update UnixHostDuino 0.4 to EpoxyDuino 0.5.
    * No functional change in this release.
* 1.2.3 (2021-01-19)
    * Add official support for STM32 by validating on a Blue Pill board.
    * Add scripts to `AutoBenchmark` to automate data collection.
    * Update `MemoryBenchmark` and `AutoBenchmark` tables with STM32; re-add
      Teensy 3.2 benchmarks now that I'm able to upload again.
    * No functional change in this release.
* 1.2.2 (2020-12-20)
    * Add generic definition of `FPSTR()` macro for unrecognized platforms.
      Allows code with `ARDUINO_ARCH_STM32` dependencies to compile. I will
      support it officially after I get hardware to perform actual validation.
    * Clean up MemoryBenchmark scripts for consistency with other `Ace*`
      libraries.
    * No functional change in this release.
* 1.2.1 (2020-11-12)
    * Add `DEVELOPER.md` notes to myself. 
    * Add python script to generate the README.md in
      [examples/MemoryBenchmarks](examples/MemoryBenchmarks) to automatically
      regenerate the embedded ascii tables. Regenerate README.md for v1.2.
    * No functional change in this release.
* 1.2 (2020-11-10)
    * Fix an infinite loop in the internal singly-linked list of coroutines when
      `resume()` is called immediately after `suspend()`, without waiting for
      `CoroutineScheduler::loop()` to actually remove the coroutine from the
      list. The fix no longer removes suspended coroutine from the linked list,
      simplifying the code considerably. Fixes [Issue
      #19](https://github.com/bxparks/AceRoutine/issues/19).
    * Fix incorrect status set by `Coroutine::reset()` which prevented it from
      actually working. Fixes [Issue
      #20](https://github.com/bxparks/AceRoutine/issues/20).
    * **Potentially Breaking**: The internal linked list of coroutines is now
      considered to be a private implementation detail that may change in the
      future. The ordering of the list of coroutines is now undefined. The
      `Coroutine::getRoot()` and `Coroutine::getNext()` public methods are now
      private. The new `setupCoroutineOrderedByName()` methods replicate the old
      behavior, but these are exposed only for testing purposes.
    * Fix documentation bug in README.md to say that `suspend()` and `resume()`
      *should not* be called within the coroutine itself. They can only be
      called from outside the coroutine. They are no-ops inside a coroutine.
    * Extract lengthy Usage section of `README.md` into `USER_GUIDE.md`. Add
      table of contents to help navigate the long document. Rewrite and update
      content to be more useful and more clear hopefully.
* 1.1 (2020-11-01)
    * Add a `Coroutine::reset()` method that causes the Coroutine to restart
      from the beginning of the coroutine upon the next iteration. (Fixes #13
      and #14).
    * **Potentially Breaking**: AceRoutine now depends on the AceCommon
      (https://github.com/bxparks/AceCommon) library to avoid maintaining
      multiple versions of low-level common code. The externally exposed API has
      not changed. The AceCommon library can be installed through the Arduino
      IDE Library Manager, as explained in Installation section of the
      README.md.
    * **Potentially Breaking**: Move the CommandLineInterface package from
      `src/ace_routine/cli` to the AceUtils library
      (https://github.com/bxparks/AceUtils). The AceUtils library is a better
      home for this higher-level package that depends on AceRoutine.
* 1.0.1 (2020-09-18)
    * Add continuous integration using GitHub Actions.
    * Add more documentation and examples of Manual Coroutines in README.md.
      Manual Coroutines are often more useful than Custom Coroutines, which I
      have found useful only for unit testing.
        * [HelloManualCoroutine](examples/HelloManualCoroutine)
        * [BlinkSlowFastManualRoutine](examples/BlinkSlowFastManualRoutine)
    * Simplify the HelloCoroutine and HelloScheduler examples.
* 1.0 (2019-09-04)
    * Support UnixHostDuino in all sketches under `examples/`.
    * Rename `Flash.h` to `compat.h`. Reenable `F()` strings for ESP8266.
    * Fix various programs in `examples/` to work using clang++ (using
      UnixHostDuio) which automatically fixes them on MacOS.
    * Graduate out of beta to v1.0.
* 0.3 (2019-08-26)
    * Update `AutoBenchmark/README.md` benchmark numbers.
    * Use a `do-while` loop `COROUTINE_AWAIT()` so that it is guaranteed to call
      `COROUTINE_YIELD()` at least once. Previously, if the `condition` of the
      await was already (or always) true, the `while-loop` caused the coroutine
      to hog the control flow without yielding.
    * Use a `do-while` loop in `COROUTINE_DELAY()` so that `COROUTINE_YIELD()`
      is guaranteed to be called at least once, even if the delay is 0.
    * Add `COROUTINE_DELAY_MICROS(delayMicros)` which is similar to the
      existing `COROUTINE_DELAY(delayMillis)` macro. The actual delay time may
      be inaccurate on slow processors (e.g. 16 MHz AVR processors) and become
      more accurate for faster processors (e.g. ESP32). (#9)
    * **Breaking**: The `COROUTINE_DELAY_SECONDS(delaySeconds)` macro now takes
      only one parmeter instead of 2 parameters. An external `loopCounter`
      variable no longer needs to be provided by the caller, which simplifies
      the API.
    * Add `examples/Delay/Delay.ino` program to validate the various
      `COROUTINE_DELAY*()` macros.
    * The `sizeof(Coroutine)` increases from 14 bytes to 15 bytes on an 8-bit
      processor. No change on 32-bit (still 28 bytes).
* 0.2.2 (2019-07-31)
    * Add `SHIFT_ARGC_ARGV()` macro for easy token shifting,
      and `isArgEqual()` method for easy comparison against flash string
      in the CLI library.
    * Verify library on SAMD21 (e.g. Arduino Zero) and ATmega2560 boards.
    * Update unit test Makefiles to use `UnixHostDuino`.
    * Fix clang++ error in `cli` library due to local `c` variable inside
      a `COROUTINE_LOOP` macro.
* 0.2.1 (2019-07-01)
    * Fix various typos in README.md.
    * Fix broken compile of ESP32 and ESP8266 boards by fixing fqbn.
    * Add missing `pinMode()` in some examples.
    * Add `ACE_ROUTINE_VERSION_STRING` for easier display for version number.
* 0.2 (2018-10-02)
    * Add `COROUTINE_DELAY_SECONDS()` to support delays longer than
      32767 millis.
    * Update `auniter.ini` and `Jenkinsfile` for compatibility with
      AUniter v1.7.
    * Coroutine Status
        * Remove `kStatusAwaiting` state used by `COROUTINE_AWAIT()` which
          wasn't being used for anything, collapse into existing
          `kStatusYielding`.
        * Print human-readable strings of the Coroutine status from
          `CoroutineScheduler::list()`.
    * Channels
        * Add a synchronized unbuffered Channel, similar to Go Lang channels.
        * Add `COROUTINE_CHANNEL_READ()` and `COROUTINE_CHANNEL_WRITE()`
          convenience macros.
        * Write `ChannelBenchmark` to determine the CPU overhead of using
          channels.
    * CLI library
        * Update `cli` library to use channels to read from serial port
          asynchronously.
        * Convert `CommandHandler` from a pointer to function to a full
          class to allow dependency injection and better code reuse.
    * Manual and custom Coroutines
        * Rename `Coroutine::run()` to `Coroutine::runCoroutine()` for clarity.
        * Rename `Coroutine::init()` to `Coroutine::setupCoroutine()` to make
          it easier to use mix-in classes.
* 0.1 (2018-08-07)
    * Beta release.
* (2018-07-24)
    * Remove `runRoutine()` indirection. Decided that it did not provide
      enough benefits and felt like over-engineering.
    * Use `Stream` and `Print` abstract classes instead of using `Serial`
      object directly.
    * Create 2 subclasses of CommandDispatcher to support c-strings or
      flash strings.
* (2018-07-19)
    * Add examples/CommandLineInterface sketch which provides a non-blocking
      command line interface using coroutines.
    * Add non-blocking SerialReader and CommandDispatcher.
    * Use runRoutine() method for the body of the coroutines instead of run().
    * Limit maximum value of COROUTINE_DELAY() to 32767 millis to give more
      breathing room to other coroutines.
    * Add support for AUniter and Jenkins continuous integration.
* (2018-06-23)
    * After allowing Coroutine::run() to be called directly,
      bypassing CoroutineScheduler, these things do actually look like
      coroutines, so changed names back to "Coroutine" and "COROUTINE*()"
      macros.
* (2018-05-11)
    * Changed names of everything from "Coroutine" to just "Routine" because
      this is not really a coroutine.
    * Add ROUTINE_AWAIT() macro.
    * Clean up nullptr handling of getName().
    * Change RoutineScheduler::list() to take a Print pointer as parameter, and
      remove define guards. If it's not used, the linker will strip it out.
    * Add RoutineScheduler::setup(), changed ::run() to ::loop().
* (2018-05-10)
    * Simplified and stabilized the API.
    * Created README.md.
* (2018-05-08)
    * Start of project.
