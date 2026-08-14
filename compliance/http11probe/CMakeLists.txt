cmake_minimum_required(VERSION 3.20)

project(doba_http11probe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DOBA_INCLUDE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../../include"
    CACHE PATH "Path to the doba include directory")

find_package(Threads REQUIRED)

add_executable(doba_http11probe
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_include_directories(doba_http11probe PRIVATE
  "${DOBA_INCLUDE_DIR}"
)

target_link_libraries(doba_http11probe PRIVATE Threads::Threads)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
