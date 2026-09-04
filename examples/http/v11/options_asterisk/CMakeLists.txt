cmake_minimum_required(
  VERSION 3.20
)

project(
  options_asterisk
  LANGUAGES CXX
)

add_executable(
  options_asterisk
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  options_asterisk
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

