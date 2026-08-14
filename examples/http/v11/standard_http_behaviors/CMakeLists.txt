cmake_minimum_required(
  VERSION 3.20
)

project(
  standard_http_behaviors
  LANGUAGES CXX
)

add_executable(
  standard_http_behaviors
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  standard_http_behaviors
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
