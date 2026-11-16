#pragma once

#include "pac/audio_buffer.hpp"
#include "pac/audio_processor.hpp"

#include <atomic>
#include <cstdint>

namespace pac {

enum class EngineState : unsigned char {
  Uninitialized,
  Configuring,
  Ready,
  Running,
  Stopped
};

class AudioEngine final {
public:
  AudioEngine() noexcept = default;

  // Control methods (initialize/start/stop/reset) must be called from a
  // non-audio thread and must not be called concurrently with each other.
  [[nodiscard]] bool initialize(const AudioFormat& format) noexcept;
  [[nodiscard]] bool start() noexcept;
  void stop() noexcept;
  void reset() noexcept;

  // Real-time callback entry point. stop() waits for in-flight calls to drain.
  [[nodiscard]] bool process(AudioBuffer& buffer) noexcept;

  // The caller owns the processor and must keep it alive until it has been
  // detached and all callbacks that could have observed it have completed.
  void set_processor(AudioProcessor* processor) noexcept;

  [[nodiscard]] EngineState state() const noexcept;
  [[nodiscard]] AudioFormat format() const noexcept;

private:
  void wait_for_callbacks() noexcept;

  std::atomic<EngineState> state_{EngineState::Uninitialized};
  std::atomic<std::uint32_t> active_callbacks_{0};
  std::atomic<std::uint32_t> sample_rate_hz_{0};
  std::atomic<std::uint16_t> channel_count_{0};
  std::atomic<SampleFormat> sample_format_{SampleFormat::Float32};
  std::atomic<AudioProcessor*> processor_{nullptr};
};

} // namespace pac
