#pragma once

#include <glibmm.h>

class Options
{
 public:
  Options(int &argc, char **&argv);

  bool generateMidiNotes() const;
  std::string getMidiInputDeviceName() const;
  std::string getAudioOutputDeviceName() const;

 private:
  Glib::ustring m_midiInputDeviceName;
  Glib::ustring m_audioOutputDeviceName;
  bool m_generateTestNotes = false;
};
