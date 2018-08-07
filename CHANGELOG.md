# Changelog

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
