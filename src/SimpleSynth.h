#pragma once

#include "Synth.h"

class SimpleSynth : public Synth
{
 public:
  void doMidi(const MidiEvent &event) override;
  void doAudio(SampleFrame *target, size_t numFrames) override;

 private:
  struct Voice
  {
    float inc = 0;
    float phase = 0;
    float vol = 0;
  };

  std::array<Voice, c_numVoices> m_voices;
};
