#ifndef ACE_ROUTINE_CHANNEL
#define ACE_ROUTINE_CHANNEL

#include <stdint.h>
#include "Coroutine.h"

/** Write the given value x to the given channel within a Coroutine. */
#define COROUTINE_CHANNEL_WRITE(channel, x) \
do { \
  (channel).setValue(x); \
  COROUTINE_AWAIT((channel).write()); \
} while (false)

/** Read the value in the channel to variable x within a Coroutine. */
#define COROUTINE_CHANNEL_READ(channel, x) \
  COROUTINE_AWAIT((channel).read(x))

namespace ace_routine {

/**
 * An unbuffered synchronized channel. Readers and writers block until the
 * writer is ready to send and the receiver is ready to receive. Then the
 * writer writes, the reader reads, reader continues processing, then the
 * writer continues processing. This sequencing is implemented as a finite
 * state machine.
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
class Channel {
  public:
    /** Constructor. */
    Channel() {}

    /**
     * Used by COROUTINE_CHANNEL_WRITE() to preserve the value of the write
     * across multiple COROUTINE_YIELD() calls. Not designed to be used
     * directly by the user.
     */
    void setValue(const T& value) {
      mValueToWrite = value;
    }

    /**
     * Same as write(constT& value) except use the value of setValue(). Used by
     * COROUTINE_CHANNEL_WRITE() macro. Not designed to be used directly by the
     * user.
     */
    bool write() {
      switch (mChannelState) {
        case kWriterReady:
          return false;
        case kReaderReady:
          mValue = mValueToWrite;
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

    /**
     * Write the given value to the channel through the COROUTINE_AWAIT() if
     * value is a static variable.
     */
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

    /**
     * Read the value through the COROUTINE_AWAIT() macro or the
     * COROUTINE_CHANNEL_READ() macro.
     */
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
    // Disable copy-constructor and assignment operator
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    static const uint8_t kWriterReady = 0;
    static const uint8_t kReaderReady = 1;
    static const uint8_t kDataProduced = 2;
    static const uint8_t kDataConsumed = 3;

    uint8_t mChannelState = kWriterReady;
    T mValue;
    T mValueToWrite;
};

}

#endif
