cmake_minimum_required(
  VERSION 3.20
)

project(
  http_async_routes
  LANGUAGES CXX
)

add_executable(
  http_async_routes
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  http_async_routes
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

