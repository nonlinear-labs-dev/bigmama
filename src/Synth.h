#pragma once

#include "Types.h"
#include "RingBuffer.h"
#include "MidiEvent.h"

#include <memory>
#include <chrono>

class MidiInput;
class AudioOutput;
class Options;

class Synth
{
 public:
  Synth(const Options &options);
  virtual ~Synth();

  void start();
  void stop();

 protected:
  virtual void doMidi(const MidiEvent &event) = 0;
  virtual void doAudio(SampleFrame *target, size_t numFrames) = 0;

 private:
  void process(SampleFrame *target, size_t numFrames);
  void pushMidiEvent(const MidiEvent &event);

  const Options &m_options;

  std::unique_ptr<MidiInput> m_in;
  std::unique_ptr<AudioOutput> m_out;
  SamplePosition m_pos = 0;
  RingBuffer<MidiEvent, 2048> m_midiRingBuffer;
  std::chrono::high_resolution_clock::time_point m_startTime = std::chrono::high_resolution_clock::time_point::min();
};
