cmake_minimum_required(
  VERSION 3.20
)

project(
  https_hello_world
  LANGUAGES CXX
)

add_executable(
  https_hello_world
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  https_hello_world
  PRIVATE
  doba_tls
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
