file(
  GLOB_RECURSE
  INTEGRATION_TEST_SOURCES
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_LIST_DIR}/*_tests.cpp"
)
list(
  REMOVE_ITEM
  INTEGRATION_TEST_SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper_tests.cpp"
)
add_executable(
  doba_integration_tests
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper.cpp"
  ${INTEGRATION_TEST_SOURCES}
)
target_include_directories(
  doba_integration_tests
  PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/helpers"
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

add_executable(
  doba_integration_test_helper_probe
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper_probe.cpp"
)
target_link_libraries(
  doba_integration_test_helper_probe
  PRIVATE
  doba_headers
)
add_executable(
  doba_integration_test_helper_tests
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper_tests.cpp"
)
target_link_libraries(
  doba_integration_test_helper_tests
  PRIVATE
  doba_headers
)
target_compile_definitions(
  doba_integration_test_helper_tests
  PRIVATE
  DOBA_TEST_HELPER_PROGRAM="$<TARGET_FILE:doba_integration_test_helper_probe>"
)
add_dependencies(
  doba_integration_test_helper_tests
  doba_integration_test_helper_probe
)
add_test(
  NAME doba_integration_test_helper
  COMMAND doba_integration_test_helper_tests
)
set_tests_properties(
  doba_integration_tests
  doba_integration_test_helper
  PROPERTIES
  TIMEOUT 120
)
