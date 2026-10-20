if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "DOBA_BUILD_FUZZERS requires Linux Clang with libFuzzer")
endif()
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_CONFIGURATION_TYPES)
  message(FATAL_ERROR "DOBA_BUILD_FUZZERS requires a single-config Release build")
endif()

add_executable(doba_fuzz_seed_corpus seed_corpus.cpp)
target_compile_features(doba_fuzz_seed_corpus PRIVATE cxx_std_20)

foreach(harness decoder chunked)
  add_executable(fuzz_${harness} ${harness}_fuzz.cpp)
  target_link_libraries(fuzz_${harness} PRIVATE doba_headers)
  target_compile_options(fuzz_${harness} PRIVATE
    -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=undefined
    -fno-omit-frame-pointer -g)
  target_link_options(fuzz_${harness} PRIVATE
    -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=undefined)
endforeach()
