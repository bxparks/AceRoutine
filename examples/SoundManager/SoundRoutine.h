#ifndef SOUND_ROUTINE_H
#define SOUND_ROUTINE_H

#include <AceRoutine.h>

const int SOUND_NONE = 0;
const int SOUND_BEEP = 1;
const int SOUND_BOOP = 2;

// Coroutine that knows how to play certain sounds.
class SoundRoutine : public ace_routine::Coroutine {
  public:
    int runCoroutine() override;
    void playSound(int sound);

  private:
    int currentSound = SOUND_NONE;
};

#endif
