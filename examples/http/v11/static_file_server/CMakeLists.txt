cmake_minimum_required(
  VERSION 3.20
)

project(
  static_file_server
  LANGUAGES CXX
)

add_executable(
  static_file_server
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  static_file_server
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
