cmake_minimum_required(VERSION 3.20)

project(001-hello_world LANGUAGES CXX)

add_executable(001-hello_world
  "${CMAKE_CURRENT_LIST_DIR}/src/001-hello_world.cpp"
)

target_link_libraries(001-hello_world PRIVATE doba_headers)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}/src"
  FILES "${CMAKE_CURRENT_LIST_DIR}/src/001-hello_world.cpp"
)
