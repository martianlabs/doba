cmake_minimum_required(
  VERSION 3.20
)

project(
  doba_httparena_http_v11
  LANGUAGES CXX
)

set(
  CMAKE_CXX_STANDARD
  20
)
set(
  CMAKE_CXX_STANDARD_REQUIRED
  ON
)

set(
  DOBA_INCLUDE_DIR
  "${CMAKE_CURRENT_LIST_DIR}/../../../../include"
  CACHE PATH "Path to the doba include directory"
)

find_package(
  Threads
  REQUIRED
)

add_executable(
  doba_httparena_http_v11
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_include_directories(
  doba_httparena_http_v11
  PRIVATE
  "${DOBA_INCLUDE_DIR}"
)

target_link_libraries(
  doba_httparena_http_v11
  PRIVATE
  Threads::Threads
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

# The standalone driver remains dependency-light. Root test builds also inherit
# doba's compiler warnings and register a correctness check, not a load test.
if(TARGET doba_headers)
  target_link_libraries(doba_httparena_http_v11 PRIVATE doba_headers)
endif()
if(DOBA_BUILD_TESTS AND BUILD_TESTING)
  find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)
  add_test(NAME doba_httparena_tests COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_LIST_DIR}/../../../../tests/integration/benchmarks/httparena.py"
    "$<TARGET_FILE:doba_httparena_http_v11>" "${CMAKE_CURRENT_LIST_DIR}/meta.json")
  set_tests_properties(doba_httparena_tests PROPERTIES TIMEOUT 90)
endif()
