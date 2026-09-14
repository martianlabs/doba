option(
  DOBA_SKIP_BACKLOG_UNIT_TESTS
  "Temporarily skip unit tests tracked in BACKLOG.md"
  OFF
)
set(
  DOBA_UNIT_TEST_ARGUMENTS
)
if(
  DOBA_SKIP_BACKLOG_UNIT_TESTS
)
  set(
    DOBA_BACKLOG_UNIT_TESTS
    # BACKLOG.md B1.
    "tests/unit/protocol/http/common/headers/if_match_tests.cpp::check applies the entity tag byte grammar"
    "tests/unit/protocol/http/common/headers/if_none_match_tests.cpp::check applies the entity tag byte grammar"
    # BACKLOG.md B2.
    "tests/unit/protocol/http/v11/headers/via_tests.cpp::check accepts protocol and comment boundaries"
    # BACKLOG.md B3.
    "tests/unit/protocol/http/v11/headers/via_tests.cpp::check keeps commas inside comments in one member"
    # BACKLOG.md B4 (test expectation).
    "tests/unit/protocol/http/v11/headers/via_tests.cpp::check accepts an IPv6 received by host"
    # BACKLOG.md B5.
    "tests/unit/protocol/http/v11/response_tests.cpp::removing one Date preserves the remaining explicit Date"
    # BACKLOG.md B6 (contract pending).
    "tests/unit/protocol/http/common/router_handler_signature_tests.cpp::noexcept sync handlers retain signature compatibility"
    "tests/unit/protocol/http/common/router_handler_signature_tests.cpp::noexcept async handlers retain signature compatibility"
    # BACKLOG.md C4.
    "tests/unit/protocol/http/v11/headers/rules/routing_tests.cpp::absolute form accepts differing Host ports"
    "tests/unit/protocol/http/v11/headers/rules/routing_tests.cpp::absolute form authority overrides a different Host name"
    "tests/unit/protocol/http/v11/decoder_tests.cpp::absolute form uses its authority when Host differs"
    # BACKLOG.md C5 (query bug and full-buffer contract).
    "tests/unit/protocol/http/v11/decoder_tests.cpp::rejects query parameter overflow without truncating"
    "tests/unit/protocol/http/v11/decoder_tests.cpp::a full incomplete head terminates instead of stalling"
    # BACKLOG.md C6.
    "tests/unit/protocol/http/v11/headers/rules/directives_tests.cpp::accepts the TE connection option"
    "tests/unit/protocol/http/v11/decoder_tests.cpp::accepts TE with its required connection option"
  )
  message(
    WARNING
    "Temporary unit-test bypass enabled for BACKLOG.md B1-B6/C4-C6. "
    "Skipped cases remain unresolved; disable DOBA_SKIP_BACKLOG_UNIT_TESTS "
    "to run the full suite."
  )
  foreach(
    DOBA_BACKLOG_UNIT_TEST IN LISTS DOBA_BACKLOG_UNIT_TESTS
  )
    list(
      APPEND DOBA_UNIT_TEST_ARGUMENTS --exclude "${DOBA_BACKLOG_UNIT_TEST}"
    )
    message(
      STATUS "Excluded unit test: ${DOBA_BACKLOG_UNIT_TEST}"
    )
  endforeach()
endif()

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
  COMMAND doba_unit_tests ${DOBA_UNIT_TEST_ARGUMENTS}
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
    "-DVERIFY_UNIT_EXCLUSIONS=ON"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_test_helper.cmake"
)
set_tests_properties(
  doba_unit_tests
  doba_unit_test_helper
  PROPERTIES
  TIMEOUT 120
)
