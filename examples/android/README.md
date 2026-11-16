# Android Oboe backend integration

PAC's Android output backend uses [Oboe](https://github.com/google/oboe) and
requires Android NDK/CMake plus the Oboe Android dependency.

## Gradle dependency

Add Oboe to the Android application's module dependencies:

```kotlin
dependencies {
    implementation(" com.google.oboe:oboe:1.9.3")
}
```

Remove the leading space inside the dependency string if copying: the
coordinate is `com.google.oboe:oboe:1.9.3`.

## CMake

Make PAC available to the Android native target and link Oboe:

```cmake
find_package(oboe REQUIRED CONFIG)
add_subdirectory(path/to/PAC-Audio-Engine- pac)
target_link_libraries(your-native-target PRIVATE PAC::Core PAC::Android Oboe)
```

The Android package must expose the Oboe CMake package to `find_package`.
The exact imported target can vary by Oboe distribution; use the target
exported by the dependency in your project.

## Minimal native usage

```cpp
#include "pac/platform/android/oboe_output.hpp"

pac::AudioEngine engine;
pac::platform::android::OboeOutput output;

const pac::AudioFormat requested{
    48000, 2, pac::SampleFormat::Float32
};

if (output.open(engine, requested) == oboe::Result::OK) {
    (void) output.start();
}

// On teardown:
(void) output.stop();
output.close();
```

The backend requests low-latency Float32 output and tries exclusive sharing
first, falling back to shared mode. Always use `negotiated_format()` rather
than assuming the requested rate/channels were accepted.

## Important limits

- This is a device-output backend, not a decoder or complete player.
- It currently accepts interleaved Float32 PCM only.
- The host must feed decoded PCM into the engine's processor pipeline.
- Do not allocate, lock, log, or perform I/O in `AudioProcessor::process()`.
- The engine and any installed processor must remain alive while callbacks
  can run. Stop and close the stream before destroying either.
- Android hardware/device testing is still required; desktop CI cannot verify
  real audio output, latency, or device-specific behavior.
