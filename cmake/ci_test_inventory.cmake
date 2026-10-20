cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED DOBA_CI_BUILD_DIR)
  message(FATAL_ERROR "DOBA_CI_BUILD_DIR is required")
endif()
get_filename_component(DOBA_CI_BUILD_DIR "${DOBA_CI_BUILD_DIR}" ABSOLUTE)
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
foreach(suite IN ITEMS unit integration)
  set(binary "${DOBA_CI_BUILD_DIR}/tests/${suite}/doba_${suite}_tests${suffix}")
  execute_process(
    COMMAND "${binary}" --list
    RESULT_VARIABLE result
    OUTPUT_VARIABLE inventory
    ERROR_VARIABLE errors
    TIMEOUT 30
  )
  file(WRITE "${evidence}/${suite}-tests.txt" "${inventory}")
  file(WRITE "${evidence}/${suite}-inventory-errors.txt" "${errors}")
  if(NOT result STREQUAL "0" OR inventory STREQUAL "")
    message(FATAL_ERROR "Cannot list ${suite} tests: ${result}\n${errors}")
  endif()
  string(REPLACE "\\" "/" inventory "${inventory}")
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
