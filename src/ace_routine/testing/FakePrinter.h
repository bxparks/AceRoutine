#ifndef ACE_ROUTINE_FAKE_PRINTER_H
#define ACE_ROUTINE_FAKE_PRINTER_H

#include <Print.h>

// TODO: Move this to a common place, like AUnit/src/aunit/testing.

namespace ace_routine {
namespace testing {

/**
 * A subclass of Print that saves the characters in an internal buffer so that
 * it can be retrieved for unit testing.
 */
class FakePrinter: public Print {
  public:
    FakePrinter() { clear(); }

    virtual size_t write(uint8_t c) override {
      if (mPos < kBufferSize - 1) {
        mBuffer[mPos] = c;
        mPos++;
        return 1;
      }
      return 0;
    }

    virtual void flush() override { clear(); }

    const uint8_t* getBuffer() { return mBuffer; }

  private:
    static const int kBufferSize = 32;

    void clear() {
      mPos = 0;
      memset(mBuffer, 0, kBufferSize);
    }

    int mPos = 0;
    uint8_t mBuffer[kBufferSize];
};

}
}

#endif
