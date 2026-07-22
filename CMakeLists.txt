cmake_minimum_required(VERSION 3.20)

project(doba LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(doba_headers INTERFACE)

set(DOBA_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")

target_include_directories(doba_headers INTERFACE
  "${DOBA_INCLUDE_DIR}"
)

file(GLOB_RECURSE DOBA_HEADERS CONFIGURE_DEPENDS
  "${DOBA_INCLUDE_DIR}/*.h"
)

target_sources(doba_headers INTERFACE
  ${DOBA_HEADERS}
)

source_group(
  TREE "${DOBA_INCLUDE_DIR}"
  FILES ${DOBA_HEADERS}
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test/ut-001-main)
