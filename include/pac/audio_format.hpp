#pragma once
#include <cstdint>
namespace pac {
enum class SampleFormat : std::uint8_t { Float32, Int16, Int24, Int32 };
struct AudioFormat {
  std::uint32_t sample_rate_hz{48000};
  std::uint16_t channel_count{2};
  SampleFormat sample_format{SampleFormat::Float32};
  [[nodiscard]] constexpr bool is_valid() const noexcept {
    return sample_rate_hz >= 8000 && sample_rate_hz <= 384000 &&
           channel_count >= 1 && channel_count <= 8;
  }
};
} // namespace pac
