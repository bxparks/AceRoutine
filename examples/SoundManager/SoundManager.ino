/*
 * An example of using the Coroutine::reset() from on coroutine to control the
 * state of another coroutine. The SoundRoutine plays different sounds
 * depending on the 'currentSound' variable. The soundManager coroutine is able
 * to interrupt and reset the SoundRoutine to play a different sound.
 *
 * Adapted from the example posted by EkardNT@ on
 * https://github.com/bxparks/AceRoutine/issues/20.
 *
 * The output of this program should be the following:
 *
 * @verbatim
 * <silence>
 * Wait 5 seconds...
 * 
 * Request Beep and wait 5 seconds...
 * Coroutine soundRoutine; status: Yielding
 * Coroutine soundManager; status: Running
 * First BEEP
 * Second BEEP
 * 
 * Request Boop and wait 5 seconds...
 * Coroutine soundRoutine; status: Yielding
 * Coroutine soundManager; status: Running
 * First BOOP
 * Second BOOP
 * 
 * Request Silence and wait 5 seconds...
 * Coroutine soundRoutine; status: Yielding
 * Coroutine soundManager; status: Running
 * Wait 5 seconds...
 * <silence>
 * 
 * Request Beep and wait 5 seconds...
 * Coroutine soundRoutine; status: Yielding
 * Coroutine soundManager; status: Running
 * First BEEP
 * Second BEEP
 *
 * ...
 * @endverbatim
 */

#include <Arduino.h>
#include <AceRoutine.h>

using namespace ace_routine;

const int SOUND_NONE = 0;
const int SOUND_BEEP = 1;
const int SOUND_BOOP = 2;

// Coroutine that knows how to play certain sounds.
class SoundRoutine : public Coroutine {
  public:
    int runCoroutine() override;
    void playSound(int sound);

  private:
    int currentSound = SOUND_NONE;
};

void SoundRoutine::playSound(int sound) {
  currentSound = sound;
  reset();
}

int SoundRoutine::runCoroutine() {
  COROUTINE_LOOP() {
    switch (currentSound) {
      case SOUND_NONE:
        Serial.println("<silence>");
        break;

      case SOUND_BEEP:
        // Calls to tone() go here, interspersed with some COROUTINE_DELAYs.
        Serial.println("First BEEP");
        COROUTINE_DELAY(500);
        Serial.println("Second BEEP");
        break;

      case SOUND_BOOP:
        // Calls to tone() go here, interspersed with some COROUTINE_DELAYs.
        Serial.println("First BOOP");
        COROUTINE_DELAY(500);
        Serial.println("Second BOOP");
        break;

      default:
        Serial.println("Unknown sound!");
    }

    currentSound = SOUND_NONE;
    COROUTINE_AWAIT(currentSound != SOUND_NONE);
  }
}

SoundRoutine soundRoutine;

// The soundManager controls what the soundRoutine will play.
COROUTINE(soundManager) {
  COROUTINE_LOOP() {
    Serial.println("Wait 5 seconds...");
    COROUTINE_DELAY(5000);

    Serial.println();
    Serial.println("Request Beep and wait 5 seconds...");
    CoroutineScheduler::list(Serial);
    soundRoutine.playSound(SOUND_BEEP);
    COROUTINE_DELAY(5000);

    Serial.println();
    Serial.println("Request Boop and wait 5 seconds...");
    CoroutineScheduler::list(Serial);
    soundRoutine.playSound(SOUND_BOOP);
    COROUTINE_DELAY(5000);

    Serial.println();
    Serial.println("Request Silence and wait 5 seconds...");
    CoroutineScheduler::list(Serial);
    soundRoutine.playSound(SOUND_NONE);
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // needed for Leonardo/Micro

  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}
