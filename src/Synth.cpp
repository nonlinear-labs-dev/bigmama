#include "Synth.h"
#include "AudioOutput.h"
#include "AlsaMidiInput.h"
#include "TestMidiInput.h"
#include "Options.h"
#include "main.h"

#include <iostream>
#include <chrono>

Synth::Synth() = default;
Synth::~Synth() = default;

void Synth::start()
{
  auto options = getOptions();
  auto outDeviceName = options->getAudioOutputDeviceName();
  m_out = std::make_unique<AudioOutput>(outDeviceName, [=](auto buf, auto length) { process(buf, length); });

  if(options->generateMidiNotes())
  {
    m_in = std::make_unique<TestMidiInput>([=](auto event) { pushMidiEvent(event); });
  }
  else
  {
    auto inDeviceName = options->getMidiInputDeviceName();
    m_in = std::make_unique<AlsaMidiInput>(inDeviceName, [=](auto event) { pushMidiEvent(event); });
  }
}

void Synth::stop()
{
  m_in.reset();
  m_out.reset();
}

void Synth::doPushMidiEvent(const MidiEvent &event)
{
  auto &c = m_midiRingBuffer.push(event);
  auto now = std::chrono::high_resolution_clock::now();
  auto age = now - m_startTime;
  auto tsNano = std::chrono::duration_cast<std::chrono::nanoseconds>(age + m_out->getLatency());
  c.time.tick = static_cast<snd_seq_tick_time_t>(1.0 * tsNano.count() * getOptions()->getSampleRate() / std::nano::den);
}

void Synth::pushMidiEvent(const MidiEvent &event)
{
  if(m_startTime == std::chrono::high_resolution_clock::time_point::min())
    return;

  if(getOptions()->useMutex())
  {
    std::scoped_lock<std::mutex> lock(m_mutex);
    doPushMidiEvent(event);
  }
  else
  {
    doPushMidiEvent(event);
  }
}

void Synth::process(SampleFrame *target, size_t numFrames)
{
  if(m_startTime == std::chrono::high_resolution_clock::time_point::min())
    m_startTime = std::chrono::high_resolution_clock::now();

  if(getOptions()->useMutex())
  {
    std::scoped_lock<std::mutex> lock(m_mutex);
    processAudio(target, numFrames);
  }
  else
  {
    processAudio(target, numFrames);
  }
}

void Synth::processAudio(SampleFrame *target, size_t numFrames)
{
  if(auto e = m_midiRingBuffer.peek())
  {
    auto eventPos = e->time.tick;

    if(eventPos <= m_pos)
    {
      doMidi(*e);
      m_midiRingBuffer.pop();
      processAudio(target, numFrames);
      return;
    }

    auto spanLength = std::min(numFrames, eventPos - m_pos);
    doAudio(target, spanLength);
    m_pos += spanLength;

    if(auto leftOver = numFrames - spanLength)
      processAudio(target + spanLength, leftOver);
  }
  else
  {
    doAudio(target, numFrames);
    m_pos += numFrames;
  }
}
