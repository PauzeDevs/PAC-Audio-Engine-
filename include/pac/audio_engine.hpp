#pragma once
#include "pac/audio_buffer.hpp"
#include "pac/audio_processor.hpp"
#include <atomic>
namespace pac {
enum class EngineState : unsigned char { Uninitialized, Ready, Running, Stopped };
class AudioEngine final {
public:
  AudioEngine() noexcept = default;
  [[nodiscard]] bool initialize(const AudioFormat& format) noexcept;
  [[nodiscard]] bool start() noexcept;
  void stop() noexcept;
  void reset() noexcept;
  // Synchronously processes decoded Float32 PCM; not a device backend yet.
  [[nodiscard]] bool process(AudioBuffer& buffer) noexcept;
  void set_processor(AudioProcessor* processor) noexcept;
  [[nodiscard]] EngineState state() const noexcept;
  [[nodiscard]] AudioFormat format() const noexcept;
private:
  AudioFormat format_{};
  std::atomic<EngineState> state_{EngineState::Uninitialized};
  std::atomic<AudioProcessor*> processor_{nullptr};
};
} // namespace pac
