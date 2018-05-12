#line 2 "ExternalRoutine.cpp"

#include <AceRoutine.h>
#include "ace_routine/testing/TestableRoutine.h"
using namespace ace_routine;
using namespace ace_routine::testing;

ROUTINE_NAMED(TestableRoutine, c) {
  ROUTINE_BEGIN();

  ROUTINE_DELAY(100);

  ROUTINE_END();
}
