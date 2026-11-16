#pragma once
#include "pac/audio_format.hpp"
#include <cstddef>
#include <span>
namespace pac {
// Non-owning view over interleaved Float32 PCM. Caller owns sample memory.
struct AudioBuffer {
  std::span<float> samples;
  std::size_t frame_count{0};
  AudioFormat format{};
  [[nodiscard]] constexpr bool is_valid() const noexcept {
    return format.is_valid() &&
           samples.size() == frame_count * format.channel_count;
  }
};
} // namespace pac
