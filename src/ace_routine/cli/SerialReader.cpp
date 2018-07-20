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
