file(
  GLOB_RECURSE
  INTEGRATION_TEST_SOURCES
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_LIST_DIR}/*_tests.cpp"
)
# Native implementation tests are separate translation units.
if(WIN32)
  list(FILTER INTEGRATION_TEST_SOURCES EXCLUDE REGEX "_linux_tests\\.cpp$")
else()
  list(FILTER INTEGRATION_TEST_SOURCES EXCLUDE REGEX "_windows_tests\\.cpp$")
endif()
if(NOT DOBA_ENABLE_TLS)
  list(REMOVE_ITEM INTEGRATION_TEST_SOURCES
       "${CMAKE_CURRENT_LIST_DIR}/transport/server/tls_tests.cpp"
       "${CMAKE_CURRENT_LIST_DIR}/transport/server/tls_http_tests.cpp")
endif()
list(
  REMOVE_ITEM
  INTEGRATION_TEST_SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper_tests.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/protocol/http/v11/server_resilience_tests.cpp"
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
if(
  DOBA_ENABLE_TLS
)
  target_link_libraries(
    doba_integration_tests
    PRIVATE
    doba_tls
  )
endif()
add_test(
  NAME doba_integration_tests
  COMMAND doba_integration_tests
)

# A separate process gives resource measurements an isolated, warmed baseline.
add_executable(doba_resilience_tests
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/protocol/http/v11/server_resilience_tests.cpp")
target_include_directories(doba_resilience_tests PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/helpers")
target_link_libraries(doba_resilience_tests PRIVATE doba_headers)
if(DOBA_ENABLE_TLS)
  target_link_libraries(doba_resilience_tests PRIVATE doba_tls)
endif()
add_test(NAME doba_resilience_tests COMMAND doba_resilience_tests)
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
  doba_resilience_tests
  doba_integration_test_helper
  PROPERTIES
  TIMEOUT 120
)
