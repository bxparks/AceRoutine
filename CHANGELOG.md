# Changelog

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
