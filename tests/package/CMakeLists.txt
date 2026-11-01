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
option(
  DOBA_PACKAGE_USE_TLS
  "Consume the doba TLS target"
  OFF
)
if(DOBA_PACKAGE_USE_TLS AND DOBA_PACKAGE_USE_SOURCE)
  set(DOBA_ENABLE_TLS ON CACHE BOOL "Enable TLS support" FORCE)
endif()

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
  if(DOBA_PACKAGE_USE_TLS)
    find_package(doba CONFIG REQUIRED COMPONENTS TLS)
  else()
    find_package(doba CONFIG REQUIRED)
  endif()
endif()

if(NOT DOBA_PACKAGE_USE_TLS AND NOT DOBA_PACKAGE_USE_SOURCE AND
    TARGET martianlabs::doba_tls)
  message(FATAL_ERROR "Plain consumers must not import the TLS target")
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

if(DOBA_PACKAGE_USE_TLS)
  add_executable(doba_tls_consumer tls_consumer.cpp)
  target_link_libraries(doba_tls_consumer PRIVATE martianlabs::doba_tls)
endif()
