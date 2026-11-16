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

if(ANDROID)
  include(FetchContent)
  set(OBOE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(OBOE_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(oboe
    GIT_REPOSITORY https://github.com/google/oboe.git
    GIT_TAG 1.9.3
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(oboe)

  add_library(pac_android src/platform/android/oboe_output.cpp)
  add_library(PAC::Android ALIAS pac_android)
  target_include_directories(pac_android PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
  target_compile_features(pac_android PUBLIC cxx_std_20)
  target_link_libraries(pac_android PUBLIC PAC::Core oboe)
  set_target_properties(pac_android PROPERTIES
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON)
endif()

if(PAC_BUILD_TESTS AND NOT ANDROID)
  enable_testing()
  add_executable(pac_core_tests tests/audio_engine_tests.cpp)
  target_link_libraries(pac_core_tests PRIVATE PAC::Core)
  add_test(NAME pac_core_tests COMMAND pac_core_tests)
endif()
