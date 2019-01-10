#pragma once

#include <glibmm.h>

class Options
{
 public:
  Options(int &argc, char **&argv);

  bool generateMidiNotes() const;
  bool areXRunsFatal() const;
  std::string getMidiInputDeviceName() const;
  std::string getAudioOutputDeviceName() const;
  int getSampleRate() const;
  double getLatency() const;

 private:
  Glib::ustring m_midiInputDeviceName;
  Glib::ustring m_audioOutputDeviceName;
  bool m_generateTestNotes = false;
  bool m_fatalXRuns = false;
  int m_rate = 48000;
  double m_latency = 5;
};
