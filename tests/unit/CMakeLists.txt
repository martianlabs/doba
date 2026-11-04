file(
  GLOB_RECURSE
  UNIT_TEST_SOURCES
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_LIST_DIR}/*_tests.cpp"
)
# Native implementation tests are separate translation units.
if(WIN32)
  list(FILTER UNIT_TEST_SOURCES EXCLUDE REGEX "_linux_tests\\.cpp$")
else()
  list(FILTER UNIT_TEST_SOURCES EXCLUDE REGEX "_windows_tests\\.cpp$")
endif()
list(
  REMOVE_ITEM
  UNIT_TEST_SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper_tests.cpp"
)
add_executable(
  doba_unit_tests
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper.cpp"
  ${UNIT_TEST_SOURCES}
)
target_include_directories(
  doba_unit_tests
  PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/helpers"
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
add_test(
  NAME doba_engine_concurrency_tests
  COMMAND doba_unit_tests
          --file protocol/http/v11/engine_concurrency_tests.cpp
)

add_executable(
  doba_unit_test_helper_probe
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper_probe.cpp"
)
target_link_libraries(
  doba_unit_test_helper_probe
  PRIVATE
  doba_headers
)
add_executable(
  doba_unit_test_helper_tests
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper_tests.cpp"
)
target_link_libraries(
  doba_unit_test_helper_tests
  PRIVATE
  doba_headers
)
target_compile_definitions(
  doba_unit_test_helper_tests
  PRIVATE
  DOBA_TEST_HELPER_PROGRAM="$<TARGET_FILE:doba_unit_test_helper_probe>"
)
add_dependencies(
  doba_unit_test_helper_tests
  doba_unit_test_helper_probe
)
add_test(
  NAME doba_unit_test_helper
  COMMAND doba_unit_test_helper_tests
)
set_tests_properties(
  doba_unit_tests
  doba_engine_concurrency_tests
  doba_unit_test_helper
  PROPERTIES
  TIMEOUT 120
)

# Run the same model in ordinary tests and coverage-guided fuzzing.
add_executable(doba_security_model_tests
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/protocol/http/v11/engine_security_state_tests.cpp")
target_include_directories(doba_security_model_tests PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/helpers")
target_link_libraries(doba_security_model_tests PRIVATE doba_headers)
add_test(NAME doba_security_model_tests COMMAND doba_security_model_tests)
set_tests_properties(doba_security_model_tests PROPERTIES TIMEOUT 60)

if(WIN32)
  # Isolate native API substitution from the ordinary unit binary.
  set(qualification "${CMAKE_CURRENT_LIST_DIR}/../qualification")
  add_executable(doba_network_fault_tests
    "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper.cpp"
    "${qualification}/network_environment_windows_probe.cpp")
  target_include_directories(doba_network_fault_tests PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/helpers")
  target_link_libraries(doba_network_fault_tests PRIVATE doba_headers)
  add_test(NAME doba_network_fault_tests COMMAND doba_network_fault_tests)
  set_tests_properties(doba_network_fault_tests PROPERTIES TIMEOUT 30)
endif()
