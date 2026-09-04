cmake_minimum_required(
  VERSION 3.20
)

project(
  doba_package_consumer
  LANGUAGES CXX
)

option(
  DOBA_PACKAGE_USE_SOURCE
  "Consume doba with add_subdirectory"
  OFF
)
if(
  DOBA_PACKAGE_USE_SOURCE
)
  include(
    CTest
  )
  if(
    NOT DOBA_SOURCE_DIR
  )
    message(
      FATAL_ERROR
      "DOBA_SOURCE_DIR is required when DOBA_PACKAGE_USE_SOURCE is enabled"
    )
  endif()
  add_subdirectory(
    "${DOBA_SOURCE_DIR}"
    doba
  )
  if(
    TARGET hello_world OR
    TARGET doba_unit_tests OR
    TARGET doba_integration_tests
  )
    message(
      FATAL_ERROR
      "doba added examples or tests to its parent project"
    )
  endif()
else()
  find_package(
    doba
    CONFIG
    REQUIRED
  )
endif()

add_executable(
  doba_package_consumer
  "${CMAKE_CURRENT_LIST_DIR}/protocol/http/v11/server_tests.cpp"
)
target_link_libraries(
  doba_package_consumer
  PRIVATE
  martianlabs::doba
)
