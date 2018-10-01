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

#include <AceRoutine.h>
using namespace ace_routine;

#if defined(ESP8266)
const unsigned long DURATION = 1000;  // prevent watch dog timer exception
#else
const unsigned long DURATION = 3000;
#endif

static unsigned long counter = 0;

COROUTINE(counterA) {
  COROUTINE_LOOP() {
    counter++;
    COROUTINE_YIELD();
  }
}

COROUTINE(counterB) {
  COROUTINE_LOOP() {
    counter++;
    COROUTINE_YIELD();
  }
}

Channel<long> masterOutSlaveInChannel;
Channel<long> masterInSlaveOutChannel;

class Incrementer: public Coroutine {
  public:
    Incrementer(Channel<long>& readChannel,
        Channel<long>& writeChannel, bool isMaster = false):
      mReadChannel(readChannel),
      mWriteChannel(writeChannel),
      mIsMaster(isMaster)
      {}

    virtual int runCoroutine() override {
      COROUTINE_LOOP() {
        long message;

        // Avoid a deadlock between the master and slave by making the master
        // write first, allowing the slave to read, and send a message back,
        // which allows the master to go to the read.
        if (counter == 0 && mIsMaster) {
          message = counter;
          COROUTINE_CHANNEL_WRITE(mWriteChannel, message);
        }

        COROUTINE_CHANNEL_READ(mReadChannel, message);
        message++;
        counter = message;
        COROUTINE_CHANNEL_WRITE(mWriteChannel, message);
      }
    }

  private:
    Channel<long>& mReadChannel;
    Channel<long>& mWriteChannel;
    bool mIsMaster;
};

Incrementer master(masterInSlaveOutChannel, masterOutSlaveInChannel,
    true /*isMaster*/);
Incrementer slave(masterOutSlaveInChannel, masterInSlaveOutChannel);

void doMasterSlaveChannel() {
  master.resume();
  slave.resume();
  counterA.suspend();
  counterB.suspend();

  counter = 0;

  unsigned long start = millis();
  yield();
  while (millis() - start < DURATION) {
    CoroutineScheduler::loop();
  }
  yield();
}

void doCoroutineCounter() {
  master.suspend();
  slave.suspend();
  counterA.resume();
  counterB.resume();

  counter = 0;
  unsigned long start = millis();
  yield();
  while (millis() - start < DURATION) {
    CoroutineScheduler::loop();
  }
  yield();
}

void printStats(float baselineDuration, float channelDuration) {
  char buf[100];
  float diff = channelDuration - baselineDuration;
  sprintf(buf, "      %2d.%02d |%2d.%02d |%2d.%02d |",
      (int)channelDuration, (int)(channelDuration*100)%100,
      (int)baselineDuration, (int)(baselineDuration*100)%100,
      (int)diff, (int)(diff*100)%100);
  Serial.println(buf);
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Leonardo/Micro

  master.setupCoroutine("master");
  slave.setupCoroutine("slave");
  CoroutineScheduler::setup();

  Serial.println(
      F("------------+------+------+"));
  Serial.println(
      F("    Channel | base | diff |"));
  Serial.println(
      F("------------+------+------+"));

  doMasterSlaveChannel();
  float channelDuration = DURATION * 1000.0 / counter;

  doCoroutineCounter();
  float baselineDuration = DURATION * 1000.0 / counter;

  printStats(baselineDuration, channelDuration);

  Serial.println(
      F("------------+------+------+"));
}

void loop() {}
