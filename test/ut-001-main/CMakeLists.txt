cmake_minimum_required(VERSION 3.20)

project(test-001 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE DOBA_HEADERS
  "${CMAKE_SOURCE_DIR}/include/*.h"
)

add_executable(test-001)

target_sources(test-001 
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
	${DOBA_HEADERS}
)

target_include_directories(test-001 
  PRIVATE 
    "${CMAKE_SOURCE_DIR}/include"
)

source_group(
  TREE "${CMAKE_CURRENT_SOURCE_DIR}"
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

source_group(
  TREE "${CMAKE_SOURCE_DIR}"
  FILES ${DOBA_HEADERS}
)
