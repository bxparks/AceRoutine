/*
 * This sketch attempts to find the amount of CPU overhead involved in
 * Channel::write() and Channel::read(). It uses 2 coroutines to ping-pong
 * integer back and forth, incrementing on each step. Each step causes one
 * COROUTINE_CHANNEL_READ() and one COROUTINE_CHANNEL_WRITE(). The baseline is
 * the amount of time taken by 2 coroutines incremeting a global 'counter'
 * variable directly. The difference is roughly the amount of overhead
 * caused by using a Channel to send a message back and forth.
 *
 * We don't include a yield() inside the timing loop, because we don't want to
 * count the time spent inside the yield() call itself. In real world
 * programming, the yield() call cause additional latency of a Channel because
 * the synchronization provided by the Channel causes additional loops through
 * the Coroutine::loop() method, which causes additional calls to yield().
 */

#include <stdint.h> // uint32_t
#include <Arduino.h>
#include <AceRoutine.h>
#include <AceCommon.h> // printUint32AsFloat3To()
using namespace ace_routine;
using ace_common::printUint32AsFloat3To;

#if ! defined(SERIAL_PORT_MONITOR)
	#define SERIAL_PORT_MONITOR Serial
#endif

#if defined(EPOXY_DUINO)
	const uint32_t NUM_COUNT = 2000000;
#elif defined(ARDUINO_ARCH_AVR)
  const uint32_t NUM_COUNT = 10000;
#elif defined(ARDUINO_ARCH_STM32)
  const uint32_t NUM_COUNT = 50000;
#elif defined(ESP8266)
  const uint32_t NUM_COUNT = 10000;  // prevent watch dog timer exception
#elif defined(ESP32)
  const uint32_t NUM_COUNT = 100000;
#elif defined(TEENSYDUINO)
  const uint32_t NUM_COUNT = 100000;
#else
  const uint32_t NUM_COUNT = 300000;
#endif

static volatile uint32_t counter = 0;
static volatile uint32_t writeCounter = 0;
static volatile uint32_t readCounter = 0;
static volatile uint32_t readPayload;

// Coroutine that simply increments the counter.
COROUTINE(countCoroutine) {
  COROUTINE_LOOP() {
    counter++;
    COROUTINE_YIELD();
  }
}

Channel<uint32_t> channel;

// A class that writes to a channel.
class WriteCoroutine: public Coroutine {
  public:
    WriteCoroutine(Channel<uint32_t>& channel):
        mChannel(channel)
        {}

    int runCoroutine() override {
      COROUTINE_LOOP() {
        writeCounter++;
        uint32_t payload = writeCounter;
        COROUTINE_CHANNEL_WRITE(mChannel, payload);
      }
    }

  private:
    Channel<uint32_t>& mChannel;
};

WriteCoroutine writeCoroutine(channel);

// A class that reads from a channel.
class ReadCoroutine: public Coroutine {
  public:
    ReadCoroutine(Channel<uint32_t>& channel):
        mChannel(channel)
        {}

    int runCoroutine() override {
      COROUTINE_LOOP() {
        readCounter++;
        uint32_t payload;
        COROUTINE_CHANNEL_READ(mChannel, payload);
        readPayload = payload;
      }
    }

  private:
    Channel<uint32_t>& mChannel;
};

ReadCoroutine readCoroutine(channel);

//-----------------------------------------------------------------------------

// Determine time taken by just the counter.
uint32_t benchmarkCountCoroutine() {
  // Disable the channel writer and reader.
  countCoroutine.resume();
  writeCoroutine.suspend();
  readCoroutine.suspend();

  counter = writeCounter = readCounter = 0;
  yield();
  uint32_t startMillis = millis();
  while (counter < NUM_COUNT) {
    CoroutineScheduler::loop();
  }
  uint32_t elapsedMillis = millis() - startMillis;
  yield();
  return elapsedMillis;
}

// Determine time taken by adding the read and write coroutines
uint32_t benchmarkReadWriteChannels() {
  countCoroutine.resume();
  writeCoroutine.resume();
  readCoroutine.resume();

  counter = writeCounter = readCounter = 0;
  yield();
  uint32_t startMillis = millis();
  while (counter < NUM_COUNT) {
    CoroutineScheduler::loop();
  }
  uint32_t elapsedMillis = millis() - startMillis;
  yield();
  return elapsedMillis;
}

void printStats(
    const __FlashStringHelper* name,
    uint32_t durationMillis,
    uint32_t counter,
    uint32_t writeCounter,
    uint32_t readCounter) {
  uint32_t nanos = durationMillis * 1000 / (counter / 1000);
  SERIAL_PORT_MONITOR.print(name);
  SERIAL_PORT_MONITOR.print(' ');
  printUint32AsFloat3To(SERIAL_PORT_MONITOR, nanos);
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(counter);
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(writeCounter);
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(readCounter);
  SERIAL_PORT_MONITOR.println();
}

//-----------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000);
#endif
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  CoroutineScheduler::setup();

  SERIAL_PORT_MONITOR.println(F("BENCHMARKS"));

  uint32_t durationMillis = benchmarkCountCoroutine();
  printStats(F("Baseline"), durationMillis, NUM_COUNT,
      writeCounter, readCounter);

  durationMillis = benchmarkReadWriteChannels();
  printStats(F("Channels"), durationMillis, NUM_COUNT,
      writeCounter, readCounter);

  SERIAL_PORT_MONITOR.println(F("END"));

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {}
