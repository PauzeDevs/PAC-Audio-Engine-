cmake_minimum_required(VERSION 3.20)
project(PauzeAudioCore VERSION 0.1.0 LANGUAGES CXX)

option(PAC_BUILD_TESTS "Build PAC unit tests" ON)

add_library(pac_core src/audio_engine.cpp)
add_library(PAC::Core ALIAS pac_core)
target_include_directories(pac_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(pac_core PUBLIC cxx_std_20)
set_target_properties(pac_core PROPERTIES CXX_EXTENSIONS OFF POSITION_INDEPENDENT_CODE ON)

if(MSVC)
  target_compile_options(pac_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(pac_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

if(PAC_BUILD_TESTS)
  enable_testing()
  add_executable(pac_core_tests tests/audio_engine_tests.cpp)
  target_link_libraries(pac_core_tests PRIVATE PAC::Core)
  add_test(NAME pac_core_tests COMMAND pac_core_tests)
endif()
