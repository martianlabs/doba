file(
  GLOB_RECURSE
  INTEGRATION_TEST_SOURCES
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_LIST_DIR}/*_tests.cpp"
)
add_executable(
  doba_integration_tests
  "${CMAKE_CURRENT_LIST_DIR}/test_helper.cpp"
  ${INTEGRATION_TEST_SOURCES}
)
target_include_directories(
  doba_integration_tests
  PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}"
)
target_link_libraries(
  doba_integration_tests
  PRIVATE
  doba_headers
)
add_test(
  NAME doba_integration_tests
  COMMAND doba_integration_tests
)
set_tests_properties(
  doba_integration_tests
  PROPERTIES
  DEPENDS doba_unit_tests
)
