cmake_minimum_required(
  VERSION 3.20
)

project(
  request_body_binary
  LANGUAGES CXX
)

add_executable(
  request_body_binary
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  request_body_binary
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
