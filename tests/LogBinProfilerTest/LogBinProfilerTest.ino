#line 2 "LogBinProfilerTest.ino"

#include <AceRoutine.h>
#include <AUnitVerbose.h>

using ace_routine::LogBinProfiler;
using ace_routine::internal::rollupExteriorBins;
using aunit::TestRunner;

// ---------------------------------------------------------------------------
// Test CorooutineLogBinProfiler functions.
// ---------------------------------------------------------------------------

test(updateElapsedMicros) {
  LogBinProfiler profiler;

  profiler.updateElapsedMicros(0);
  assertEqual(profiler.mBins[0], 1);

  profiler.updateElapsedMicros(1);
  assertEqual(profiler.mBins[0], 2);

  profiler.updateElapsedMicros(2);
  assertEqual(profiler.mBins[1], 1);

  profiler.updateElapsedMicros(1024);
  assertEqual(profiler.mBins[10], 1);

  profiler.updateElapsedMicros(1025);
  assertEqual(profiler.mBins[10], 2);

  profiler.updateElapsedMicros(UINT32_MAX);
  assertEqual(profiler.mBins[31], 1);
}

test(rollupExteriorBins) {
  LogBinProfiler profiler;
  profiler.mBins[0] = 1;
  profiler.mBins[1] = 2;
  profiler.mBins[2] = 3;
  profiler.mBins[30] = 11;
  profiler.mBins[31] = 12;

  uint16_t bufBins[32];

  rollupExteriorBins(bufBins, profiler.mBins, profiler.kNumBins, 0, 32);
  assertEqual(bufBins[0], 1);
  assertEqual(bufBins[1], 2);
  assertEqual(bufBins[2], 3);
  assertEqual(bufBins[3], 0);
  assertEqual(bufBins[30], 11);
  assertEqual(bufBins[31], 12);

  rollupExteriorBins(bufBins, profiler.mBins, profiler.kNumBins, 1, 31);
  assertEqual(bufBins[1], 3);
  assertEqual(bufBins[2], 3);
  assertEqual(bufBins[3], 0);
  assertEqual(bufBins[29], 0);
  assertEqual(bufBins[30], 23);

  rollupExteriorBins(bufBins, profiler.mBins, profiler.kNumBins, 2, 30);
  assertEqual(bufBins[2], 6);
  assertEqual(bufBins[3], 0);
  assertEqual(bufBins[29], 23);

  rollupExteriorBins(bufBins, profiler.mBins, profiler.kNumBins, 10, 12);
  assertEqual(bufBins[10], 6);
  assertEqual(bufBins[11], 23);

  rollupExteriorBins(bufBins, profiler.mBins, profiler.kNumBins, 10, 11);
  assertEqual(bufBins[10], 29);

  // If startBin == endBin, no rollup should be performed, so bufBins[] should
  // be untouched.
  bufBins[0] = 9999;
  bufBins[1] = 9999;
  rollupExteriorBins(bufBins, profiler.mBins, profiler.kNumBins, 0, 0);
  assertEqual(bufBins[0], 9999);
  assertEqual(bufBins[1], 9999);
}

// ---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // some boards reboot twice
#endif

  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro
}

void loop() {
  TestRunner::run();
}
