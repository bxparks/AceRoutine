/*
 * A sketch that illustrates using two coroutines and a channel to create a
 * pipe between them.
 */

#include <Arduino.h>
#include <AceRoutine.h>

#define CHANNEL_TYPE_SYNC 0
#define CHANNEL_TYPE_NO_SYNC 1
#define CHANNEL_TYPE CHANNEL_TYPE_SYNC

#define TEST_TYPE_LOOP 0
#define TEST_TYPE_SEQ 1
#define TEST_TYPE TEST_TYPE_LOOP

using namespace ace_routine;

/**
 * An unbuffered channel that provides no synchronization. Perhaps better
 * described as a buffered channel of size 1.
 *
 * Because of the buffering of size one, sending 10 integers from the writer to
 * the reader has the following order:
 *
 * @code
 * Writer: sending 0
 * Writer: sending 1
 * Reader: received 0
 * Writer: sending 2
 * Reader: received 1
 * Writer: sending 3
 * Reader: received 2
 * ...
 * @endcode
 *
 * In other words, the receiver is one iteration behind the writer.
 *
 * TODO: Figure out if it's useful to move into the src/ace_time/ directory,
 * perhaps as a BufferedChannel?
 */
template<typename T>
class NoSyncChannel {
  public:
    void setValue(const T& value) {
      mValueToWrite = value;
    }

    bool write() {
      if (mDataReady) {
        return false;
      } else {
        mValue = mValueToWrite;
        mDataReady = true;
        return true;
      }
    }

    bool write(const T& value) {
      if (mDataReady) {
        return false;
      } else {
        mValue = value;
        mDataReady = true;
        return true;
      }
    }

    bool read(T& value) {
      if (mDataReady) {
        value = mValue;
        mDataReady = false;
        return true;
      } else {
        return false;
      }
    }

  private:
    T mValue;
    T mValueToWrite;
    bool mDataReady = false;
};

struct Message {
  static uint8_t const kStatusOk = 0;
  static uint8_t const kStatusError = 1;

  uint8_t status;
  int value;
};

#if CHANNEL_TYPE == CHANNEL_TYPE_NO_SYNC
  NoSyncChannel<Message> channel;
#elif CHANNEL_TYPE == CHANNEL_TYPE_SYNC
  // This is a synchronized unbuffered Channel.
  Channel<Message> channel;
#endif

#if TEST_TYPE == TEST_TYPE_LOOP
// Test the ordering of sending 10 integers and receiving 10 integers.
COROUTINE(writer) {
  static int i;
  COROUTINE_BEGIN();
  for (i = 0; i < 10; i++) {
    Serial.print("Writer: sending ");
    Serial.println(i);
    Message message = {Message::kStatusOk, i};
    COROUTINE_CHANNEL_WRITE(channel, message);
  }
  Serial.println("Writer: done");
  COROUTINE_END();
}

COROUTINE(reader) {
  COROUTINE_LOOP() {
    Message message;
    COROUTINE_CHANNEL_READ(channel, message);
    Serial.print("Reader: received ");
    Serial.println(message.value);
  }
}

#else

// Test the sequencing of the writer and reader.
COROUTINE(writer) {
  COROUTINE_BEGIN();
  Serial.println("Writer: sending data");
  COROUTINE_CHANNEL_WRITE(channel, 42);
  Serial.println("Writer: sent data");
  COROUTINE_END();
}

COROUTINE(reader) {
  COROUTINE_BEGIN();
  Serial.println("Reader: sleeping for 1 second");
  COROUTINE_DELAY(1000);
  Serial.println("Reader: receiving data");
  int data;
  COROUTINE_CHANNEL_READ(channel, data);
  Serial.println("Reader: received data");
  COROUTINE_END();
}

#endif

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro
}

void loop() {
  // The order of these 2 shouldn't matter, but we should flip them
  // occasionally to verify that.
  reader.runCoroutine();
  writer.runCoroutine();
}
