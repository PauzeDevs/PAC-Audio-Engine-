# Pauze Audio Core (PAC)

A cross-platform native audio-engine SDK by Pauze, built around a shared C++20 processing core and native Android/iOS output backends.

> **Status: Early development (0.1.0 foundation).** The current code is a PCM processing core and test harness, not yet a complete player, decoder, device backend, or bit-perfect playback solution.

## Current implementation
- Audio format and interleaved Float32 PCM buffer types.
- Engine lifecycle state machine.
- Pluggable synchronous PCM processor interface.
- Input format validation.
- CMake build and CTest unit tests.\n- Android low-latency Float32 output backend using Oboe (Android builds only).
- GitHub Actions CI on Linux, macOS, and Windows.

## Android backend\n\nThe Android-only `PAC::Android` target wraps Oboe for low-latency Float32 output. See [`examples/android/README.md`](examples/android/README.md) for dependency setup and usage. Android hardware testing is still required.\n\n## Build
Requirements: CMake 3.20+ and a C++20 compiler.

```bash
cmake -S . -B build -DPAC_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Real-time safety
AudioProcessor implementations must not allocate memory, perform file/network I/O, acquire blocking locks, or execute unbounded work in process(). Hosts must coordinate processor lifetime so a processor is not destroyed while an audio thread may still use it.

## Roadmap
1. Core PCM pipeline and lifecycle.
2. Native Android Oboe backend.
3. Native iOS Audio Unit backend.
4. Decoder modules and streaming source abstraction.
5. DSP: EQ, filters, gain, limiter and resampling.
6. Kotlin/Swift SDK packaging and examples.
7. Device testing, loopback measurements and release hardening.

## Output claims
PAC will report the actual negotiated device format and processing path. High-resolution source support does not guarantee high-resolution hardware output. Bit-perfect playback will only be claimed for verified paths that bypass sample-changing DSP and resampling.

## License
License selection is pending. Redistribution terms have not yet been established.
