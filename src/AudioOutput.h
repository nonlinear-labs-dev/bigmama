#pragma once

#include "Types.h"
#include <asoundlib.h>
#include <functional>
#include <thread>

class AudioOutput
{
 public:
  using Callback = std::function<void(SampleFrame *target, size_t numFrames)>;

  AudioOutput(const std::string &deviceName, Callback cb);
  ~AudioOutput();

  std::chrono::nanoseconds getLatency() const;

 private:
  void open(const std::string &deviceName);
  void start();
  void close();
  void doBackgroundWork();
  void handleWriteError(snd_pcm_sframes_t result);

  Callback m_cb;
  snd_pcm_t *m_handle = nullptr;
  std::thread m_bgThread;
  bool m_run = true;
  unsigned int m_latency = 0;
  snd_pcm_uframes_t m_framesPerPeriod = 0;
};
