#pragma once
#include "pac/audio_buffer.hpp"
namespace pac {
// process() must be real-time safe: no allocation, blocking locks, or I/O.
class AudioProcessor {
public:
  virtual ~AudioProcessor() = default;
  virtual void process(AudioBuffer& buffer) noexcept = 0;
};
} // namespace pac
