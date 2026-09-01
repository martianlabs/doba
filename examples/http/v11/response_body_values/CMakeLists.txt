cmake_minimum_required(
  VERSION 3.20
)

project(
  response_body_values
  LANGUAGES CXX
)

add_executable(
  response_body_values
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  response_body_values
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
