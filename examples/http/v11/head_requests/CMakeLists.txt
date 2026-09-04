cmake_minimum_required(
  VERSION 3.20
)

project(
  head_requests
  LANGUAGES CXX
)

add_executable(
  head_requests
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  head_requests
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

