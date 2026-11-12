cmake_minimum_required(
  VERSION 3.20
)

project(
  common_reader
  LANGUAGES CXX
)

add_executable(
  common_reader
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  common_reader
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

