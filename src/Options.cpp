#include "Options.h"
#include <glibmm/optiongroup.h>
#include <iostream>

Options::Options(int &argc, char **&argv)
{
  Glib::OptionGroup mainGroup("common", "common options");
  Glib::OptionContext ctx;

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

std::string Options::getMidiInputDeviceName() const
{
  return m_midiInputDeviceName;
}

std::string Options::getAudioOutputDeviceName() const
{
  return m_audioOutputDeviceName;
}
