#include "AudioOutput.h"
#include <iostream>

AudioOutput::AudioOutput(const std::string& deviceName, Callback cb)
    : m_cb(cb)
{
  open(deviceName);
  start();
}

AudioOutput::~AudioOutput()
{
  close();
}

std::chrono::nanoseconds AudioOutput::getLatency() const
{
  return std::chrono::microseconds(static_cast<uint64_t>(m_latency));
}

void AudioOutput::close()
{
  m_run = false;

  if(m_bgThread.joinable())
    m_bgThread.join();

  if(auto h = std::exchange(m_handle, nullptr))
    snd_pcm_close(h);
}

void AudioOutput::open(const std::string& deviceName)
{
  snd_pcm_open(&m_handle, deviceName.c_str(), SND_PCM_STREAM_PLAYBACK, 0);

  snd_pcm_hw_params_t* hwparams = NULL;
  snd_pcm_sw_params_t* swparams = NULL;

  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_sw_params_alloca(&swparams);

  unsigned int sampleRate = c_sampleRate;
  unsigned int numBuffers = c_numAlsaBuffers;

  m_framesPerPeriod = c_desiredLatency * sampleRate / 1000;
  snd_pcm_uframes_t ringBufferFrames = numBuffers * m_framesPerPeriod;

  snd_pcm_hw_params_any(m_handle, hwparams);
  snd_pcm_hw_params_set_access(m_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
  snd_pcm_hw_params_set_format(m_handle, hwparams, SND_PCM_FORMAT_FLOAT);
  snd_pcm_hw_params_set_channels(m_handle, hwparams, 2);
  snd_pcm_hw_params_set_rate_near(m_handle, hwparams, &sampleRate, 0);
  snd_pcm_hw_params_set_periods(m_handle, hwparams, numBuffers, 0);
  snd_pcm_hw_params_set_period_size_near(m_handle, hwparams, &m_framesPerPeriod, 0);
  snd_pcm_hw_params_set_buffer_size_near(m_handle, hwparams, &ringBufferFrames);
  snd_pcm_hw_params(m_handle, hwparams);
  snd_pcm_sw_params_current(m_handle, swparams);
  snd_pcm_sw_params_set_start_threshold(m_handle, swparams, std::numeric_limits<int>::max());
  snd_pcm_sw_params_set_avail_min(m_handle, swparams, m_framesPerPeriod);
  snd_pcm_sw_params(m_handle, swparams);

  snd_pcm_hw_params_get_period_time(hwparams, &m_latency, 0);
  unsigned int periods = 0;
  snd_pcm_hw_params_get_periods(hwparams, &periods, 0);
  m_latency *= periods + 1;

  snd_pcm_start(m_handle);

  std::cout << "Midi2Audio latency is: " << m_latency / 1000 << "ms." << std::endl;
}

void AudioOutput::start()
{
  m_run = true;
  m_bgThread = std::thread([=]() { doBackgroundWork(); });
}

void AudioOutput::doBackgroundWork()
{
  struct sched_param param;
  param.sched_priority = 50;

  if(auto r = pthread_setschedparam(pthread_self(), SCHED_FIFO, &param))
    std::cerr << "Could not set thread priority - consider 'sudo setcap 'cap_sys_nice=eip' <application>'" << std::endl;

  while(true)
  {
    SampleFrame audio[m_framesPerPeriod];
    m_cb(audio, m_framesPerPeriod);

    auto result = snd_pcm_writei(m_handle, &audio, m_framesPerPeriod);

    if(!m_run)
      break;

    if(static_cast<snd_pcm_uframes_t>(result) != m_framesPerPeriod)
      handleWriteError(result);
  }
}

void AudioOutput::handleWriteError(snd_pcm_sframes_t result)
{
  if(result < 0)
  {
    if(auto recoverResult = snd_pcm_recover(m_handle, result, 1))
    {
      std::cerr << "Could not recover:" << recoverResult << std::endl;
    }
    else
    {
      std::cerr << "recovered from x-run" << std::endl;
      snd_pcm_start(m_handle);
    }
  }
}
