cmake_minimum_required(
  VERSION 3.20
)

project(
  doba
  LANGUAGES CXX
)

set(
  CMAKE_CXX_STANDARD
  20
)
set(
  CMAKE_CXX_STANDARD_REQUIRED
  ON
)

add_library(
  doba_headers
  INTERFACE
)

# Sources are UTF-8 without BOM. Tell MSVC to treat both the source and the
# execution character set as UTF-8, since there is no BOM to infer it from.
if(
  MSVC
)
  target_compile_options(
    doba_headers
    INTERFACE
    "/utf-8"
  )
endif()

set(
  DOBA_INCLUDE_DIR
  "${CMAKE_CURRENT_LIST_DIR}/include"
)

target_include_directories(
  doba_headers
  INTERFACE
  "${DOBA_INCLUDE_DIR}"
)

file(
  GLOB_RECURSE
  DOBA_HEADERS
  CONFIGURE_DEPENDS
  "${DOBA_INCLUDE_DIR}/*.h"
)

target_sources(
  doba_headers
  INTERFACE
  ${DOBA_HEADERS}
)

source_group(
  TREE "${DOBA_INCLUDE_DIR}"
  FILES ${DOBA_HEADERS}
)

enable_testing(
)

add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/hello_world
)
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/static_routes
)
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/parametrized_routes
)
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/request_information
)
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/query_parameters
)
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/request_headers
)
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/cookies
)
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/request_body_text
)
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/request_body_binary
)
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/response_status_and_headers
)
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/response_body_values
)
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/response_body_writers
)
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/large_response_bodies
)
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/standard_http_behaviors
)
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/tests/unit
)
