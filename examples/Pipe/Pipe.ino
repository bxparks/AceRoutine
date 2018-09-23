/*
 * A sketch that illustrates using two coroutines and a channel to create a
 * pipe between them. I'm borrowing concepts from the GoLang channels.
 *
 * WORK IN PROGRESS
 */

#include <Arduino.h>
#include <AceRoutine.h>

#define CHANNEL_TYPE_SIMPLE 0
#define CHANNEL_TYPE_SYNCHRONIZED 1
#define CHANNEL_TYPE CHANNEL_TYPE_SYNCHRONIZED

#define TEST_TYPE_LOOP 0
#define TEST_TYPE_SEQ 1
#define TEST_TYPE TEST_TYPE_LOOP

using namespace ace_routine;

/**
 * An unbuffered channel that provides no synchronization. Perhaps better
 * described as a buffered channel of size 1.
 *
 * Because of the buffering of size one, sending 20 integers from the writer to
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
 */
template<typename T>
class SimpleChannel {
  public:
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
    bool mDataReady = false;
};

/**
 * An unbuffered synchronizing channel. Readers and writers block until the
 * writer is ready to send and the receiver is ready to receive. Then the
 * writer writes, the reader reads, then continues processing. Then the writer
 * is allowed to continue processing. This sequencing is implemented as a
 * finite state machine.
 *
 * The result of sending 20 integers from the writer to the reader looks
 * like this:
 *
 * @code
 * Writer: sending 0
 * Reader: received 0
 * Writer: sending 1
 * Reader: received 1
 * Writer: sending 2
 * Reader: received 2
 * ...
 * @endcode
 *
 * This sequence of events matches the user's expectations.
 */
template<typename T>
class SynchronizedChannel {
  public:
    bool write(const T& value) {
      switch (mChannelState) {
        case kWriterReady:
          return false;
        case kReaderReady:
          mValue = value;
          mChannelState = kDataProduced;
          return false;
        case kDataProduced:
          return false;
        case kDataConsumed:
          mChannelState = kWriterReady;
          return true;
        default:
          return false;
      }
    }

    bool read(T& value) {
      switch (mChannelState) {
        case kWriterReady:
          mChannelState = kReaderReady;
          return false;
        case kReaderReady:
          return false;
        case kDataProduced:
          value = mValue;
          mChannelState = kDataConsumed;
          return true;
        case kDataConsumed:
          return false;
        default:
          return false;
      }
    }

  private:
    static const uint8_t kWriterReady = 0;
    static const uint8_t kReaderReady = 1;
    static const uint8_t kDataProduced = 2;
    static const uint8_t kDataConsumed = 3;
    uint8_t mChannelState = kWriterReady;
    T mValue;
};

#if CHANNEL_TYPE == CHANNEL_TYPE_SIMPLE
  SimpleChannel<int> channel;
#else
  SynchronizedChannel<int> channel;
#endif

#if TEST_TYPE == TEST_TYPE_LOOP
// Test the ordering of sending 20 integers and receiving 20 integers.
COROUTINE(writer) {
  COROUTINE_BEGIN();
  static int i;
  for (i = 0; i < 20; i++) {
    Serial.print("Writer: sending ");
    Serial.println(i);
    COROUTINE_AWAIT(channel.write(i));
  }
  Serial.println("Writer: done");
  COROUTINE_END();
}

COROUTINE(reader) {
  COROUTINE_LOOP() {
    int i;
    COROUTINE_AWAIT(channel.read(i));
    Serial.print("Reader: received ");
    Serial.println(i);
  }
}

#else

// Test the sequencing of the writer and reader.
COROUTINE(writer) {
  COROUTINE_BEGIN();
  Serial.println("Writer: sending data");
  COROUTINE_AWAIT(channel.write(42));
  Serial.println("Writer: sent data");
  COROUTINE_END();
}

COROUTINE(reader) {
  static int data;
  COROUTINE_BEGIN();
  Serial.println("Reader: sleeping for 1 second");
  COROUTINE_DELAY(1000);
  Serial.println("Reader: receiving data");
  COROUTINE_AWAIT(channel.read(data));
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
