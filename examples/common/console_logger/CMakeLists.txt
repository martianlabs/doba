cmake_minimum_required(
  VERSION 3.20
)

project(
  common_console_logger
  LANGUAGES CXX
)

add_executable(
  common_console_logger
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  common_console_logger
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

