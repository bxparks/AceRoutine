/*
MIT License

Copyright (c) 2018 Brian T. Park

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef ACE_ROUTINE_SERIAL_READER_H
#define ACE_ROUTINE_SERIAL_READER_H

#include <Arduino.h> // Stream

namespace ace_routine {
namespace cli {

/**
 * Reads tokens (lines, words, integers, characters, etc) from the Stream
 * device. The Stream will normally be the global Serial object. These are
 * non-blocking helper functions designed to be used inside COROUTINE() macros
 * from the AceRoutine library.
 */
class StreamReader {
  public:
    /**
     * Constructor.
     *
     * @param serial The input stream, usually the global Serial object.
     * @param buffer The input character buffer.
     * @param bufferSize The size of the buffer, should be set large enough to
     * hold the longest line without triggering buffer overflow.
     */
    StreamReader(Stream& serial, char* buf, int bufSize):
      mStream(serial),
      mBuf(buf),
      mBufSize(bufSize)
    {}

    /**
     * Get a character from serial port. Return true if parsing is finished,
     * otherwise false indicates that this method should be called again for
     * further processing.
     */
    bool getChar(char* c);

    /**
     * Get a string bounded by whitespace, i.e. a "word". Return true if
     * parsing is finished, otherwise false indicates that this method should
     * be called again for further processing.
     *
     * If isError is true, then a parsing error occurred. If is false, then
     * word contains the pointer to the word string
     */
    bool getWordString(bool* isError, char** word);

    /**
     * Get an integer. Return true if parsing is finished, otherwise false
     * indicates that this method should be called again for further
     * processing.
     *
     * If isError is true, then a parsing error occurred. If is false, then
     * value contains the integer.
     */
    bool getInteger(bool* isError, int* value);

    /**
     * Parse a comma. Return true if parsing is finished, otherwise false
     * indicates that this method should be called again for further
     * processing.
     *
     * If isError is true, then a parsing error ocurred.
     */
    bool getComma(bool* isError);

    /**
     * Skip whitespace. Return true if parsing is finished, otherwise false
     * indicates that this method should be called again for further processing.
     */
    bool skipWhiteSpace();

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
    bool getLine(bool* isError, char** line);

  private:
    // Disable copy-constructor and assignment operator
    StreamReader(const StreamReader&) = delete;
    StreamReader& operator=(const StreamReader&) = delete;

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
    bool addToBuffer(char c);

    Stream& mStream;
    char* const mBuf;
    int mBufSize;
    int mIndex = 0;
    char mPushback = 0;
};

}
}

#endif
