#include "pac/platform/android/oboe_output.hpp"

#if defined(__ANDROID__)

#include <algorithm>
#include <cstddef>

namespace pac::platform::android {

OboeOutput::~OboeOutput() { close(); }

oboe::Result OboeOutput::open(AudioEngine& engine,
                             const AudioFormat& requested_format) noexcept {
  if (stream_ || !requested_format.is_valid() ||
      requested_format.sample_format != SampleFormat::Float32) {
    return oboe::Result::ErrorInvalidFormat;
  }

  oboe::AudioStreamBuilder builder;
  builder.setDirection(oboe::Direction::Output)
      ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
      ->setFormat(oboe::AudioFormat::Float)
      ->setChannelCount(static_cast<std::int32_t>(requested_format.channel_count))
      ->setSampleRate(static_cast<std::int32_t>(requested_format.sample_rate_hz))
      ->setSharingMode(oboe::SharingMode::Exclusive)
      ->setDataCallback(this);

  oboe::Result result = builder.openStream(stream_);
  if (result != oboe::Result::OK) {
    // Exclusive mode is not available on every Android device.
    builder.setSharingMode(oboe::SharingMode::Shared);
    result = builder.openStream(stream_);
  }
  if (result != oboe::Result::OK || !stream_) {
    stream_.reset();
    return result;
  }

  const auto actual_rate = stream_->getSampleRate();
  const auto actual_channels = stream_->getChannelCount();
  if (actual_rate <= 0 || actual_channels <= 0) {
    stream_->close();
    stream_.reset();
    return oboe::Result::ErrorInvalidFormat;
  }

  negotiated_format_ = AudioFormat{
      static_cast<std::uint32_t>(actual_rate),
      static_cast<std::uint16_t>(actual_channels),
      SampleFormat::Float32};

  if (!negotiated_format_.is_valid() || !engine.initialize(negotiated_format_)) {
    stream_->close();
    stream_.reset();
    return oboe::Result::ErrorInvalidFormat;
  }

  engine_ = &engine;
  return oboe::Result::OK;
}

oboe::Result OboeOutput::start() noexcept {
  if (!stream_ || engine_ == nullptr) return oboe::Result::ErrorClosed;
  if (!engine_->start()) return oboe::Result::ErrorInvalidState;

  const oboe::Result result = stream_->requestStart();
  if (result != oboe::Result::OK) engine_->stop();
  return result;
}

oboe::Result OboeOutput::stop() noexcept {
  if (!stream_) return oboe::Result::OK;
  const oboe::Result result = stream_->requestStop();
  if (engine_ != nullptr) engine_->stop();
  return result;
}

void OboeOutput::close() noexcept {
  if (!stream_) {
    engine_ = nullptr;
    return;
  }
  (void)stop();
  (void)stream_->close();
  stream_.reset();
  engine_ = nullptr;
}

bool OboeOutput::is_open() const noexcept { return static_cast<bool>(stream_); }

AudioFormat OboeOutput::negotiated_format() const noexcept {
  return negotiated_format_;
}

oboe::DataCallbackResult OboeOutput::onAudioReady(oboe::AudioStream*,
                                                   void* audio_data,
                                                   std::int32_t num_frames) {
  if (audio_data == nullptr || num_frames <= 0 || engine_ == nullptr) {
    return oboe::DataCallbackResult::Stop;
  }

  const auto frame_count = static_cast<std::size_t>(num_frames);
  const auto sample_count = frame_count * negotiated_format_.channel_count;
  AudioBuffer buffer{
      std::span<float>(static_cast<float*>(audio_data), sample_count),
      frame_count,
      negotiated_format_};

  if (!engine_->process(buffer)) {
    std::fill(buffer.samples.begin(), buffer.samples.end(), 0.0F);
  }
  return oboe::DataCallbackResult::Continue;
}

} // namespace pac::platform::android

#endif // defined(__ANDROID__)
