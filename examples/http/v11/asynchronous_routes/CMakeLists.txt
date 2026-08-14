cmake_minimum_required(
  VERSION 3.20
)

project(
  asynchronous_routes
  LANGUAGES CXX
)

add_executable(
  asynchronous_routes
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  asynchronous_routes
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
