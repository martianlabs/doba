file(
  GLOB_RECURSE
  UNIT_TEST_SOURCES
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_LIST_DIR}/*_tests.cpp"
)
add_executable(
  doba_unit_tests
  "${CMAKE_CURRENT_LIST_DIR}/test_helper.cpp"
  ${UNIT_TEST_SOURCES}
)
target_include_directories(
  doba_unit_tests
  PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}"
)
target_link_libraries(
  doba_unit_tests
  PRIVATE
  doba_headers
)
add_test(
  NAME doba_unit_tests
  COMMAND doba_unit_tests
)

add_executable(
  doba_unit_test_helper_probe
  "${CMAKE_CURRENT_LIST_DIR}/test_helper.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/test_helper_probe.cpp"
)
target_link_libraries(
  doba_unit_test_helper_probe
  PRIVATE
  doba_headers
)
add_test(
  NAME doba_unit_test_helper
  COMMAND
    "${CMAKE_COMMAND}"
    "-DTEST_PROGRAM=$<TARGET_FILE:doba_unit_test_helper_probe>"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_test_helper.cmake"
)
set_tests_properties(
  doba_unit_tests
  doba_unit_test_helper
  PROPERTIES
  TIMEOUT 120
)
