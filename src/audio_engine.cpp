#include "pac/audio_engine.hpp"
namespace pac {
bool AudioEngine::initialize(const AudioFormat& format) noexcept {
  if (!format.is_valid() || state_.load(std::memory_order_acquire) == EngineState::Running) return false;
  format_ = format;
  state_.store(EngineState::Ready, std::memory_order_release);
  return true;
}
bool AudioEngine::start() noexcept {
  auto expected = EngineState::Ready;
  if (state_.compare_exchange_strong(expected, EngineState::Running, std::memory_order_acq_rel)) return true;
  expected = EngineState::Stopped;
  return state_.compare_exchange_strong(expected, EngineState::Running, std::memory_order_acq_rel);
}
void AudioEngine::stop() noexcept {
  auto expected = EngineState::Running;
  (void)state_.compare_exchange_strong(expected, EngineState::Stopped, std::memory_order_acq_rel);
}
void AudioEngine::reset() noexcept {
  stop();
  processor_.store(nullptr, std::memory_order_release);
  state_.store(EngineState::Uninitialized, std::memory_order_release);
  format_ = AudioFormat{};
}
bool AudioEngine::process(AudioBuffer& buffer) noexcept {
  if (state_.load(std::memory_order_acquire) != EngineState::Running ||
      !buffer.is_valid() || buffer.format.sample_format != SampleFormat::Float32 ||
      buffer.format.sample_rate_hz != format_.sample_rate_hz ||
      buffer.format.channel_count != format_.channel_count) return false;
  if (auto* processor = processor_.load(std::memory_order_acquire)) processor->process(buffer);
  return true;
}
void AudioEngine::set_processor(AudioProcessor* processor) noexcept {
  processor_.store(processor, std::memory_order_release);
}
EngineState AudioEngine::state() const noexcept { return state_.load(std::memory_order_acquire); }
AudioFormat AudioEngine::format() const noexcept { return format_; }
} // namespace pac
