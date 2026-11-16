#pragma once

#if defined(__ANDROID__)

#include "pac/audio_engine.hpp"

#include <oboe/Oboe.h>
#include <memory>

namespace pac::platform::android {

// Low-latency Float32 output using Oboe. The AudioEngine and processor must
// outlive this object. Processor lifetime must also cover any active callback.
class OboeOutput final : public oboe::AudioStreamDataCallback {
public:
  OboeOutput() = default;
  ~OboeOutput() override;

  OboeOutput(const OboeOutput&) = delete;
  OboeOutput& operator=(const OboeOutput&) = delete;

  // Opens an output stream. The device's negotiated sample rate and channel
  // count are read back and used to initialize the engine.
  [[nodiscard]] oboe::Result open(AudioEngine& engine,
                                  const AudioFormat& requested_format) noexcept;
  [[nodiscard]] oboe::Result start() noexcept;
  [[nodiscard]] oboe::Result stop() noexcept;
  void close() noexcept;

  [[nodiscard]] bool is_open() const noexcept;
  [[nodiscard]] AudioFormat negotiated_format() const noexcept;

  oboe::DataCallbackResult onAudioReady(oboe::AudioStream* stream,
                                        void* audio_data,
                                        std::int32_t num_frames) override;

private:
  AudioEngine* engine_{nullptr};
  std::shared_ptr<oboe::AudioStream> stream_{};
  AudioFormat negotiated_format_{};
};

} // namespace pac::platform::android

#endif // defined(__ANDROID__)
