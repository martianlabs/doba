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
list(
  REMOVE_ITEM
  INTEGRATION_TEST_SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper_tests.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/protocol/http/v11/server_resilience_windows_tests.cpp"
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

if(WIN32)
  # A separate process gives resource measurements an isolated, warmed baseline.
  add_executable(doba_resilience_tests
    "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/protocol/http/v11/server_resilience_windows_tests.cpp")
  target_include_directories(doba_resilience_tests PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/helpers")
  target_link_libraries(doba_resilience_tests PRIVATE doba_headers)
  add_test(NAME doba_resilience_tests COMMAND doba_resilience_tests)
endif()
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

if(WIN32)
  set_tests_properties(doba_resilience_tests PROPERTIES TIMEOUT 120)
else()
  message(STATUS
    "Linux: legacy async, TLS and operating-policy integration tests excluded")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_test(NAME doba_linux_transport_tests
    COMMAND doba_integration_tests --file transport/server/tcpip_linux_tests.cpp)
  set_tests_properties(doba_linux_transport_tests PROPERTIES TIMEOUT 120)
  add_executable(doba_linux_accept_error_tests
    "${CMAKE_CURRENT_LIST_DIR}/transport/server/tcp_accept_error_linux.cpp")
  target_include_directories(doba_linux_accept_error_tests PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/helpers")
  target_link_libraries(doba_linux_accept_error_tests PRIVATE doba_headers)
  add_test(NAME doba_linux_accept_error_tests
    COMMAND doba_linux_accept_error_tests)
  set_tests_properties(doba_linux_accept_error_tests PROPERTIES TIMEOUT 15)
endif()

if(WIN32)
  # Isolate native API interception from all other transport instantiations.
  set(DOBA_IOCP_TEST_DIR "${CMAKE_CURRENT_LIST_DIR}/transport/server")
  add_executable(doba_iocp_completion_tests
    "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper.cpp"
    "${DOBA_IOCP_TEST_DIR}/tcp_iocp_completion_windows_probe.cpp")
  target_include_directories(doba_iocp_completion_tests PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/helpers")
  target_link_libraries(doba_iocp_completion_tests PRIVATE doba_headers)
  add_test(NAME doba_iocp_completion_tests COMMAND doba_iocp_completion_tests)
  set_tests_properties(doba_iocp_completion_tests PROPERTIES TIMEOUT 30)
endif()

if(WIN32)
  # Keep API fault injection separate from normal transport instantiations.
  add_executable(doba_windows_lifecycle_tests
    "${CMAKE_CURRENT_LIST_DIR}/helpers/test_helper.cpp"
    "${DOBA_IOCP_TEST_DIR}/tcp_lifecycle_windows_probe.cpp")
  target_include_directories(doba_windows_lifecycle_tests PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/helpers")
  target_link_libraries(doba_windows_lifecycle_tests PRIVATE doba_headers)
  add_test(NAME doba_windows_lifecycle_tests
    COMMAND doba_windows_lifecycle_tests)
  set_tests_properties(doba_windows_lifecycle_tests PROPERTIES TIMEOUT 45)
endif()
