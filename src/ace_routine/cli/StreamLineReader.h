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

#ifndef ACE_ROUTINE_STREAM_LINE_READER_H
#define ACE_ROUTINE_STREAM_LINE_READER_H

#include <Arduino.h> // Stream
#include <AceRoutine.h>
#include "InputLine.h"

namespace ace_routine {
namespace cli {

/**
 * An AceRoutine coroutine that reads lines (terminated by '\\n' or '\\r' from
 * the Stream device, and write the InputLine message into the provided
 * Channel. The Stream will normally be the global Serial object.
 */
class StreamLineReader: public ace_routine::Coroutine {
  public:
    /**
     * Constructor.
     *
     * @param channel The output Channel used to send an InputLine message
     *        back to the receiver.
     * @param stream The input stream, usually the global Serial object.
     * @param buffer The input character buffer.
     * @param bufferSize The size of the buffer, should be set large enough to
     *        hold the longest line without triggering buffer overflow.
     */
    StreamLineReader(ace_routine::Channel<InputLine>& channel, Stream& stream,
        char* buffer, int bufferSize):
      mChannel(channel),
      mStream(stream),
      mBuf(buffer),
      mBufSize(bufferSize)
    {}

    virtual int runCoroutine() override {
      InputLine input;
      COROUTINE_LOOP() {
        COROUTINE_AWAIT(mStream.available() > 0);
        while (mStream.available() > 0) {
          char c = mStream.read();
          mBuf[mIndex] = c;
          mIndex++;
          if (mIndex >= mBufSize - 1) {
            input.status = InputLine::kStatusOverflow;
            input.line = mBuf;
            mFlushLine = true;
            resetBuffer();
            COROUTINE_CHANNEL_WRITE(mChannel, input);
          } else if (c == '\n' || c == '\r') {
            input.status =
                mFlushLine ? InputLine::kStatusOverflow : InputLine::kStatusOk;
            input.line = mBuf;
            mFlushLine = false;
            resetBuffer();
            COROUTINE_CHANNEL_WRITE(mChannel, input);
          }
        }
      }
    }

  private:
    // Disable copy-constructor and assignment operator
    StreamLineReader(const StreamLineReader&) = delete;
    StreamLineReader& operator=(const StreamLineReader&) = delete;

    /**
     * Terminate the current buffer with the NUL character (so that the current
     * string can be retrieved), and reset the character index to the
     * beginning of the buffer to prepare for the next getChar() or getLine().
     */
    void resetBuffer() {
      mBuf[mIndex] = '\0';
      mIndex = 0;
    }

    ace_routine::Channel<InputLine>& mChannel;
    Stream& mStream;
    char* const mBuf;
    int const mBufSize;

    int mIndex = 0;
    bool mFlushLine = false;
};

}
}

#endif
