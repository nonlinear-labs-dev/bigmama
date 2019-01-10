#include "Options.h"
#include <glibmm/optiongroup.h>
#include <iostream>

Options::Options(int &argc, char **&argv)
{
  Glib::OptionGroup mainGroup("common", "common options");
  Glib::OptionContext ctx;

  Glib::OptionEntry mutex;
  mutex.set_long_name("mutex");
  mutex.set_short_name('u');
  mutex.set_description("Use a mutex for sync midi against audio thread");
  mainGroup.add_entry(mutex, m_useMutex);

  Glib::OptionEntry samplerate;
  samplerate.set_long_name("sample-rate");
  samplerate.set_short_name('r');
  samplerate.set_description("Samplerate of audio engine");
  mainGroup.add_entry(samplerate, m_rate);

  Glib::OptionEntry latency;
  latency.set_long_name("latency");
  latency.set_short_name('l');
  latency.set_description("Midi-In to Audio-Out round trip time");
  mainGroup.add_entry(latency, m_latency);

  Glib::OptionEntry midiIn;
  midiIn.set_long_name("midi-in");
  midiIn.set_short_name('m');
  midiIn.set_description("Name of the alsa midi input device");
  mainGroup.add_entry(midiIn, m_midiInputDeviceName);

  Glib::OptionEntry audioOut;
  audioOut.set_long_name("audio-out");
  audioOut.set_short_name('a');
  audioOut.set_description("Name of the alsa audio output device");
  mainGroup.add_entry(audioOut, m_audioOutputDeviceName);

  Glib::OptionEntry testNotes;
  testNotes.set_long_name("test-notes");
  testNotes.set_short_name('t');
  testNotes.set_description("Generate midi notes instead of using midi in");
  mainGroup.add_entry(testNotes, m_generateTestNotes);

  Glib::OptionEntry fatalXRuns;
  fatalXRuns.set_long_name("fatal-xruns");
  fatalXRuns.set_short_name('f');
  fatalXRuns.set_description("Terminate program in case of alsa underrun or overrun");
  mainGroup.add_entry(fatalXRuns, m_fatalXRuns);

  ctx.set_main_group(mainGroup);
  ctx.set_help_enabled(true);

  if(!ctx.parse(argc, argv))
    std::cerr << ctx.get_summary();

  if(m_audioOutputDeviceName.empty())
  {
    g_printerr("%s", ctx.get_help().c_str());
    exit(-1);
  }
}

bool Options::generateMidiNotes() const
{
  return m_generateTestNotes;
}

bool Options::areXRunsFatal() const
{
  return m_fatalXRuns;
}

bool Options::useMutex() const
{
  return m_useMutex;
}

std::string Options::getMidiInputDeviceName() const
{
  return m_midiInputDeviceName;
}

std::string Options::getAudioOutputDeviceName() const
{
  return m_audioOutputDeviceName;
}

int Options::getSampleRate() const
{
  return m_rate;
}

double Options::getLatency() const
{
  return m_latency;
}
