cmake_minimum_required(
  VERSION 3.20
)

file(
  STRINGS
  "${CMAKE_CURRENT_LIST_DIR}/include/version.h"
  DOBA_VERSION_DEFINITIONS
  REGEX "^#define DOBA_VERSION_(MAJOR|MINOR|PATCH) [0-9]+$"
)
foreach(
  DOBA_VERSION_PART
  MAJOR
  MINOR
  PATCH
)
  string(
    REGEX MATCH
    "#define DOBA_VERSION_${DOBA_VERSION_PART} ([0-9]+)"
    DOBA_VERSION_MATCH
    "${DOBA_VERSION_DEFINITIONS}"
  )
  if(
    NOT DOBA_VERSION_MATCH
  )
    message(
      FATAL_ERROR
      "Unable to read DOBA_VERSION_${DOBA_VERSION_PART} from include/version.h"
    )
  endif()
  set(
    DOBA_VERSION_${DOBA_VERSION_PART}
    "${CMAKE_MATCH_1}"
  )
endforeach()
set(
  DOBA_VERSION
  "${DOBA_VERSION_MAJOR}.${DOBA_VERSION_MINOR}.${DOBA_VERSION_PATCH}"
)

project(
  doba
  VERSION "${DOBA_VERSION}"
  LANGUAGES CXX
)

if(
  CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR
)
  set(
    DOBA_IS_TOP_LEVEL
    ON
  )
else()
  set(
    DOBA_IS_TOP_LEVEL
    OFF
  )
endif()

option(
  DOBA_BUILD_EXAMPLES
  "Build doba examples"
  ${DOBA_IS_TOP_LEVEL}
)
option(
  DOBA_BUILD_TESTS
  "Build doba tests"
  ${DOBA_IS_TOP_LEVEL}
)

include(
  CMakePackageConfigHelpers
)
include(
  GNUInstallDirs
)

find_package(
  Threads
  REQUIRED
)

option(
  DOBA_ENABLE_STRICT_WARNINGS
  "Enable strict warnings for doba targets"
  OFF
)
if(
  DOBA_ENABLE_STRICT_WARNINGS
)
  if(
    MSVC
  )
    add_compile_options(
      /W4
      /WX
    )
  elseif(
    CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"
  )
    add_compile_options(
      -Wall
      -Wextra
      -Wpedantic
      -Werror
    )
  endif()
endif()

add_library(
  doba_headers
  INTERFACE
)
add_library(
  martianlabs::doba
  ALIAS
  doba_headers
)
set_target_properties(
  doba_headers
  PROPERTIES
  EXPORT_NAME doba
)

target_compile_features(
  doba_headers
  INTERFACE
  cxx_std_20
)
target_link_libraries(
  doba_headers
  INTERFACE
  Threads::Threads
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
  "$<BUILD_INTERFACE:${DOBA_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
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
  "$<BUILD_INTERFACE:${DOBA_HEADERS}>"
)

source_group(
  TREE "${DOBA_INCLUDE_DIR}"
  FILES ${DOBA_HEADERS}
)

install(
  TARGETS doba_headers
  EXPORT dobaTargets
)
install(
  DIRECTORY "${DOBA_INCLUDE_DIR}/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

set(
  DOBA_INSTALL_CMAKEDIR
  "${CMAKE_INSTALL_LIBDIR}/cmake/doba"
)
configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/dobaConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/dobaConfig.cmake"
  INSTALL_DESTINATION "${DOBA_INSTALL_CMAKEDIR}"
)
if(
  PROJECT_VERSION_MAJOR EQUAL 0
)
  set(
    DOBA_VERSION_COMPATIBILITY
    SameMinorVersion
  )
else()
  set(
    DOBA_VERSION_COMPATIBILITY
    SameMajorVersion
  )
endif()
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/dobaConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY "${DOBA_VERSION_COMPATIBILITY}"
  ARCH_INDEPENDENT
)
install(
  EXPORT dobaTargets
  FILE dobaTargets.cmake
  NAMESPACE martianlabs::
  DESTINATION "${DOBA_INSTALL_CMAKEDIR}"
)
install(
  FILES
  "${CMAKE_CURRENT_BINARY_DIR}/dobaConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/dobaConfigVersion.cmake"
  DESTINATION "${DOBA_INSTALL_CMAKEDIR}"
)

if(
  DOBA_BUILD_EXAMPLES
)
  add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/examples/common/byte_storage
  )
  add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/examples/common/writer
  )
  add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/examples/common/reader
  )
  add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/examples/common/console_logger
  )
  add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/examples/common/date_server
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
    ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/wildcard_routes
  )
  add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/asynchronous_routes
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
    ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/response_statuses
  )
  add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/response_headers
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
    ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/head_requests
  )
  add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/automatic_not_found
  )
  add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/method_not_allowed
  )
  add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/options_asterisk
  )
  add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/expect_continue
  )
  add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/connection_close
  )
  add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/automatic_date
  )
  add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/examples/http/v11/request_rejections
  )
endif()

if(
  DOBA_BUILD_TESTS
)
  include(
    CTest
  )
  if(
    BUILD_TESTING
  )
    add_subdirectory(
      ${CMAKE_CURRENT_LIST_DIR}/tests/unit
    )
    add_subdirectory(
      ${CMAKE_CURRENT_LIST_DIR}/tests/integration
    )
  endif()
endif()
