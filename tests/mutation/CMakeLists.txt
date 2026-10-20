cmake_minimum_required(VERSION 3.20)
project(doba_mutation_detection LANGUAGES CXX)
if(NOT CMAKE_SYSTEM_NAME MATCHES "^(Linux|Windows)$" OR
   NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
   CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  message(FATAL_ERROR "Mutation detection requires Linux or Windows clang++")
endif()
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_CONFIGURATION_TYPES)
  message(FATAL_ERROR "Mutation detection requires single-config Release")
endif()
if(NOT DOBA_MUTATION_ROOT OR NOT DOBA_MUTATION_HEADERS)
  message(FATAL_ERROR "Use run_detection.cmake to prepare the private header copy")
endif()
add_executable(detection_tests
  "${DOBA_MUTATION_ROOT}/tests/unit/helpers/test_helper.cpp"
  "${DOBA_MUTATION_ROOT}/tests/unit/protocol/http/v11/regression_detection_tests.cpp")
target_include_directories(detection_tests PRIVATE
  "${DOBA_MUTATION_HEADERS}" "${DOBA_MUTATION_ROOT}/tests/unit/helpers")
target_compile_features(detection_tests PRIVATE cxx_std_20)
find_package(Threads REQUIRED)
target_link_libraries(detection_tests PRIVATE Threads::Threads)
target_compile_options(detection_tests PRIVATE -g -Wall -Wextra -Wpedantic -Werror
  -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer)
target_link_options(detection_tests PRIVATE -fsanitize=address,undefined -fno-sanitize-recover=undefined)
include("${DOBA_MUTATION_ROOT}/cmake/clang_windows_test_runtime.cmake")
doba_clang_windows_test_runtime(detection_tests)
