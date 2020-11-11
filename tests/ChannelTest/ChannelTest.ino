#line 2 "ChannelTest.ino"

#include <AceRoutine.h>
#include <AUnitVerbose.h>

using namespace ace_routine;
using namespace aunit;

Channel<int> channel;

test(ChannelTest, channelReadAndWrite) {
  int writeValue = 2;
  int readValue = 0;

  assertFalse(channel.read(readValue));
  assertFalse(channel.read(readValue)); // second call should also return false
  assertFalse(channel.write(writeValue));
  assertFalse(channel.write(writeValue)); // second call should return false

  assertTrue(channel.read(readValue));
  assertTrue(channel.write(writeValue)); // write() can proceed after read()

  assertEqual(readValue, writeValue);
}

test(ChannelTest, channelWriteMacro) {
  int writeValue = 2;
  int readValue = 0;

  assertFalse(channel.read(readValue));
  assertFalse(channel.read(readValue)); // second call should also return false

  // Test the methods used by COROUTINE_CHANNEL_WRITE()
  channel.setValue(writeValue);
  assertFalse(channel.write());
  assertFalse(channel.write()); // second call should return false

  assertTrue(channel.read(readValue));
  assertTrue(channel.write()); // write() can proceed after read()

  assertEqual(readValue, writeValue);
}

// ---------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  delay(1000); // some boards reboot twice
#endif

  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro
}

void loop() {
  TestRunner::run();
}
