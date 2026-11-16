#include "pac/audio_engine.hpp"
#include <array>
#include <cassert>
namespace {
class GainProcessor final : public pac::AudioProcessor {
public:
  void process(pac::AudioBuffer& buffer) noexcept override {
    for (float& sample : buffer.samples) sample *= 0.5F;
  }
};
}
int main() {
  pac::AudioEngine engine;
  const pac::AudioFormat format{48000, 2, pac::SampleFormat::Float32};
  assert(!engine.start());
  assert(!engine.initialize(pac::AudioFormat{0, 2, pac::SampleFormat::Float32}));
  assert(engine.initialize(format));
  assert(engine.state() == pac::EngineState::Ready);
  assert(engine.start());
  std::array<float, 4> samples{1.0F, -1.0F, 0.5F, -0.5F};
  pac::AudioBuffer buffer{samples, 2, format};
  GainProcessor gain;
  engine.set_processor(&gain);
  assert(engine.process(buffer));
  assert(samples[0] == 0.5F && samples[1] == -0.5F);
  assert(samples[2] == 0.25F && samples[3] == -0.25F);
  engine.stop();
  assert(engine.state() == pac::EngineState::Stopped);
  assert(!engine.process(buffer));
  engine.reset();
  assert(engine.state() == pac::EngineState::Uninitialized);
}
