#line 2 "ExternalCoroutine.cpp"

#include <AceRoutine.h>
#include "ace_routine/testing/TestableCoroutine.h"
using namespace ace_routine;
using namespace ace_routine::testing;

COROUTINE(TestableCoroutine, c) {
  COROUTINE_BEGIN();
  COROUTINE_DELAY(100);
  COROUTINE_END();
}
