cmake_minimum_required(
  VERSION 3.20
)

project(
  request_limits
  LANGUAGES CXX
)

add_executable(
  request_limits
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  request_limits
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
