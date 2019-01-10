#include "Synth.h"
#include "AudioOutput.h"
#include "AlsaMidiInput.h"
#include "TestMidiInput.h"
#include "Options.h"

#include <iostream>
#include <chrono>

Synth::Synth(const Options &options)
    : m_options(options)
{
}

Synth::~Synth() = default;

void Synth::start()
{
  auto outDeviceName = m_options.getAudioOutputDeviceName();
  m_out = std::make_unique<AudioOutput>(outDeviceName, [=](auto buf, auto length) { process(buf, length); });

  if(m_options.generateMidiNotes())
  {
    m_in = std::make_unique<TestMidiInput>([=](auto event) { pushMidiEvent(event); });
  }
  else
  {
    auto inDeviceName = m_options.getMidiInputDeviceName();
    m_in = std::make_unique<AlsaMidiInput>(inDeviceName, [=](auto event) { pushMidiEvent(event); });
  }
}

void Synth::stop()
{
  m_in.reset();
  m_out.reset();
}

void Synth::pushMidiEvent(const MidiEvent &event)
{
  auto &c = m_midiRingBuffer.push(event);
  auto now = std::chrono::high_resolution_clock::now();
  auto age = now - m_startTime;
  auto tsNano = std::chrono::duration_cast<std::chrono::nanoseconds>(age + m_out->getLatency());
  c.time.tick = static_cast<snd_seq_tick_time_t>(1.0 * tsNano.count() * c_sampleRate / std::nano::den);
}

void Synth::process(SampleFrame *target, size_t numFrames)
{
  if(m_startTime == std::chrono::high_resolution_clock::time_point::min())
    m_startTime = std::chrono::high_resolution_clock::now();

  if(auto e = m_midiRingBuffer.peek())
  {
    auto eventPos = e->time.tick;

    if(eventPos <= m_pos)
    {
      doMidi(*e);
      m_midiRingBuffer.pop();
      process(target, numFrames);
      return;
    }

    auto spanLength = std::min(numFrames, eventPos - m_pos);
    doAudio(target, spanLength);
    m_pos += spanLength;

    if(auto leftOver = numFrames - spanLength)
      process(target + spanLength, leftOver);
  }
  else
  {
    doAudio(target, numFrames);
    m_pos += numFrames;
  }
}
