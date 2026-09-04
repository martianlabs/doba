cmake_minimum_required(
  VERSION 3.20
)

project(
  method_not_allowed
  LANGUAGES CXX
)

add_executable(
  method_not_allowed
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  method_not_allowed
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

