#include "pac/audio_engine.hpp"

#include <thread>

namespace pac {

bool AudioEngine::initialize(const AudioFormat& format) noexcept {
  if (!format.is_valid() || format.sample_format != SampleFormat::Float32) {
    return false;
  }

  auto current = state_.load(std::memory_order_acquire);
  for (;;) {
    if (current == EngineState::Running || current == EngineState::Configuring) {
      return false;
    }
    if (state_.compare_exchange_weak(current, EngineState::Configuring,
                                     std::memory_order_acq_rel,
                                     std::memory_order_acquire)) {
      break;
    }
  }

  wait_for_callbacks();
  sample_rate_hz_.store(format.sample_rate_hz, std::memory_order_relaxed);
  channel_count_.store(format.channel_count, std::memory_order_relaxed);
  sample_format_.store(format.sample_format, std::memory_order_relaxed);
  state_.store(EngineState::Ready, std::memory_order_release);
  return true;
}

bool AudioEngine::start() noexcept {
  auto expected = EngineState::Ready;
  if (state_.compare_exchange_strong(expected, EngineState::Running,
                                     std::memory_order_acq_rel)) {
    return true;
  }
  expected = EngineState::Stopped;
  return state_.compare_exchange_strong(expected, EngineState::Running,
                                        std::memory_order_acq_rel);
}

void AudioEngine::wait_for_callbacks() noexcept {
  // This runs only on a control thread. The real-time callback never waits.
  while (active_callbacks_.load(std::memory_order_acquire) != 0) {
    std::this_thread::yield();
  }
}

void AudioEngine::stop() noexcept {
  auto expected = EngineState::Running;
  if (state_.compare_exchange_strong(expected, EngineState::Stopped,
                                     std::memory_order_acq_rel)) {
    wait_for_callbacks();
  }
}

void AudioEngine::reset() noexcept {
  stop();

  auto previous = state_.exchange(EngineState::Configuring,
                                  std::memory_order_acq_rel);
  if (previous == EngineState::Running) {
    // A concurrent start won the race with stop(). Quiesce it before reset.
    state_.store(EngineState::Stopped, std::memory_order_release);
    wait_for_callbacks();
    previous = state_.exchange(EngineState::Configuring,
                               std::memory_order_acq_rel);
  }
  (void)previous;

  wait_for_callbacks();
  processor_.store(nullptr, std::memory_order_release);
  sample_rate_hz_.store(0, std::memory_order_relaxed);
  channel_count_.store(0, std::memory_order_relaxed);
  sample_format_.store(SampleFormat::Float32, std::memory_order_relaxed);
  state_.store(EngineState::Uninitialized, std::memory_order_release);
}

bool AudioEngine::process(AudioBuffer& buffer) noexcept {
  if (state_.load(std::memory_order_acquire) != EngineState::Running) {
    return false;
  }

  active_callbacks_.fetch_add(1, std::memory_order_acq_rel);
  const auto finish_callback = [this]() noexcept {
    active_callbacks_.fetch_sub(1, std::memory_order_release);
  };

  // Close the race where stop() changes state after the first check but
  // before this callback registers itself as active.
  if (state_.load(std::memory_order_acquire) != EngineState::Running) {
    finish_callback();
    return false;
  }

  const AudioFormat active_format{
      sample_rate_hz_.load(std::memory_order_relaxed),
      channel_count_.load(std::memory_order_relaxed),
      sample_format_.load(std::memory_order_relaxed)};

  if (!buffer.is_valid() || buffer.format.sample_format != SampleFormat::Float32 ||
      buffer.format.sample_rate_hz != active_format.sample_rate_hz ||
      buffer.format.channel_count != active_format.channel_count) {
    finish_callback();
    return false;
  }

  if (auto* processor = processor_.load(std::memory_order_acquire)) {
    processor->process(buffer);
  }
  finish_callback();
  return true;
}

void AudioEngine::set_processor(AudioProcessor* processor) noexcept {
  processor_.store(processor, std::memory_order_release);
}

EngineState AudioEngine::state() const noexcept {
  return state_.load(std::memory_order_acquire);
}

AudioFormat AudioEngine::format() const noexcept {
  return AudioFormat{
      sample_rate_hz_.load(std::memory_order_relaxed),
      channel_count_.load(std::memory_order_relaxed),
      sample_format_.load(std::memory_order_relaxed)};
}

} // namespace pac
