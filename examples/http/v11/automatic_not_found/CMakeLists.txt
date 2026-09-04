cmake_minimum_required(
  VERSION 3.20
)

project(
  automatic_not_found
  LANGUAGES CXX
)

add_executable(
  automatic_not_found
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  automatic_not_found
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

