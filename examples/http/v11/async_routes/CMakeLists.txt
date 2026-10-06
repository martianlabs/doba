cmake_minimum_required(
  VERSION 3.20
)

project(
  async_routes
  LANGUAGES CXX
)

add_executable(
  async_routes
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  async_routes
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
