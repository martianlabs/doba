cmake_minimum_required(
  VERSION 3.20
)

project(
  common_filesystem_monitor
  LANGUAGES CXX
)

add_executable(
  common_filesystem_monitor
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  common_filesystem_monitor
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
