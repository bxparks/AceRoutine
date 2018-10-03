# Changelog

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
