/*
 * A sketch that illustrates creating tasks and dispatching them to the
 * scheduler.
 *
 * WORK IN PROGRESS
 */

#include <Arduino.h>
#include <AceRoutine.h>
using namespace ace_routine;

class LineBuffer {
  public:
    void append(char c) {
      if (mPos >= kBufferSize - 1) return;
      mBuffer[mPos] = c;
      mPos++;
    }

    const char* getLine() {
      mBuffer[mPos] = '\0';
      mPos++;
      return mBuffer;
    }

    void clear() {
      mPos = 0;
    }

  private:
    static const uint8_t kBufferSize = 32;

    char mBuffer[kBufferSize];
    uint8_t mPos = 0;
};

LineBuffer lineBuffer;

COROUTINE(reader) {
  COROUTINE_LOOP() {
    COROUTINE_AWAIT(Serial.available() > 0);
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      const char* line = lineBuffer.getLine();
      if (strcmp(line, "list") == 0) {
        CoroutineScheduler::list(Serial);
      } else if (strcmp(line, "millis") == 0) {
        Serial.println(millis());
      } else if (strcmp(line, "micros") == 0) {
        Serial.println(micros());
      } 
      lineBuffer.clear();
    } else {
      lineBuffer.append(c);
    }
  }
}

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
