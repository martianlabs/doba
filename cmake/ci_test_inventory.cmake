cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED DOBA_CI_BUILD_DIR)
  message(FATAL_ERROR "DOBA_CI_BUILD_DIR is required")
endif()
get_filename_component(DOBA_CI_BUILD_DIR "${DOBA_CI_BUILD_DIR}" ABSOLUTE)
if(NOT DEFINED DOBA_CI_INVENTORY_TIMEOUT)
  set(DOBA_CI_INVENTORY_TIMEOUT 30)
endif()
if(NOT DOBA_CI_INVENTORY_TIMEOUT MATCHES "^[1-9][0-9]*$")
  message(FATAL_ERROR "DOBA_CI_INVENTORY_TIMEOUT must be positive seconds")
endif()
set(evidence "${DOBA_CI_BUILD_DIR}/ci-evidence")
file(MAKE_DIRECTORY "${evidence}")

if(DEFINED DOBA_CI_SOURCE_DESCRIPTION)
  set(revision "${DOBA_CI_SOURCE_DESCRIPTION}")
else()
  find_package(Git REQUIRED)
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_LIST_DIR}/.." rev-parse HEAD
    OUTPUT_VARIABLE revision
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
  )
endif()
string(CONCAT configuration_pattern
  "^(CMAKE_BUILD_TYPE|CMAKE_CXX_COMPILER|CMAKE_CXX_FLAGS[^:]*|"
  "CMAKE_EXE_LINKER_FLAGS[^:]*|DOBA_ENABLE_TLS|DOBA_ENABLE_STRICT_WARNINGS|"
  "DOBA_BUILD_EXAMPLES):")
file(STRINGS "${DOBA_CI_BUILD_DIR}/CMakeCache.txt" configuration
  REGEX "${configuration_pattern}")
string(REPLACE ";" "\n" configuration "${configuration}")
file(WRITE "${evidence}/configuration.txt"
  "Revision: ${revision}\nRunner: $ENV{RUNNER_OS}\n"
  "CMake: ${CMAKE_VERSION}\n${configuration}\n")
if(NOT configuration MATCHES "CMAKE_BUILD_TYPE:STRING=Release(\n|$)")
  message(FATAL_ERROR "CI tests must execute a Release build")
endif()

if(WIN32)
  set(suffix ".exe")
else()
  set(suffix "")
endif()
set(suites unit integration)
if(WIN32)
  list(APPEND suites resilience)
endif()
foreach(suite IN LISTS suites)
  if(suite STREQUAL "resilience")
    set(directory integration)
  else()
    set(directory "${suite}")
  endif()
  set(binary
    "${DOBA_CI_BUILD_DIR}/tests/${directory}/doba_${suite}_tests${suffix}")
  set(inventory_file "${evidence}/${suite}-tests.txt")
  set(errors_file "${evidence}/${suite}-inventory-errors.txt")
  message(STATUS
    "Listing ${suite} tests (timeout: ${DOBA_CI_INVENTORY_TIMEOUT}s)")
  string(TIMESTAMP started "%s" UTC)
  # Write directly to disk so a terminated process leaves its partial output.
  execute_process(
    COMMAND "${binary}" --list
    RESULT_VARIABLE result
    OUTPUT_FILE "${inventory_file}"
    ERROR_FILE "${errors_file}"
    TIMEOUT "${DOBA_CI_INVENTORY_TIMEOUT}"
  )
  string(TIMESTAMP finished "%s" UTC)
  math(EXPR elapsed "${finished} - ${started}")
  file(READ "${inventory_file}" inventory)
  file(READ "${errors_file}" errors)
  file(WRITE "${evidence}/${suite}-inventory-status.txt"
    "Command: ${binary} --list\nResult: ${result}\nElapsed: ${elapsed}s\n"
    "Timeout: ${DOBA_CI_INVENTORY_TIMEOUT}s\n")
  if(NOT result STREQUAL "0" OR inventory STREQUAL "")
    message(FATAL_ERROR
      "Cannot list ${suite} tests: ${result}\n"
      "Elapsed: ${elapsed}s; timeout: ${DOBA_CI_INVENTORY_TIMEOUT}s\n"
      "Command: ${binary} --list\n"
      "Evidence: ${evidence}\n${errors}")
  endif()
  message(STATUS "Listed ${suite} tests in ${elapsed}s")
  string(REPLACE "\\" "/" inventory "${inventory}")
  if(suite STREQUAL "resilience")
    if(NOT inventory MATCHES " - resilience TCP drains mixed clients")
      message(FATAL_ERROR "Missing TCP resilience scenario")
    endif()
    message(STATUS "Verified resilience inventory")
    continue()
  endif()
  if(suite STREQUAL "unit")
    set(processing_files task_execution_tests.cpp processing_result_tests.cpp
      engine_concurrency_tests.cpp processing_turn_tests.cpp
      engine_security_state_tests.cpp policy_behavior_tests.cpp)
    if(NOT WIN32)
      list(APPEND processing_files processing_queue_linux_tests.cpp)
    endif()
  else()
    set(processing_files tcp_factory_tests.cpp processing_result_tests.cpp
      server_stream_integrity_tests.cpp http_test_helper_tests.cpp
      tcp_policy_limits_tests.cpp)
    if(WIN32)
      list(APPEND processing_files tcp_processing_windows_tests.cpp)
    else()
      list(APPEND processing_files tcp_processing_linux_tests.cpp)
    endif()
  endif()
  foreach(required_file IN LISTS processing_files)
    if(NOT inventory MATCHES "${required_file}:[0-9]+ - ")
      message(FATAL_ERROR "Missing ${suite} processing cases: ${required_file}")
    endif()
  endforeach()
  if(NOT WIN32)
    if(suite STREQUAL "unit")
      set(native_file tcp_context_linux_tests.cpp)
    else()
      set(native_file tcpip_linux_tests.cpp)
    endif()
    if(NOT inventory MATCHES "${native_file}:[0-9]+ - ")
      message(FATAL_ERROR "Missing Linux core tests: ${native_file}")
    endif()
  endif()

