if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
   NOT CMAKE_SYSTEM_NAME MATCHES "^(Linux|Windows)$" OR
   CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  message(FATAL_ERROR "DOBA_BUILD_FUZZERS requires Linux or Windows clang++ with libFuzzer")
endif()
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_CONFIGURATION_TYPES)
  message(FATAL_ERROR "DOBA_BUILD_FUZZERS requires a single-config Release build")
endif()

add_executable(doba_fuzz_seed_corpus seed_corpus.cpp)
target_compile_features(doba_fuzz_seed_corpus PRIVATE cxx_std_20)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/clang_windows_test_runtime.cmake")

foreach(harness decoder chunked)
  add_executable(fuzz_${harness} ${harness}_fuzz.cpp)
  target_link_libraries(fuzz_${harness} PRIVATE doba_headers)
  target_compile_options(fuzz_${harness} PRIVATE
    -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=undefined
    -fno-omit-frame-pointer -g)
  target_link_options(fuzz_${harness} PRIVATE
    -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=undefined)
  if(WIN32)
    # libFuzzer on Windows requires the static CRT and non-incremental linking.
    # Prebuilt libFuzzer uses the unannotated MSVC STL container ABI.
    target_compile_definitions(fuzz_${harness} PRIVATE
      _DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)
  endif()
  doba_clang_windows_test_runtime(fuzz_${harness})
endforeach()
