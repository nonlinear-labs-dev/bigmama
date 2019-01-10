#pragma once

#include "MidiInput.h"

#include <thread>
#include <atomic>

class TestMidiInput : public MidiInput
{
 public:
  TestMidiInput(Callback cb);
  ~TestMidiInput();

 private:
  void doBackgroundWork();

  std::atomic<bool> m_run = true;
  std::thread m_bgThread;
};
