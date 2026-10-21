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

find_package(Git REQUIRED)
execute_process(
  COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_LIST_DIR}/.." rev-parse HEAD
  OUTPUT_VARIABLE revision
  OUTPUT_STRIP_TRAILING_WHITESPACE
  COMMAND_ERROR_IS_FATAL ANY
)
file(STRINGS "${DOBA_CI_BUILD_DIR}/CMakeCache.txt" configuration
  REGEX "^(CMAKE_BUILD_TYPE|CMAKE_CXX_COMPILER|CMAKE_CXX_FLAGS[^:]*|CMAKE_EXE_LINKER_FLAGS[^:]*|DOBA_ENABLE_TLS|DOBA_ENABLE_STRICT_WARNINGS):")
string(REPLACE ";" "\n" configuration "${configuration}")
file(WRITE "${evidence}/configuration.txt"
  "Revision: ${revision}\nRunner: $ENV{RUNNER_OS}\nCMake: ${CMAKE_VERSION}\n${configuration}\n")
if(NOT configuration MATCHES "CMAKE_BUILD_TYPE:STRING=Release(\n|$)")
  message(FATAL_ERROR "CI tests must execute a Release build")
endif()

if(WIN32)
  set(suffix ".exe")
else()
  set(suffix "")
endif()
foreach(suite IN ITEMS unit integration resilience)
  if(suite STREQUAL "resilience")
    set(directory integration)
  else()
    set(directory "${suite}")
  endif()
  set(binary "${DOBA_CI_BUILD_DIR}/tests/${directory}/doba_${suite}_tests${suffix}")
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
    "Command: ${binary} --list\nResult: ${result}\nElapsed: ${elapsed}s\nTimeout: ${DOBA_CI_INVENTORY_TIMEOUT}s\n")
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
    if(DOBA_CI_REQUIRE_TLS AND NOT inventory MATCHES " - resilience TLS drains mixed clients")
      message(FATAL_ERROR "Missing TLS resilience scenario")
    endif()
    message(STATUS "Verified resilience inventory")
    continue()
  endif()
  if(DOBA_CI_REQUIRE_TLS)
    if(suite STREQUAL "unit")
      set(required_files tls_session_tests.cpp)
    else()
      set(required_files tls_tests.cpp tls_http_tests.cpp)
    endif()
    foreach(required_file IN LISTS required_files)
      string(REPLACE "." "\\." pattern "${required_file}")
      if(NOT inventory MATCHES "transport/server/${pattern}:[0-9]+ - ")
        message(FATAL_ERROR "Missing TLS test family: ${suite}/${required_file}")
      endif()
      message(STATUS "Verified TLS inventory: ${suite}/${required_file}")
    endforeach()
  endif()
endforeach()