endforeach()

if(NOT WIN32)
  execute_process(
    COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${DOBA_CI_BUILD_DIR}" -N
    OUTPUT_VARIABLE registered
    COMMAND_ERROR_IS_FATAL ANY
  )
  foreach(required IN ITEMS doba_unit_tests doba_integration_tests
      doba_test_diagnostics_tests doba_stream_integrity_tests
      doba_qualification_tests doba_http_policy_tests doba_transport_policy_tests
      doba_security_model_tests
      doba_linux_transport_tests doba_linux_accept_error_tests
      doba_linux_processing_tests doba_httparena_tests)
    if(NOT registered MATCHES "Test +#[0-9]+: ${required}(\n|$)")
      message(FATAL_ERROR "Missing registered Linux test: ${required}")
    endif()
  endforeach()
  file(WRITE "${evidence}/registered-tests.txt" "${registered}")
endif()

if(WIN32)
  execute_process(
    COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${DOBA_CI_BUILD_DIR}" -N
    OUTPUT_VARIABLE registered
    COMMAND_ERROR_IS_FATAL ANY
  )
  if(NOT registered MATCHES "Test +#[0-9]+: doba_iocp_completion_tests(\n|$)")
    message(FATAL_ERROR "Missing Windows IOCP completion interleaving test")
  endif()
  if(NOT registered MATCHES
      "Test +#[0-9]+: doba_windows_lifecycle_tests(\n|$)")
    message(FATAL_ERROR "Missing Windows socket lifecycle fault tests")
  endif()
  foreach(required IN ITEMS doba_windows_processing_tests doba_httparena_tests
      doba_network_fault_tests
      doba_test_diagnostics_tests doba_stream_integrity_tests
      doba_qualification_tests doba_http_policy_tests doba_transport_policy_tests
      doba_security_model_tests)
    if(NOT registered MATCHES "Test +#[0-9]+: ${required}(\n|$)")
      message(FATAL_ERROR "Missing registered Windows test: ${required}")
    endif()
  endforeach()
  foreach(native IN ITEMS iocp_completion windows_lifecycle windows_processing)
    set(binary
      "${DOBA_CI_BUILD_DIR}/tests/integration/doba_${native}_tests${suffix}")
    execute_process(
      COMMAND "${binary}" --list
      RESULT_VARIABLE result
      OUTPUT_VARIABLE inventory
      ERROR_VARIABLE errors
      TIMEOUT "${DOBA_CI_INVENTORY_TIMEOUT}"
    )
    if(NOT result STREQUAL "0" OR inventory STREQUAL "")
      message(FATAL_ERROR "Cannot list Windows ${native}: ${result} ${errors}")
    endif()
    file(WRITE "${evidence}/${native}-inventory.txt" "${inventory}")
  endforeach()
  file(WRITE "${evidence}/registered-tests.txt" "${registered}")
endif()

if(DOBA_CI_REQUIRE_TLS)
  execute_process(
    COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${DOBA_CI_BUILD_DIR}" -N
    OUTPUT_VARIABLE registered COMMAND_ERROR_IS_FATAL ANY)
  set(required_tls doba_tls_tests doba_tls_core_tests
      doba_tls_configuration_tests
      doba_tls_processing_tests doba_tls_processing_wire_tests)
  if(configuration MATCHES "DOBA_BUILD_EXAMPLES:BOOL=ON(\n|$)")
    list(APPEND required_tls doba_tls_examples_tests)
  endif()
  foreach(required IN LISTS required_tls)
    if(NOT registered MATCHES "Test +#[0-9]+: ${required}(\n|$)")
      message(FATAL_ERROR "Missing registered TLS test: ${required}")
    endif()
  endforeach()
  if(NOT configuration MATCHES "DOBA_ENABLE_TLS:BOOL=ON(\n|$)")
    message(FATAL_ERROR "TLS inventory requires DOBA_ENABLE_TLS=ON")
  endif()
endif()

if(configuration MATCHES "DOBA_BUILD_EXAMPLES:BOOL=ON(\n|$)")
  if(NOT registered MATCHES
      "Test +#[0-9]+: doba_processing_examples_tests(\n|$)")
    message(FATAL_ERROR "Missing registered processing executable examples")
  endif()
endif()
