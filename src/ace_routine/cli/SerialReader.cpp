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

#include <ctype.h> // atoi()
#include "SerialReader.h"

namespace ace_routine {
namespace cli {

bool SerialReader::getChar(char* c) {
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

bool SerialReader::getWordString(bool* isError, char** word) {
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

bool SerialReader::getInteger(bool* isError, int* value) {
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

bool SerialReader::getComma(bool* isError) {
  char c;
  if (!getChar(&c)) return false;
  *isError = (c != ',');
  return true;
}

bool SerialReader::skipWhiteSpace() {
  char c;
  while (getChar(&c)) {
    if (!isspace(c)) {
      pushback(c);
      return true;
    }
  }
  return false;
}

bool SerialReader::getLine(bool* isError, char** line) {
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

bool SerialReader::addToBuffer(char c) {
  mBuf[mIndex] = c;
  mIndex++;
  if (mIndex >= READ_BUF_SIZE - 1) {
    resetBuffer();
    return true;
  } else {
    return false;
  }
}

}
}
