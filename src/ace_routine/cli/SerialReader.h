#ifndef ACE_ROUTINE_SERIAL_READER_H
#define ACE_ROUTINE_SERIAL_READER_H

#include <ctype.h>
#include <AceRoutine.h>

namespace ace_routine {
namespace cli {

/**
 * Reads tokens (lines, words, integers, characters, etc) from the Serial
 * device. These are non-blocking helper routines designed to be used inside
 * COROUTINE() macros from the AceRoutine library.
 */
class SerialReader {
  public:
    static const uint8_t READ_BUF_SIZE = 64;

    SerialReader(Stream& serial):
      mSerial(serial)
    {}

    /**
     * Get a character from serial port. Return true if parsing is finished,
     * otherwise false indicates that this method should be called again for
     * further processing.
     */
    bool getChar(char* c) {
      if (mPushback != 0) {
        *c = mPushback;
        mPushback = 0;
        return true;
      }
      if (mSerial.available() == 0) return false;
      *c = mSerial.read();
      mPushback = 0;
      return true;
    }

    /**
     * Get a string bounded by whitespace, i.e. a "word". Return true if
     * parsing is finished, otherwise false indicates that this method should
     * be called again for further processing.
     *
     * If isError is true, then a parsing error occurred. If is false, then
     * word contains the pointer to the word string
     */
    bool getWordString(bool* isError, char** word) {
      char c;
      while (getChar(&c)) {
        if (isspace(c)) {
          if (mIndex == 0) {
            *isError = true;
          } else {
            pushback(c);
            resetBuffer();
            *isError = false;
            *word = mBuf;
          }
          return true;
        }
        bool error = addToBuffer(c);
        if (error) {
          *isError = error;
          return true;
        }
      }
      return false;
    }

    /**
     * Get an integer. Return true if parsing is finished, otherwise false
     * indicates that this method should be called again for further
     * processing.
     *
     * If isError is true, then a parsing error occurred. If is false, then
     * value contains the integer.
     */
    bool getInteger(bool* isError, int* value) {
      char c;
      while (getChar(&c)) {
        if (!isdigit(c)) {
          if (mIndex == 0) {
            resetBuffer();
            *isError = true;
          } else {
            pushback(c);
            resetBuffer();
            // NOTE: What happens if the intger in mBuf is too big?
            *value = atoi(mBuf);
            *isError = false;
          }
          return true;
        }
        bool error = addToBuffer(c);
        if (error) {
          *isError = error;
          return true;
        }
      }
      return false;
    }

    /**
     * Parse a comma. Return true if parsing is finished, otherwise false
     * indicates that this method should be called again for further
     * processing.
     *
     * If isError is true, then a parsing error ocurred.
     */
    bool getComma(bool* isError) {
      char c;
      if (!getChar(&c)) return false;
      *isError = (c != ',');
      return true;
    }

    /**
     * Skip whitespace. Return true if parsing is finished, otherwise false
     * indicates that this method should be called again for further processing.
     */
    bool skipWhiteSpace() {
      char c;
      while (getChar(&c)) {
        if (!isspace(c)) {
          pushback(c);
          return true;
        }
      }
      return false;
    }

    /**
     * Get a line. Returns true if parsing is finished required, otherwise
     * false indicates that this method should be called again for further
     * processing.
     *
     * If a full line was read, then this returns true, isError=false, 
     * and line=line
     * If the buffer overflowed, then this returns true, isError=true, 
     * and line is the partial buffer of the long line.
     */
    bool getLine(bool* isError, char** line) {
      while (mSerial.available() > 0) {
        char c = mSerial.read();
        mBuf[mIndex] = c;
        mIndex++;
        if (mIndex >= READ_BUF_SIZE - 1) {
          resetBuffer();
          *isError = true;
          *line = mBuf;
          return true;
        }
        if (c == '\n') {
          resetBuffer();
          *isError = false;
          *line = mBuf;
          return true;
        }
      }
      return false;
    }

  private:
    /**
     * Push the given character back into the getChar() buffer. This allows us
     * to get a one-character look-ahead.
     */
    void pushback(char c) {
      mPushback = c;
    }

    /**
     * Terminate the current buffer with the NUL character (so that the current
     * string can be retrieved), and reset the character index to the
     * beginning of the buffer to prepare for the next getChar() or getLine().
     */
    void resetBuffer() {
      mBuf[mIndex] = '\0';
      mIndex = 0;
    }

    /** Add char c to buffer. Return true if error. */
    bool addToBuffer(char c) {
      mBuf[mIndex] = c;
      mIndex++;
      if (mIndex >= READ_BUF_SIZE - 1) {
        resetBuffer();
        return true;
      } else {
        return false;
      }
    }

    Stream& mSerial;
    uint8_t mIndex = 0;
    char mBuf[READ_BUF_SIZE];
    char mPushback = 0;
};

}
}

#endif
